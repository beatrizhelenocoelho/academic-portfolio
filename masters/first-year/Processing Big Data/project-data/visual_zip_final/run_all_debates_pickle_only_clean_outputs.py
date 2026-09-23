"""
Run the pickle-only visual pipeline for every debate.

This script intentionally mirrors the notebook logic, but forces pickle-only identity features:
1) clean detections with visual_cleaning.clean_visual_dataframe + add_clean_emotion_columns
2) run visual_identity_solver with the same Config values as the notebook, except use_insightface=False
3) keep single-person shots unconstrained exactly as the notebook does
4) infer person_A/person_C names from 10+ second single-person speaking runs exactly as the notebook does
5) export only the four final CSV files requested: candidate mapping, movement, emotions, topics

Expected project structure:
project_root/
  src/
    visual_cleaning.py
    visual_identity_solver.py
  data/
    *_visual.pkl
    speaker_by_second_all_debates_named_clean.csv
    analise_temas_por_segmento.csv
  outputs/
"""

from __future__ import annotations

import argparse
import json
import re
import sys
import traceback
from itertools import permutations
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import numpy as np
import pandas as pd


# ---------------------------------------------------------------------
# Notebook constants
# ---------------------------------------------------------------------

WINDOW_SIZE = 10
MIN_SINGLE_RUN = 10
MIN_DOMINANT_SHARE = 0.60
MIN_MEAN_CONFIDENCE = 0.55
VOICE_TIME_OFFSET = 0
HAND_AND_ARM_INDICES = [7, 8, 9, 10]


# ---------------------------------------------------------------------
# Utility helpers
# ---------------------------------------------------------------------

def add_src_to_path(project_root: Path):
    src_path = project_root / "src"
    if str(src_path) not in sys.path:
        sys.path.insert(0, str(src_path))

    import visual_identity_solver as solver
    import visual_cleaning
    from visual_cleaning import clean_visual_dataframe, add_clean_emotion_columns

    return solver, clean_visual_dataframe, add_clean_emotion_columns


def strip_feature_suffix(path_or_name: Any) -> str:
    name = Path(str(path_or_name)).name
    for suffix in ["_visual.pkl", "_audio.pkl", "_speech.pkl", "_text.pkl", "_ocr.pkl", ".pkl", ".csv"]:
        if name.endswith(suffix):
            return name[: -len(suffix)]
    return Path(name).stem


def norm_id(x: Any) -> str:
    s = strip_feature_suffix(x).lower()
    s = re.sub(r"[^a-z0-9]+", "_", s)
    s = re.sub(r"_+", "_", s).strip("_")
    return s


def read_csv_flexible(path: Path) -> pd.DataFrame:
    try:
        return pd.read_csv(path)
    except Exception:
        return pd.read_csv(path, sep=";")


def mode_or_none(series: pd.Series):
    series = series.dropna()
    series = series[series.astype(str) != ""]
    if len(series) == 0:
        return None
    return series.value_counts().index[0]


def safe_emotion_confidence(row):
    """
    Exact notebook logic: read top_emotion and return the probability for that exact key.
    """
    top_emotion = row.get("top_emotion", None)

    if top_emotion is None or pd.isna(top_emotion) or top_emotion == "":
        return np.nan

    probs_raw = row.get("emotion_probs_json", None)

    if probs_raw is None or pd.isna(probs_raw) or probs_raw == "":
        return np.nan

    try:
        probs = json.loads(probs_raw) if isinstance(probs_raw, str) else probs_raw
        return float(probs.get(top_emotion, np.nan))
    except Exception:
        return np.nan


def keypoint_movement(current_kp, previous_kp, frame_width: int, frame_height: int, indices=None):
    """
    Exact notebook logic, except frame_width/frame_height are passed explicitly.
    """
    try:
        cur = np.asarray(current_kp, dtype=float)
        prev = np.asarray(previous_kp, dtype=float)
    except Exception:
        return np.nan

    if cur.ndim != 2 or prev.ndim != 2:
        return np.nan

    if cur.shape[0] == 0 or prev.shape[0] == 0:
        return np.nan

    n = min(cur.shape[0], prev.shape[0])

    cur = cur[:n]
    prev = prev[:n]

    if indices is not None:
        valid_indices = [i for i in indices if i < n]
        if len(valid_indices) == 0:
            return np.nan
        cur = cur[valid_indices]
        prev = prev[valid_indices]

    cur_xy = cur[:, :2].astype(float)
    prev_xy = prev[:, :2].astype(float)

    if cur.shape[1] >= 3 and prev.shape[1] >= 3:
        conf_mask = (cur[:, 2] > 0.20) & (prev[:, 2] > 0.20)
    else:
        conf_mask = np.ones(len(cur_xy), dtype=bool)

    finite_mask = (
        np.isfinite(cur_xy).all(axis=1)
        & np.isfinite(prev_xy).all(axis=1)
        & conf_mask
    )

    if finite_mask.sum() == 0:
        return np.nan

    cur_norm = cur_xy[finite_mask] / np.array([frame_width, frame_height])
    prev_norm = prev_xy[finite_mask] / np.array([frame_width, frame_height])

    distances = np.sqrt(((cur_norm - prev_norm) ** 2).sum(axis=1))

    return float(np.nanmean(distances))


def build_speaker_meta(speaker_df: pd.DataFrame) -> pd.DataFrame:
    cols = [c for c in ["debate_name", "source_file", "candidate_1", "candidate_2", "month", "day"] if c in speaker_df.columns]
    meta = speaker_df[cols].drop_duplicates().copy()
    if "source_file" in meta.columns:
        meta["source_id"] = meta["source_file"].map(strip_feature_suffix)
        meta["match_id"] = meta["source_file"].map(norm_id)
    else:
        meta["source_id"] = meta["debate_name"]
        meta["match_id"] = meta["debate_name"].map(norm_id)
    return meta


def match_visual_to_speaker(visual_pkl: Path, speaker_meta: pd.DataFrame) -> Optional[pd.Series]:
    visual_id = norm_id(visual_pkl.name)
    exact = speaker_meta[speaker_meta["match_id"] == visual_id]
    if len(exact) > 0:
        return exact.iloc[0]

    # Some projects store copied/renamed files. Try containment as fallback.
    contains = speaker_meta[
        speaker_meta["match_id"].map(lambda x: visual_id in x or x in visual_id)
    ]
    if len(contains) > 0:
        return contains.iloc[0]

    return None


# ---------------------------------------------------------------------
# 1) Identity solver: copied from notebook logic
# ---------------------------------------------------------------------

def run_identity_pickle_only(
    visual_pkl: Path,
    debate_id: str,
    project_root: Path,
    processed_dir: Path,
    solver,
    clean_visual_dataframe,
    add_clean_emotion_columns,
    force_rerun: bool,
):
    solver_output_path = processed_dir / f"{debate_id}_identity_solver_PICKLE_ONLY_SINGLE_UNCONSTRAINED_CLEANED_predictions.pkl"
    solver_frames_path = processed_dir / f"{debate_id}_identity_solver_PICKLE_ONLY_SINGLE_UNCONSTRAINED_CLEANED_frames.pkl"
    solver_meta_path = processed_dir / f"{debate_id}_identity_solver_PICKLE_ONLY_SINGLE_UNCONSTRAINED_CLEANED_metadata.json"

    if solver_output_path.exists() and solver_frames_path.exists() and solver_meta_path.exists() and not force_rerun:
        out = pd.read_pickle(solver_output_path)
        df_solver = pd.read_pickle(solver_frames_path)
        meta = json.loads(solver_meta_path.read_text(encoding="utf-8"))
        return out, df_solver, int(meta["width"]), int(meta["height"]), meta.get("model_name", "cached")

    cfg = solver.Config(
        pkl=visual_pkl,
        project_root=project_root,
        frames_root=Path("Frames"),
        out=processed_dir / f"{debate_id}_identity_solver_pickle_only_single_unconstrained_outputs",

        # PICKLE-ONLY MODE: do not use InsightFace or raw-frame embeddings.
        use_insightface=False,

        # Multi-person constraints stay active
        force_small_two_has_person2=True,

        # IMPORTANT: same as notebook
        use_first_single_p2_prior=False,
        early_single_p2_always=False,

        constraint_first=True,

        prototype_temperature=2.25,
        low_confidence_threshold=0.55,

        two_large_sum_area=0.55,
        two_touching_gap=0.035,

        save_debug_images=False,
        annotate_every=0,
    )

    print("Loading raw pickle...")
    df_raw = solver.load_visual_pickle(visual_pkl)

    print("Cleaning visual detections first...")
    df_clean = clean_visual_dataframe(df_raw)
    df_clean = add_clean_emotion_columns(df_clean)

    df_solver = df_clean.copy()
    df_solver["Poses"] = df_solver["Clean_Poses"]
    df_solver["Fer"] = df_solver["Clean_Fer"]

    df_solver["frame_num"] = df_solver["Frame"].map(solver.parse_frame_num)
    df_solver = df_solver.sort_values("frame_num").reset_index(drop=True)

    df_solver["n_poses"] = df_solver["Poses"].map(
        lambda x: len(x) if isinstance(x, list) else 0
    )

    df_solver["n_faces"] = df_solver["Fer"].map(
        lambda x: sum(1 for f in x if isinstance(f, dict)) if isinstance(x, list) else 0
    )

    print("Inferring frame size...")
    width, height = solver.infer_frame_size(df_solver, project_root)
    print("Frame size:", width, height)

    print("Building detection table from CLEANED boxes...")
    det = solver.build_detection_table(df_solver, cfg, width, height)
    print("Cleaned detections:", len(det))

    print("Creating base features...")
    base_X, base_names, aux = solver.create_base_features(det, width, height)
    det = pd.concat([det.reset_index(drop=True), aux.reset_index(drop=True)], axis=1)

    print("Skipping InsightFace: using pickle features only.")
    arc_X = None

    print("Building model features...")
    X = solver.build_model_features(det, base_X, arc_X, cfg)

    print("Assigning weak labels...")
    det = solver.assign_weak_labels(det, cfg)

    print("Fitting identity model...")
    if cfg.constraint_first:
        probs, pred, model_name = solver.fit_prototype_identity_model(X, det, cfg)
    else:
        probs, pred, model_name = solver.fit_identity_model(X, det, cfg)

    print("Model:", model_name)

    print("Applying frame constraints...")
    out = solver.apply_frame_constraints(det, probs, cfg)

    # IMPORTANT CORRECTION from notebook: single-person shots stay model/prototype predictions.
    out["unconstrained_model_person"] = out["model_person"]
    out["unconstrained_model_confidence"] = out["model_confidence"]

    single_mask = out["n_faces"] == 1

    out.loc[single_mask, "person_label"] = out.loc[single_mask, "unconstrained_model_person"]
    out.loc[single_mask, "confidence"] = out.loc[single_mask, "unconstrained_model_confidence"]
    out.loc[single_mask, "assignment_source"] = "model_single_unconstrained"

    out["model_person"] = out["person_label"]
    out["model_confidence"] = out["confidence"]

    out.to_pickle(solver_output_path)
    df_solver.to_pickle(solver_frames_path)
    solver_meta_path.write_text(
        json.dumps(
            {
                "debate_id": debate_id,
                "visual_pkl": str(visual_pkl),
                "width": int(width),
                "height": int(height),
                "model_name": model_name,
                "use_insightface": False,
            },
            indent=2,
            ensure_ascii=False,
        ),
        encoding="utf-8",
    )

    return out, df_solver, int(width), int(height), model_name


# ---------------------------------------------------------------------
# 2) Name mapping: exact notebook logic generalized per debate
# ---------------------------------------------------------------------

def infer_names_like_notebook(out: pd.DataFrame, voice_df: pd.DataFrame, candidate_names: List[str]):
    single_frames = (
        out.loc[
            out["n_faces"] == 1,
            ["frame", "frame_num", "person_label", "confidence"]
        ]
        .drop_duplicates(subset=["frame_num"])
        .sort_values("frame_num")
        .reset_index(drop=True)
    )

    single_frames["voice_second"] = single_frames["frame_num"].astype(int) + VOICE_TIME_OFFSET

    single_frames = single_frames.merge(
        voice_df[["second", "speaker_name", "voice_label", "estimated_speaker"]],
        left_on="voice_second",
        right_on="second",
        how="left",
    )

    single_frames["new_run"] = (
        single_frames["person_label"].ne(single_frames["person_label"].shift())
        | single_frames["frame_num"].diff().fillna(1).ne(1)
    )

    single_frames["run_id"] = single_frames["new_run"].cumsum()

    run_rows = []

    for run_id, g in single_frames.groupby("run_id"):
        g = g.sort_values("frame_num")

        visual_label = g["person_label"].iloc[0]
        run_length = len(g)

        if visual_label not in ["person_A", "person_C"]:
            continue

        if run_length < MIN_SINGLE_RUN:
            continue

        start_sec = int(g["frame_num"].min())
        end_sec = int(g["frame_num"].max())
        mean_confidence = float(g["confidence"].mean())

        candidate_speaking = g[g["speaker_name"].isin(candidate_names)]

        if len(candidate_speaking) == 0:
            dominant_speaker = None
            dominant_count = 0
            dominant_share = 0.0
        else:
            counts = candidate_speaking["speaker_name"].value_counts()
            dominant_speaker = counts.index[0]
            dominant_count = int(counts.iloc[0])
            dominant_share = dominant_count / run_length

        run_rows.append(
            {
                "run_id": int(run_id),
                "person_label": visual_label,
                "start_sec": start_sec,
                "end_sec": end_sec,
                "run_length": run_length,
                "mean_confidence": mean_confidence,
                "dominant_speaker": dominant_speaker,
                "dominant_count": dominant_count,
                "dominant_share": dominant_share,
            }
        )

    named_run_summary = pd.DataFrame(run_rows)

    if len(named_run_summary) == 0:
        reliable_named_runs = pd.DataFrame(columns=list(named_run_summary.columns))
    else:
        reliable_named_runs = named_run_summary[
            (named_run_summary["run_length"] >= MIN_SINGLE_RUN)
            & (named_run_summary["dominant_share"] >= MIN_DOMINANT_SHARE)
            & (named_run_summary["mean_confidence"] >= MIN_MEAN_CONFIDENCE)
            & (named_run_summary["dominant_speaker"].notna())
        ].copy()

    if len(reliable_named_runs) > 0:
        named_mapping_votes = (
            reliable_named_runs
            .groupby(["person_label", "dominant_speaker"])
            .agg(
                n_runs=("run_id", "count"),
                total_seconds=("run_length", "sum"),
                mean_run_length=("run_length", "mean"),
                mean_share=("dominant_share", "mean"),
                mean_confidence=("mean_confidence", "mean"),
            )
            .reset_index()
            .sort_values(
                ["person_label", "total_seconds", "n_runs", "mean_share", "mean_confidence"],
                ascending=[True, False, False, False, False],
            )
        )
    else:
        named_mapping_votes = pd.DataFrame(
            columns=[
                "person_label",
                "dominant_speaker",
                "n_runs",
                "total_seconds",
                "mean_run_length",
                "mean_share",
                "mean_confidence",
            ]
        )

    score_table = {
        (row["person_label"], row["dominant_speaker"]): float(row["total_seconds"])
        for _, row in named_mapping_votes.iterrows()
    }

    best_score = -1
    best_assignment = {}

    if len(candidate_names) >= 2:
        for perm in permutations(candidate_names, 2):
            candidate_assignment = {
                "person_A": perm[0],
                "person_C": perm[1],
            }

            score = sum(
                score_table.get((visual_label, speaker_name), 0)
                for visual_label, speaker_name in candidate_assignment.items()
            )

            if score > best_score:
                best_score = score
                best_assignment = candidate_assignment

    # Exact notebook fallback: unresolved labels remain person_A/person_C.
    visual_to_name_map = {
        "person_A": best_assignment.get("person_A", "person_A"),
        "person_B": "Moderador/Other",
        "person_C": best_assignment.get("person_C", "person_C"),
    }

    return visual_to_name_map, named_run_summary, reliable_named_runs, named_mapping_votes, best_score


# ---------------------------------------------------------------------
# 3) Visual movement/emotion export: exact notebook logic generalized
# ---------------------------------------------------------------------

def build_visual_second_like_notebook(
    out_named: pd.DataFrame,
    debate_name: str,
    debate_id: str,
    frame_width: int,
    frame_height: int,
) -> pd.DataFrame:
    visual_timeline = out_named.copy()

    visual_timeline["debate_name"] = debate_name
    visual_timeline["debate_id"] = debate_id
    visual_timeline["second"] = visual_timeline["frame_num"].astype(int)

    if "display_label" not in visual_timeline.columns:
        visual_timeline["display_label"] = visual_timeline["person_label"]

    visual_timeline["candidate"] = visual_timeline["display_label"]

    visual_timeline["face_cx_norm"] = visual_timeline["face_cx"] / frame_width
    visual_timeline["face_cy_norm"] = visual_timeline["face_cy"] / frame_height

    visual_timeline["face_width_norm"] = (
        visual_timeline["face_x2"] - visual_timeline["face_x1"]
    ) / frame_width

    visual_timeline["face_height_norm"] = (
        visual_timeline["face_y2"] - visual_timeline["face_y1"]
    ) / frame_height

    if "emotion_probs_json" in visual_timeline.columns:
        visual_timeline["top_emotion_confidence"] = visual_timeline.apply(
            safe_emotion_confidence,
            axis=1,
        )
    else:
        visual_timeline["top_emotion_confidence"] = np.nan

    visual_timeline["top_emotion_clean"] = (
        visual_timeline["top_emotion"]
        .fillna("unknown")
        .astype(str)
    )

    visual_timeline["is_non_neutral_emotion"] = (
        ~visual_timeline["top_emotion_clean"].str.lower().isin(
            ["neutral", "unknown", "none", ""]
        )
    ).astype(int)

    visual_timeline["non_neutral_emotion_score"] = (
        visual_timeline["is_non_neutral_emotion"]
        * visual_timeline["top_emotion_confidence"].fillna(0)
    )

    visual_timeline = visual_timeline.sort_values(
        ["display_label", "second", "face_idx_lr"]
    ).reset_index(drop=True)

    visual_timeline["prev_second"] = visual_timeline.groupby("display_label")["second"].shift(1)
    visual_timeline["prev_face_cx_norm"] = visual_timeline.groupby("display_label")["face_cx_norm"].shift(1)
    visual_timeline["prev_face_cy_norm"] = visual_timeline.groupby("display_label")["face_cy_norm"].shift(1)

    visual_timeline["is_consecutive_frame"] = (
        visual_timeline["second"] - visual_timeline["prev_second"] == 1
    )

    visual_timeline["face_movement"] = np.where(
        visual_timeline["is_consecutive_frame"],
        np.sqrt(
            (visual_timeline["face_cx_norm"] - visual_timeline["prev_face_cx_norm"]) ** 2
            + (visual_timeline["face_cy_norm"] - visual_timeline["prev_face_cy_norm"]) ** 2
        ),
        np.nan,
    )

    visual_timeline["pose_movement"] = np.nan
    visual_timeline["hand_movement"] = np.nan

    if "pose_keypoints" in visual_timeline.columns:
        for label, group in visual_timeline.groupby("display_label"):
            group = group.sort_values("second")

            previous_idx = None
            previous_kp = None
            previous_second = None

            for idx, row in group.iterrows():
                current_second = int(row["second"])
                current_kp = row["pose_keypoints"]

                if (
                    previous_idx is not None
                    and previous_second is not None
                    and current_second - previous_second == 1
                ):
                    visual_timeline.at[idx, "pose_movement"] = keypoint_movement(
                        current_kp,
                        previous_kp,
                        frame_width,
                        frame_height,
                        indices=None,
                    )

                    visual_timeline.at[idx, "hand_movement"] = keypoint_movement(
                        current_kp,
                        previous_kp,
                        frame_width,
                        frame_height,
                        indices=HAND_AND_ARM_INDICES,
                    )

                previous_idx = idx
                previous_kp = current_kp
                previous_second = current_second

    visual_timeline["movement_score"] = visual_timeline[
        ["face_movement", "pose_movement", "hand_movement"]
    ].mean(axis=1, skipna=True)

    visual_timeline["movement_percentile_by_person"] = (
        visual_timeline
        .groupby("display_label")["movement_score"]
        .rank(pct=True)
    )

    visual_timeline["high_movement_flag"] = (
        visual_timeline["movement_percentile_by_person"] >= 0.90
    ).astype(int)

    visual_timeline["strong_emotion_flag"] = (
        visual_timeline["non_neutral_emotion_score"] >= 0.60
    ).astype(int)

    visual_timeline["visual_interest_flag"] = (
        (visual_timeline["high_movement_flag"] == 1)
        | (visual_timeline["strong_emotion_flag"] == 1)
    ).astype(int)

    visual_second_cols = [
        "debate_name",
        "debate_id",
        "second",
        "frame_num",
        "candidate",
        "display_label",
        "person_label",
        "confidence",
        "assignment_source",
        "n_faces",
        "n_poses",
        "face_cx_norm",
        "face_cy_norm",
        "face_area_norm",
        "face_width_norm",
        "face_height_norm",
        "top_emotion_clean",
        "top_emotion_confidence",
        "non_neutral_emotion_score",
        "face_movement",
        "pose_movement",
        "hand_movement",
        "movement_score",
        "movement_percentile_by_person",
        "high_movement_flag",
        "strong_emotion_flag",
        "visual_interest_flag",
    ]

    visual_second_cols = [c for c in visual_second_cols if c in visual_timeline.columns]
    return visual_timeline[visual_second_cols].copy()


def build_visual_windows_like_notebook(visual_second_export: pd.DataFrame) -> pd.DataFrame:
    visual_second_export = visual_second_export.copy()

    visual_second_export["window_start"] = (
        visual_second_export["second"] // WINDOW_SIZE
    ) * WINDOW_SIZE

    visual_second_export["window_end"] = (
        visual_second_export["window_start"] + WINDOW_SIZE - 1
    )

    emotion_counts = (
        visual_second_export
        .groupby(["debate_name", "debate_id", "candidate", "window_start", "top_emotion_clean"])
        .size()
        .reset_index(name="emotion_count")
    )

    emotion_totals = (
        visual_second_export
        .groupby(["debate_name", "debate_id", "candidate", "window_start"])
        .size()
        .reset_index(name="visible_seconds")
    )

    emotion_counts = emotion_counts.merge(
        emotion_totals,
        on=["debate_name", "debate_id", "candidate", "window_start"],
        how="left",
    )

    emotion_counts["emotion_share"] = (
        emotion_counts["emotion_count"] / emotion_counts["visible_seconds"]
    )

    emotion_pivot = emotion_counts.pivot_table(
        index=["debate_name", "debate_id", "candidate", "window_start"],
        columns="top_emotion_clean",
        values="emotion_share",
        fill_value=0,
    ).reset_index()

    emotion_pivot.columns = [
        f"emotion_share_{c}" if c not in ["debate_name", "debate_id", "candidate", "window_start"] else c
        for c in emotion_pivot.columns
    ]

    visual_windows = (
        visual_second_export
        .groupby(["debate_name", "debate_id", "candidate", "window_start"])
        .agg(
            window_end=("window_end", "max"),
            visible_seconds=("second", "count"),
            mean_identity_confidence=("confidence", "mean"),

            mean_movement=("movement_score", "mean"),
            max_movement=("movement_score", "max"),
            mean_face_movement=("face_movement", "mean"),
            mean_pose_movement=("pose_movement", "mean"),
            mean_hand_movement=("hand_movement", "mean"),

            dominant_emotion=("top_emotion_clean", mode_or_none),
            mean_emotion_confidence=("top_emotion_confidence", "mean"),
            mean_non_neutral_emotion_score=("non_neutral_emotion_score", "mean"),

            high_movement_seconds=("high_movement_flag", "sum"),
            strong_emotion_seconds=("strong_emotion_flag", "sum"),
            visually_interesting_seconds=("visual_interest_flag", "sum"),
        )
        .reset_index()
    )

    visual_windows = visual_windows.merge(
        emotion_pivot,
        on=["debate_name", "debate_id", "candidate", "window_start"],
        how="left",
    )

    visual_windows["visible_share_of_window"] = (
        visual_windows["visible_seconds"] / WINDOW_SIZE
    )

    visual_windows["high_movement_share"] = (
        visual_windows["high_movement_seconds"] / visual_windows["visible_seconds"]
    )

    visual_windows["strong_emotion_share"] = (
        visual_windows["strong_emotion_seconds"] / visual_windows["visible_seconds"]
    )

    visual_windows["visual_interest_share"] = (
        visual_windows["visually_interesting_seconds"] / visual_windows["visible_seconds"]
    )

    visual_windows["visual_intensity_score"] = (
        visual_windows["mean_movement"].rank(pct=True)
        + visual_windows["mean_non_neutral_emotion_score"].rank(pct=True)
        + visual_windows["mean_hand_movement"].rank(pct=True)
    )

    visual_windows = visual_windows.sort_values(
        ["debate_name", "window_start", "candidate"]
    ).reset_index(drop=True)

    return visual_windows


# ---------------------------------------------------------------------
# 4) Topics aligned to the same 10-second candidate-window schema
# ---------------------------------------------------------------------

def align_topics_to_10s(topics_df: pd.DataFrame) -> pd.DataFrame:
    required = {"debate_name", "candidate", "timestamp", "duration", "Tema_Dominante"}
    missing = required - set(topics_df.columns)
    if missing:
        raise ValueError(f"Topics CSV missing columns: {sorted(missing)}")

    rows = []
    topic_score_cols = [
        c for c in topics_df.columns
        if c not in {"timestamp", "duration", "transcript", "text_embedding", "midpoint_sec", "second", "candidate", "debate_name", "Tema_Dominante"}
        and pd.api.types.is_numeric_dtype(topics_df[c])
    ]

    for _, r in topics_df.iterrows():
        debate_name = r["debate_name"]
        debate_id = norm_id(debate_name)
        candidate = r["candidate"]
        start = float(r["timestamp"])
        duration = float(r["duration"])
        end = start + duration
        topic = r["Tema_Dominante"]

        if not np.isfinite(start) or not np.isfinite(end) or end <= start:
            continue

        first_window = int(start // WINDOW_SIZE) * WINDOW_SIZE
        last_window = int((max(start, end - 1e-9)) // WINDOW_SIZE) * WINDOW_SIZE

        for ws in range(first_window, last_window + 1, WINDOW_SIZE):
            we_excl = ws + WINDOW_SIZE
            overlap = max(0.0, min(end, we_excl) - max(start, ws))
            if overlap <= 0:
                continue
            row = {
                "debate_name": debate_name,
                "debate_id": debate_id,
                "candidate": candidate,
                "window_start": int(ws),
                "window_end": int(ws + WINDOW_SIZE - 1),
                "topic_segment_start": start,
                "topic_segment_end": end,
                "topic_overlap_seconds": float(overlap),
                "topic": topic,
            }
            for col in topic_score_cols:
                row[f"topic_score_{col}"] = float(r[col]) if pd.notna(r[col]) else np.nan
            rows.append(row)

    expanded = pd.DataFrame(rows)
    if expanded.empty:
        return expanded

    summary_rows = []
    for keys, g in expanded.groupby(["debate_name", "debate_id", "candidate", "window_start", "window_end"], sort=False):
        debate_name, debate_id, candidate, ws, we = keys
        topic_scores = g.groupby("topic")["topic_overlap_seconds"].sum().sort_values(ascending=False)
        dominant_topic = topic_scores.index[0]
        dominant_overlap = float(topic_scores.iloc[0])
        total_overlap = float(g["topic_overlap_seconds"].sum())

        row = {
            "debate_name": debate_name,
            "debate_id": debate_id,
            "candidate": candidate,
            "window_start": int(ws),
            "window_end": int(we),
            "dominant_topic": dominant_topic,
            "dominant_topic_overlap_seconds": dominant_overlap,
            "total_topic_overlap_seconds": total_overlap,
            "dominant_topic_share": dominant_overlap / max(total_overlap, 1e-9),
        }

        for topic, seconds in topic_scores.items():
            safe_topic = re.sub(r"[^A-Za-z0-9_]+", "_", str(topic)).strip("_")
            row[f"topic_overlap_{safe_topic}"] = float(seconds)

        # Topic model score columns: take max score in the overlapping segments.
        for col in [c for c in expanded.columns if c.startswith("topic_score_")]:
            row[col] = float(g[col].max()) if g[col].notna().any() else np.nan

        summary_rows.append(row)

    return pd.DataFrame(summary_rows).sort_values(["debate_name", "window_start", "candidate"]).reset_index(drop=True)


# ---------------------------------------------------------------------
# Save standardized output split into names/movement/emotions/topics
# ---------------------------------------------------------------------

def save_outputs(export_dir: Path, mapping_rows, windows_rows, topics_10s):
    """
    Clean final outputs only.

    This intentionally saves only the four CSV files requested:
      01_candidate_name_mapping_all_debates.csv
      02_movement_10s_all_debates.csv
      03_emotions_10s_all_debates.csv
      04_topics_10s_all_debates.csv

    The script may still cache intermediate PKL files in outputs/processed so that reruns are faster,
    but those are not final analysis CSVs.
    """
    export_dir.mkdir(parents=True, exist_ok=True)

    mapping_df = pd.DataFrame(mapping_rows)
    visual_windows = pd.concat(windows_rows, ignore_index=True) if windows_rows else pd.DataFrame()

    mapping_df.to_csv(export_dir / "01_candidate_name_mapping_all_debates.csv", index=False)

    if visual_windows.empty:
        movement = pd.DataFrame()
        emotions = pd.DataFrame()
    else:
        key_cols = ["debate_name", "debate_id", "candidate", "window_start", "window_end"]

        movement_cols = key_cols + [
            "visible_seconds", "visible_share_of_window", "mean_identity_confidence",
            "mean_movement", "max_movement",
            "mean_face_movement", "mean_pose_movement", "mean_hand_movement",
            "high_movement_seconds", "high_movement_share",
            "visual_interest_share", "visual_intensity_score",
        ]
        movement_cols = [c for c in movement_cols if c in visual_windows.columns]
        movement = visual_windows[movement_cols].copy()

        emotion_cols = key_cols + [
            "visible_seconds", "visible_share_of_window", "mean_identity_confidence",
            "dominant_emotion", "mean_emotion_confidence", "mean_non_neutral_emotion_score",
            "strong_emotion_seconds", "strong_emotion_share",
            "visual_interest_share", "visual_intensity_score",
        ] + [c for c in visual_windows.columns if c.startswith("emotion_share_")]
        emotion_cols = [c for c in emotion_cols if c in visual_windows.columns]
        emotions = visual_windows[emotion_cols].copy()

    movement.to_csv(export_dir / "02_movement_10s_all_debates.csv", index=False)
    emotions.to_csv(export_dir / "03_emotions_10s_all_debates.csv", index=False)

    if topics_10s is None:
        topics_10s = pd.DataFrame()
    topics_10s.to_csv(export_dir / "04_topics_10s_all_debates.csv", index=False)


# ---------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-root", type=Path, default=Path("."))
    parser.add_argument("--data-dir", type=Path, default=None)
    parser.add_argument("--speaker-csv", type=Path, default=None)
    parser.add_argument("--topics-csv", type=Path, default=None)
    parser.add_argument("--limit", type=int, default=None)
    parser.add_argument("--force-rerun", action="store_true")
    args = parser.parse_args()

    project_root = args.project_root.resolve()
    data_dir = args.data_dir.resolve() if args.data_dir else project_root / "data"
    processed_dir = project_root / "outputs" / "processed"
    export_dir = project_root / "outputs" / "multimodal_exports"
    processed_dir.mkdir(parents=True, exist_ok=True)
    export_dir.mkdir(parents=True, exist_ok=True)

    speaker_csv = args.speaker_csv or data_dir / "speaker_by_second_all_debates_named_clean.csv"
    topics_csv = args.topics_csv or data_dir / "analise_temas_por_segmento.csv"

    solver, clean_visual_dataframe, add_clean_emotion_columns = add_src_to_path(project_root)

    print("Project root:", project_root)
    print("Data dir:", data_dir)
    print("Speaker CSV:", speaker_csv)
    print("Topics CSV:", topics_csv)
    print("Use InsightFace: False (pickle-only mode)")
    print("Solver file:", solver.__file__)

    speaker_df = read_csv_flexible(speaker_csv)
    speaker_meta = build_speaker_meta(speaker_df)

    visual_pkls = sorted(data_dir.rglob("*_visual.pkl"))
    if args.limit is not None:
        visual_pkls = visual_pkls[: args.limit]

    print(f"Found {len(visual_pkls)} visual PKLs to process.")

    mapping_rows = []
    windows_rows = []
    failures = []

    for i, visual_pkl in enumerate(visual_pkls, start=1):
        print("\n" + "=" * 100)
        print(f"[{i}/{len(visual_pkls)}] {visual_pkl.name}")

        try:
            meta = match_visual_to_speaker(visual_pkl, speaker_meta)
            if meta is None:
                raise ValueError(f"Could not match visual PKL to speaker CSV: {visual_pkl.name}")

            debate_name = str(meta["debate_name"])
            debate_id = strip_feature_suffix(visual_pkl.name)
            voice_df = speaker_df[speaker_df["debate_name"] == debate_name].copy().sort_values("second").reset_index(drop=True)
            candidate_names = sorted(
                set(voice_df["candidate_1"].dropna().unique()).union(
                    set(voice_df["candidate_2"].dropna().unique())
                )
            )

            print("Debate name:", debate_name)
            print("Debate ID:", debate_id)
            print("Candidate names:", candidate_names)

            out, df_solver, width, height, model_name = run_identity_pickle_only(
                visual_pkl=visual_pkl,
                debate_id=debate_id,
                project_root=project_root,
                processed_dir=processed_dir,
                solver=solver,
                clean_visual_dataframe=clean_visual_dataframe,
                add_clean_emotion_columns=add_clean_emotion_columns,
                force_rerun=args.force_rerun,
            )

            visual_to_name_map, run_summary, reliable_runs, votes, best_score = infer_names_like_notebook(
                out=out,
                voice_df=voice_df,
                candidate_names=candidate_names,
            )

            out_named = out.copy()
            out_named["display_label"] = (
                out_named["person_label"]
                .map(visual_to_name_map)
                .fillna(out_named["person_label"])
            )

            for visual_label in ["person_A", "person_B", "person_C"]:
                mapping_rows.append(
                    {
                        "debate_name": debate_name,
                        "debate_id": debate_id,
                        "visual_label": visual_label,
                        "candidate": visual_to_name_map.get(visual_label, visual_label),
                        "model_name": model_name,
                        "use_insightface": False,
                        "mapping_score": best_score,
                        "mapping_method": "exact_notebook_single_person_runs",
                        "is_unresolved": int(visual_to_name_map.get(visual_label, visual_label) in ["person_A", "person_C"]),
                    }
                )

            visual_second = build_visual_second_like_notebook(out_named, debate_name, debate_id, width, height)
            visual_windows = build_visual_windows_like_notebook(visual_second)

            windows_rows.append(visual_windows)

        except Exception as e:
            print("FAILED:", visual_pkl)
            traceback.print_exc()
            failures.append({"visual_pkl": str(visual_pkl), "error": repr(e)})

    topics_10s = None
    if topics_csv.exists():
        topics_df = read_csv_flexible(topics_csv)
        topics_10s = align_topics_to_10s(topics_df)
    else:
        print("Topics CSV not found; skipping topics:", topics_csv)

    save_outputs(export_dir, mapping_rows, windows_rows, topics_10s)

    if failures:
        pd.DataFrame(failures).to_csv(export_dir / "00_failed_debates.csv", index=False)
        print("Failures saved to:", export_dir / "00_failed_debates.csv")

    print("\nDone. Main outputs in:", export_dir)
    print("  01_candidate_name_mapping_all_debates.csv")
    print("  02_movement_10s_all_debates.csv")
    print("  03_emotions_10s_all_debates.csv")
    print("  04_topics_10s_all_debates.csv")


if __name__ == "__main__":
    main()
