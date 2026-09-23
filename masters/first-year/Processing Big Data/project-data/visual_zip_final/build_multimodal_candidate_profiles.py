#!/usr/bin/env python
"""
Build multimodal candidate profiles and correlations.

Inputs expected in the same folder, or passed explicitly:
  01_candidate_name_mapping_all_debates.csv
  02_movement_10s_all_debates.csv
  03_emotions_10s_all_debates.csv
  04_topics_10s_all_debates.csv
  audio_events_by_second_all_debates_clean.csv

Outputs:
  outputs/analysis/
    01_multimodal_10s_windows.csv
    02_candidate_profiles_global.csv
    03_candidate_profiles_by_debate.csv
    04_candidate_topic_profiles.csv
    05_global_correlations.csv
    06_candidate_correlations.csv
    07_topic_correlations.csv
    08_peak_events.csv
    09_analysis_assessments.txt
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path
from typing import Iterable, List, Optional

import numpy as np
import pandas as pd


WINDOW_SIZE = 10
KEYS = ["debate_name", "candidate", "window_start", "window_end"]


# ---------------------------------------------------------------------
# Basic helpers
# ---------------------------------------------------------------------

def read_csv(path: Path) -> pd.DataFrame:
    try:
        return pd.read_csv(path)
    except UnicodeDecodeError:
        return pd.read_csv(path, encoding="utf-8-sig")


def norm_id(x) -> str:
    s = str(x).lower()
    s = re.sub(r"[^a-z0-9]+", "_", s)
    return re.sub(r"_+", "_", s).strip("_")


def mode_or_none(s: pd.Series):
    s = s.dropna()
    s = s[s.astype(str) != ""]
    if len(s) == 0:
        return np.nan
    return s.value_counts().index[0]


def weighted_mean(values, weights):
    values = pd.Series(values, dtype="float64")
    weights = pd.Series(weights, dtype="float64")
    mask = values.notna() & weights.notna() & (weights > 0)
    if mask.sum() == 0:
        return np.nan
    return float(np.average(values[mask], weights=weights[mask]))


def weighted_share(df: pd.DataFrame, col: str, weight_col: str):
    if col not in df.columns:
        return np.nan
    return weighted_mean(df[col], df[weight_col])


def safe_div(num, den):
    if den is None or pd.isna(den) or den == 0:
        return np.nan
    return num / den


def clean_candidate_name(name):
    if pd.isna(name):
        return np.nan
    s = str(name).strip()
    aliases = {
        "Person 1": None,
        "Person 2": None,
        "Person 3": "Moderador/Other",
        "No speech": "No speech",
        "nan": np.nan,
    }
    return aliases.get(s, s)


# ---------------------------------------------------------------------
# Audio aggregation
# ---------------------------------------------------------------------

def map_audio_speaker_to_candidate(row):
    """
    The audio file uses Person 1 / Person 2 / Person 3.
    In the speaker annotations, Person 1 corresponds to candidate_1,
    Person 2 corresponds to candidate_2, and Person 3 is the moderator/other.
    """
    speaker = str(row.get("speaker_name", "")).strip()

    if speaker == "Person 1":
        return row.get("candidate_1")
    if speaker == "Person 2":
        return row.get("candidate_2")
    if speaker == "Person 3":
        return "Moderador/Other"
    if speaker == "No speech":
        return "No speech"

    # If the file is already named, keep it.
    return speaker if speaker else np.nan


def aggregate_audio_to_10s(audio: pd.DataFrame) -> pd.DataFrame:
    audio = audio.copy()

    audio["candidate"] = audio.apply(map_audio_speaker_to_candidate, axis=1)
    audio["window_start"] = (audio["second"].astype(int) // WINDOW_SIZE) * WINDOW_SIZE
    audio["window_end"] = audio["window_start"] + WINDOW_SIZE - 1

    # Use overlap seconds as speaking weight.
    if "speaker_overlap_sec" not in audio.columns:
        audio["speaker_overlap_sec"] = 1.0

    rows = []
    group_cols = ["debate_name", "candidate", "window_start", "window_end"]

    for keys, g in audio.groupby(group_cols, dropna=False):
        debate_name, candidate, ws, we = keys

        speaking_seconds = float(g["speaker_overlap_sec"].fillna(0).sum())

        row = {
            "debate_name": debate_name,
            "candidate": candidate,
            "window_start": int(ws),
            "window_end": int(we),
            "speaking_seconds": speaking_seconds,
            "speaking_share_of_window": speaking_seconds / WINDOW_SIZE,
            "audio_rows": int(len(g)),
        }

        for col in ["speechrate", "speechrate_z", "pitch_variability", "pitchvar_z", "segment_duration", "npause"]:
            if col in g.columns:
                row[f"mean_{col}"] = weighted_mean(g[col], g["speaker_overlap_sec"])

        for col in [
            "speechrate_increase",
            "pitchvar_increase",
            "speaker_change",
            "overlap_detected",
            "interruption_proxy",
            "visual_interest_event",
        ]:
            if col in g.columns:
                row[f"{col}_seconds"] = float(g[col].fillna(0).sum())

        if "event_type" in g.columns:
            row["dominant_audio_event_type"] = mode_or_none(g["event_type"])

        rows.append(row)

    out = pd.DataFrame(rows)

    # Do not use No speech as a candidate profile row.
    out = out[out["candidate"].notna()].copy()

    return out


# ---------------------------------------------------------------------
# Merge
# ---------------------------------------------------------------------

def build_multimodal_windows(movement, emotions, topics, audio_10s):
    # Some files have debate_id, but topics had a lower-case debate_id in earlier versions.
    # Use debate_name + candidate + window_start/window_end as the reliable merge key.
    for df in [movement, emotions, topics, audio_10s]:
        if "window_start" in df.columns:
            df["window_start"] = df["window_start"].astype(int)
        if "window_end" in df.columns:
            df["window_end"] = df["window_end"].astype(int)

    # Avoid duplicate columns from movement/emotion shared metadata.
    movement_cols = list(movement.columns)
    emotion_drop = [
        c for c in ["debate_id", "visible_seconds", "visible_share_of_window", "mean_identity_confidence",
                  "visual_interest_share", "visual_intensity_score"]
        if c in emotions.columns
    ]
    emotions_slim = emotions.drop(columns=emotion_drop, errors="ignore")

    topics_drop = [c for c in ["debate_id"] if c in topics.columns]
    topics_slim = topics.drop(columns=topics_drop, errors="ignore")

    merged = movement.merge(emotions_slim, on=KEYS, how="outer")
    merged = merged.merge(topics_slim, on=KEYS, how="outer")
    merged = merged.merge(audio_10s, on=KEYS, how="outer")

    # Normalize a few convenience indicators.
    merged["is_moderator"] = (merged["candidate"].astype(str) == "Moderador/Other").astype(int)
    merged["is_no_speech"] = (merged["candidate"].astype(str) == "No speech").astype(int)
    merged["has_visual"] = merged.get("visible_seconds", pd.Series(index=merged.index, dtype=float)).fillna(0).gt(0).astype(int)
    merged["has_audio"] = merged.get("speaking_seconds", pd.Series(index=merged.index, dtype=float)).fillna(0).gt(0).astype(int)
    merged["has_topic"] = merged.get("total_topic_overlap_seconds", pd.Series(index=merged.index, dtype=float)).fillna(0).gt(0).astype(int)

    return merged.sort_values(KEYS).reset_index(drop=True)


# ---------------------------------------------------------------------
# Profiles
# ---------------------------------------------------------------------

def build_candidate_profiles_global(merged: pd.DataFrame) -> pd.DataFrame:
    df = merged[
        (~merged["candidate"].isin(["Moderador/Other", "No speech"]))
        & merged["candidate"].notna()
    ].copy()

    emotion_cols = [c for c in df.columns if c.startswith("emotion_share_")]
    topic_overlap_cols = [c for c in df.columns if c.startswith("topic_overlap_")]

    rows = []

    for candidate, g in df.groupby("candidate"):
        visible_weight = g.get("visible_seconds", pd.Series(0, index=g.index)).fillna(0)
        speaking_weight = g.get("speaking_seconds", pd.Series(0, index=g.index)).fillna(0)
        topic_weight = g.get("total_topic_overlap_seconds", pd.Series(0, index=g.index)).fillna(0)

        row = {
            "candidate": candidate,
            "n_debates": int(g["debate_name"].nunique()),
            "n_windows": int(len(g)),
            "total_visible_seconds": float(visible_weight.sum()),
            "total_speaking_seconds": float(speaking_weight.sum()),
            "avg_visible_share_of_window": weighted_mean(g.get("visible_share_of_window", np.nan), visible_weight),
            "avg_speaking_share_of_window": weighted_mean(g.get("speaking_share_of_window", np.nan), speaking_weight),
            "avg_movement": weighted_mean(g.get("mean_movement", np.nan), visible_weight),
            "avg_hand_movement": weighted_mean(g.get("mean_hand_movement", np.nan), visible_weight),
            "avg_pose_movement": weighted_mean(g.get("mean_pose_movement", np.nan), visible_weight),
            "avg_face_movement": weighted_mean(g.get("mean_face_movement", np.nan), visible_weight),
            "avg_non_neutral_emotion_score": weighted_mean(g.get("mean_non_neutral_emotion_score", np.nan), visible_weight),
            "avg_strong_emotion_share": weighted_mean(g.get("strong_emotion_share", np.nan), visible_weight),
            "avg_speechrate": weighted_mean(g.get("mean_speechrate", np.nan), speaking_weight),
            "avg_speechrate_z": weighted_mean(g.get("mean_speechrate_z", np.nan), speaking_weight),
            "avg_pitch_variability": weighted_mean(g.get("mean_pitch_variability", np.nan), speaking_weight),
            "avg_pitchvar_z": weighted_mean(g.get("mean_pitchvar_z", np.nan), speaking_weight),
            "speechrate_increase_seconds": float(g.get("speechrate_increase_seconds", pd.Series(0, index=g.index)).fillna(0).sum()),
            "pitchvar_increase_seconds": float(g.get("pitchvar_increase_seconds", pd.Series(0, index=g.index)).fillna(0).sum()),
            "interruption_proxy_seconds": float(g.get("interruption_proxy_seconds", pd.Series(0, index=g.index)).fillna(0).sum()),
            "overlap_detected_seconds": float(g.get("overlap_detected_seconds", pd.Series(0, index=g.index)).fillna(0).sum()),
        }

        row["interruptions_per_100_speaking_sec"] = 100 * safe_div(row["interruption_proxy_seconds"], row["total_speaking_seconds"])
        row["overlap_per_100_speaking_sec"] = 100 * safe_div(row["overlap_detected_seconds"], row["total_speaking_seconds"])

        # Emotion accumulation: weighted average of every emotion share, not reduced to one emotion.
        emotion_values = {}
        for col in emotion_cols:
            clean_name = col.replace("emotion_share_", "emotion_avg_share_")
            value = weighted_mean(g[col], visible_weight)
            row[clean_name] = value
            emotion_values[col.replace("emotion_share_", "")] = value

        top_emotions = sorted(
            [(k, v) for k, v in emotion_values.items() if pd.notna(v)],
            key=lambda kv: kv[1],
            reverse=True,
        )
        row["top_emotions"] = "; ".join([f"{k}:{v:.3f}" for k, v in top_emotions[:5]])

        # Topic accumulation.
        topic_values = {}
        for col in topic_overlap_cols:
            total = float(g[col].fillna(0).sum())
            topic = col.replace("topic_overlap_", "")
            topic_values[topic] = total

        total_topic_seconds = sum(topic_values.values())
        top_topics = sorted(topic_values.items(), key=lambda kv: kv[1], reverse=True)
        row["top_topics"] = "; ".join(
            [f"{topic}:{seconds:.1f}s ({safe_div(seconds, total_topic_seconds) or 0:.1%})"
             for topic, seconds in top_topics[:5] if seconds > 0]
        )
        row["total_topic_seconds"] = float(total_topic_seconds)

        # Simple descriptive indexes. These are relative z-scores across candidates added later.
        rows.append(row)

    profiles = pd.DataFrame(rows)

    # Add relative ranks/z-style interpretation columns.
    for col in ["avg_movement", "avg_hand_movement", "avg_non_neutral_emotion_score", "avg_speechrate_z", "avg_pitchvar_z", "interruptions_per_100_speaking_sec"]:
        if col in profiles.columns:
            profiles[f"{col}_rank_desc"] = profiles[col].rank(ascending=False, method="min")

    profiles["expressiveness_score"] = (
        profiles["avg_movement"].rank(pct=True)
        + profiles["avg_hand_movement"].rank(pct=True)
        + profiles["avg_non_neutral_emotion_score"].rank(pct=True)
        + profiles["avg_pitchvar_z"].rank(pct=True)
        + profiles["avg_speechrate_z"].rank(pct=True)
    )

    profiles["composure_score"] = (
        profiles["emotion_avg_share_Neutral"].rank(pct=True) if "emotion_avg_share_Neutral" in profiles.columns else 0
    ) - profiles["expressiveness_score"].rank(pct=True)

    return profiles.sort_values("expressiveness_score", ascending=False).reset_index(drop=True)


def build_candidate_profiles_by_debate(merged: pd.DataFrame) -> pd.DataFrame:
    df = merged[
        (~merged["candidate"].isin(["Moderador/Other", "No speech"]))
        & merged["candidate"].notna()
    ].copy()

    rows = []
    for (debate, candidate), g in df.groupby(["debate_name", "candidate"]):
        visible_weight = g.get("visible_seconds", pd.Series(0, index=g.index)).fillna(0)
        speaking_weight = g.get("speaking_seconds", pd.Series(0, index=g.index)).fillna(0)

        rows.append({
            "debate_name": debate,
            "candidate": candidate,
            "visible_seconds": float(visible_weight.sum()),
            "speaking_seconds": float(speaking_weight.sum()),
            "avg_movement": weighted_mean(g.get("mean_movement", np.nan), visible_weight),
            "avg_hand_movement": weighted_mean(g.get("mean_hand_movement", np.nan), visible_weight),
            "avg_non_neutral_emotion_score": weighted_mean(g.get("mean_non_neutral_emotion_score", np.nan), visible_weight),
            "avg_strong_emotion_share": weighted_mean(g.get("strong_emotion_share", np.nan), visible_weight),
            "avg_speechrate_z": weighted_mean(g.get("mean_speechrate_z", np.nan), speaking_weight),
            "avg_pitchvar_z": weighted_mean(g.get("mean_pitchvar_z", np.nan), speaking_weight),
            "interruption_proxy_seconds": float(g.get("interruption_proxy_seconds", pd.Series(0, index=g.index)).fillna(0).sum()),
            "dominant_topic_mode": mode_or_none(g.get("dominant_topic", pd.Series(dtype=object))),
        })

    return pd.DataFrame(rows).sort_values(["candidate", "debate_name"]).reset_index(drop=True)


def build_candidate_topic_profiles(merged: pd.DataFrame) -> pd.DataFrame:
    df = merged[
        (~merged["candidate"].isin(["Moderador/Other", "No speech"]))
        & merged["candidate"].notna()
        & merged["dominant_topic"].notna()
    ].copy()

    rows = []
    for (candidate, topic), g in df.groupby(["candidate", "dominant_topic"]):
        visible_weight = g.get("visible_seconds", pd.Series(0, index=g.index)).fillna(0)
        speaking_weight = g.get("speaking_seconds", pd.Series(0, index=g.index)).fillna(0)
        topic_weight = g.get("dominant_topic_overlap_seconds", pd.Series(0, index=g.index)).fillna(0)

        rows.append({
            "candidate": candidate,
            "topic": topic,
            "n_windows": int(len(g)),
            "topic_overlap_seconds": float(topic_weight.sum()),
            "visible_seconds": float(visible_weight.sum()),
            "speaking_seconds": float(speaking_weight.sum()),
            "avg_movement": weighted_mean(g.get("mean_movement", np.nan), visible_weight),
            "avg_hand_movement": weighted_mean(g.get("mean_hand_movement", np.nan), visible_weight),
            "avg_non_neutral_emotion_score": weighted_mean(g.get("mean_non_neutral_emotion_score", np.nan), visible_weight),
            "avg_strong_emotion_share": weighted_mean(g.get("strong_emotion_share", np.nan), visible_weight),
            "avg_speechrate_z": weighted_mean(g.get("mean_speechrate_z", np.nan), speaking_weight),
            "avg_pitchvar_z": weighted_mean(g.get("mean_pitchvar_z", np.nan), speaking_weight),
            "interruption_proxy_seconds": float(g.get("interruption_proxy_seconds", pd.Series(0, index=g.index)).fillna(0).sum()),
            "dominant_emotion_mode": mode_or_none(g.get("dominant_emotion", pd.Series(dtype=object))),
        })

    return pd.DataFrame(rows).sort_values(["candidate", "topic_overlap_seconds"], ascending=[True, False]).reset_index(drop=True)


# ---------------------------------------------------------------------
# Correlations and events
# ---------------------------------------------------------------------

def corr_pair(df: pd.DataFrame, x: str, y: str, group=None, min_n=20):
    cols = [x, y]
    if group:
        cols.append(group)
    d = df[cols].replace([np.inf, -np.inf], np.nan).dropna()
    if group is None:
        if len(d) < min_n:
            return []
        return [{
            "group": "ALL",
            "x": x,
            "y": y,
            "n": int(len(d)),
            "pearson": float(d[x].corr(d[y], method="pearson")),
            "spearman": float(d[x].corr(d[y], method="spearman")),
        }]

    rows = []
    for gname, g in d.groupby(group):
        if len(g) < min_n:
            continue
        rows.append({
            "group": gname,
            "x": x,
            "y": y,
            "n": int(len(g)),
            "pearson": float(g[x].corr(g[y], method="pearson")),
            "spearman": float(g[x].corr(g[y], method="spearman")),
        })
    return rows


def build_correlations(merged: pd.DataFrame):
    df = merged[
        (~merged["candidate"].isin(["Moderador/Other", "No speech"]))
        & merged["candidate"].notna()
    ].copy()

    candidate_pairs = [
        ("mean_movement", "mean_speechrate_z"),
        ("mean_hand_movement", "mean_speechrate_z"),
        ("mean_movement", "mean_pitchvar_z"),
        ("mean_hand_movement", "mean_pitchvar_z"),
        ("mean_non_neutral_emotion_score", "mean_speechrate_z"),
        ("mean_non_neutral_emotion_score", "mean_pitchvar_z"),
        ("strong_emotion_share", "mean_speechrate_z"),
        ("strong_emotion_share", "mean_pitchvar_z"),
        ("high_movement_share", "interruption_proxy_seconds"),
        ("mean_hand_movement", "interruption_proxy_seconds"),
    ]

    # Add emotion shares vs audio/movement.
    emotion_cols = [c for c in df.columns if c.startswith("emotion_share_")]
    for e in emotion_cols:
        candidate_pairs.extend([
            (e, "mean_speechrate_z"),
            (e, "mean_pitchvar_z"),
            (e, "mean_movement"),
        ])

    global_rows = []
    candidate_rows = []
    topic_rows = []

    for x, y in candidate_pairs:
        if x in df.columns and y in df.columns:
            global_rows.extend(corr_pair(df, x, y, group=None, min_n=30))
            candidate_rows.extend(corr_pair(df, x, y, group="candidate", min_n=20))
            if "dominant_topic" in df.columns:
                topic_rows.extend(corr_pair(df, x, y, group="dominant_topic", min_n=30))

    return pd.DataFrame(global_rows), pd.DataFrame(candidate_rows), pd.DataFrame(topic_rows)


def build_peak_events(merged: pd.DataFrame) -> pd.DataFrame:
    df = merged[
        (~merged["candidate"].isin(["Moderador/Other", "No speech"]))
        & merged["candidate"].notna()
    ].copy()

    # Percentiles within each candidate make peaks relative to their own baseline.
    for col in ["mean_movement", "mean_hand_movement", "mean_non_neutral_emotion_score", "mean_speechrate_z", "mean_pitchvar_z"]:
        if col in df.columns:
            df[f"{col}_pct_by_candidate"] = df.groupby("candidate")[col].rank(pct=True)

    conditions = []
    if {"mean_movement_pct_by_candidate", "mean_speechrate_z_pct_by_candidate"}.issubset(df.columns):
        conditions.append(("high_movement_high_speechrate",
                           (df["mean_movement_pct_by_candidate"] >= 0.90) & (df["mean_speechrate_z_pct_by_candidate"] >= 0.90)))
    if {"mean_hand_movement_pct_by_candidate", "mean_pitchvar_z_pct_by_candidate"}.issubset(df.columns):
        conditions.append(("high_hand_movement_high_pitch_variability",
                           (df["mean_hand_movement_pct_by_candidate"] >= 0.90) & (df["mean_pitchvar_z_pct_by_candidate"] >= 0.90)))
    if {"mean_non_neutral_emotion_score_pct_by_candidate", "mean_speechrate_z_pct_by_candidate"}.issubset(df.columns):
        conditions.append(("high_emotion_high_speechrate",
                           (df["mean_non_neutral_emotion_score_pct_by_candidate"] >= 0.90) & (df["mean_speechrate_z_pct_by_candidate"] >= 0.90)))
    if {"mean_non_neutral_emotion_score_pct_by_candidate", "mean_pitchvar_z_pct_by_candidate"}.issubset(df.columns):
        conditions.append(("high_emotion_high_pitch_variability",
                           (df["mean_non_neutral_emotion_score_pct_by_candidate"] >= 0.90) & (df["mean_pitchvar_z_pct_by_candidate"] >= 0.90)))

    event_rows = []
    for event_type, mask in conditions:
        tmp = df[mask].copy()
        tmp["multimodal_event_type"] = event_type
        event_rows.append(tmp)

    if not event_rows:
        return pd.DataFrame()

    events = pd.concat(event_rows, ignore_index=True)

    keep = [
        "multimodal_event_type", "debate_name", "candidate", "window_start", "window_end",
        "dominant_topic", "dominant_emotion",
        "mean_movement", "mean_hand_movement", "mean_non_neutral_emotion_score",
        "mean_speechrate_z", "mean_pitchvar_z",
        "speaking_seconds", "visible_seconds",
        "interruption_proxy_seconds",
    ]
    keep = [c for c in keep if c in events.columns]

    return events[keep].sort_values(["candidate", "debate_name", "window_start"]).reset_index(drop=True)


def write_assessments(path: Path, profiles: pd.DataFrame, global_corr: pd.DataFrame, candidate_corr: pd.DataFrame, topic_profiles: pd.DataFrame, peak_events: pd.DataFrame):
    lines = []
    lines.append("MULTIMODAL ANALYSIS ASSESSMENTS")
    lines.append("=" * 80)
    lines.append("")
    lines.append("Important: these are descriptive associations, not causal claims.")
    lines.append("They should be used as starting points for manual inspection and interpretation.")
    lines.append("")

    if not profiles.empty:
        lines.append("1) Candidate style profiles")
        lines.append("-" * 80)

        for _, r in profiles.sort_values("expressiveness_score", ascending=False).iterrows():
            lines.append(
                f"{r['candidate']}: visible={r['total_visible_seconds']:.0f}s, "
                f"speaking={r['total_speaking_seconds']:.0f}s, "
                f"avg_movement={r['avg_movement']:.4f}, "
                f"avg_hand_movement={r['avg_hand_movement']:.4f}, "
                f"avg_non_neutral_emotion={r['avg_non_neutral_emotion_score']:.4f}, "
                f"avg_speechrate_z={r['avg_speechrate_z']:.3f}, "
                f"avg_pitchvar_z={r['avg_pitchvar_z']:.3f}, "
                f"interruptions/100s={r['interruptions_per_100_speaking_sec']:.2f}."
            )
            lines.append(f"  Top emotions: {r.get('top_emotions', '')}")
            lines.append(f"  Top topics: {r.get('top_topics', '')}")
        lines.append("")

    if not global_corr.empty:
        lines.append("2) Strongest global correlations")
        lines.append("-" * 80)
        tmp = global_corr.copy()
        tmp["abs_spearman"] = tmp["spearman"].abs()
        for _, r in tmp.sort_values("abs_spearman", ascending=False).head(15).iterrows():
            direction = "positive" if r["spearman"] > 0 else "negative"
            lines.append(
                f"{r['x']} vs {r['y']}: Spearman={r['spearman']:.3f} "
                f"({direction}, n={int(r['n'])})."
            )
        lines.append("")

    if not candidate_corr.empty:
        lines.append("3) Candidate-specific correlations worth inspecting")
        lines.append("-" * 80)
        tmp = candidate_corr.copy()
        tmp["abs_spearman"] = tmp["spearman"].abs()
        for _, r in tmp.sort_values("abs_spearman", ascending=False).head(20).iterrows():
            lines.append(
                f"{r['group']}: {r['x']} vs {r['y']} has Spearman={r['spearman']:.3f} "
                f"(n={int(r['n'])})."
            )
        lines.append("")

    if not topic_profiles.empty:
        lines.append("4) Topic-style combinations")
        lines.append("-" * 80)
        # Top expressive candidate-topic combos.
        tmp = topic_profiles.copy()
        for col in ["avg_movement", "avg_non_neutral_emotion_score", "avg_speechrate_z", "avg_pitchvar_z"]:
            if col not in tmp.columns:
                tmp[col] = np.nan
        tmp["topic_expressiveness_score"] = (
            tmp["avg_movement"].rank(pct=True)
            + tmp["avg_non_neutral_emotion_score"].rank(pct=True)
            + tmp["avg_speechrate_z"].rank(pct=True)
            + tmp["avg_pitchvar_z"].rank(pct=True)
        )
        for _, r in tmp.sort_values("topic_expressiveness_score", ascending=False).head(15).iterrows():
            lines.append(
                f"{r['candidate']} on {r['topic']}: movement={r['avg_movement']:.4f}, "
                f"emotion_score={r['avg_non_neutral_emotion_score']:.4f}, "
                f"speechrate_z={r['avg_speechrate_z']:.3f}, pitchvar_z={r['avg_pitchvar_z']:.3f}, "
                f"dominant_emotion={r.get('dominant_emotion_mode', '')}."
            )
        lines.append("")

    if not peak_events.empty:
        lines.append("5) Peak multimodal events")
        lines.append("-" * 80)
        lines.append(f"Found {len(peak_events)} multimodal peak windows.")
        for _, r in peak_events.head(25).iterrows():
            lines.append(
                f"{r['candidate']} | {r['debate_name']} | {int(r['window_start'])}-{int(r['window_end'])}s | "
                f"{r['multimodal_event_type']} | topic={r.get('dominant_topic', '')} | "
                f"emotion={r.get('dominant_emotion', '')}."
            )
        lines.append("")

    path.write_text("\n".join(lines), encoding="utf-8")


# ---------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-dir", type=Path, default=Path("."))
    parser.add_argument("--output-dir", type=Path, default=Path("outputs/analysis"))
    parser.add_argument("--mapping", type=Path, default=None)
    parser.add_argument("--movement", type=Path, default=None)
    parser.add_argument("--emotions", type=Path, default=None)
    parser.add_argument("--topics", type=Path, default=None)
    parser.add_argument("--audio", type=Path, default=None)
    args = parser.parse_args()

    input_dir = args.input_dir
    output_dir = args.output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    movement_path = args.movement or input_dir / "02_movement_10s_all_debates.csv"
    emotions_path = args.emotions or input_dir / "03_emotions_10s_all_debates.csv"
    topics_path = args.topics or input_dir / "04_topics_10s_all_debates.csv"
    audio_path = args.audio or input_dir / "audio_events_by_second_all_debates_clean.csv"

    movement = read_csv(movement_path)
    emotions = read_csv(emotions_path)
    topics = read_csv(topics_path)
    audio = read_csv(audio_path)

    print("Loaded:")
    print(" movement:", movement.shape, movement_path)
    print(" emotions:", emotions.shape, emotions_path)
    print(" topics:", topics.shape, topics_path)
    print(" audio:", audio.shape, audio_path)

    audio_10s = aggregate_audio_to_10s(audio)
    audio_10s.to_csv(output_dir / "00_audio_10s_all_debates_named.csv", index=False)

    merged = build_multimodal_windows(movement, emotions, topics, audio_10s)
    merged.to_csv(output_dir / "01_multimodal_10s_windows.csv", index=False)

    profiles_global = build_candidate_profiles_global(merged)
    profiles_global.to_csv(output_dir / "02_candidate_profiles_global.csv", index=False)

    profiles_by_debate = build_candidate_profiles_by_debate(merged)
    profiles_by_debate.to_csv(output_dir / "03_candidate_profiles_by_debate.csv", index=False)

    topic_profiles = build_candidate_topic_profiles(merged)
    topic_profiles.to_csv(output_dir / "04_candidate_topic_profiles.csv", index=False)

    global_corr, candidate_corr, topic_corr = build_correlations(merged)
    global_corr.to_csv(output_dir / "05_global_correlations.csv", index=False)
    candidate_corr.to_csv(output_dir / "06_candidate_correlations.csv", index=False)
    topic_corr.to_csv(output_dir / "07_topic_correlations.csv", index=False)

    peak_events = build_peak_events(merged)
    peak_events.to_csv(output_dir / "08_peak_events.csv", index=False)

    write_assessments(
        output_dir / "09_analysis_assessments.txt",
        profiles_global,
        global_corr,
        candidate_corr,
        topic_profiles,
        peak_events,
    )

    print("\nSaved analysis outputs to:", output_dir)
    for p in sorted(output_dir.glob("*")):
        print(" -", p.name)


if __name__ == "__main__":
    main()
