"""
visual_identity_solver_A_B_C.py

Constraint-first / neural-embedding person identity assignment for debate visual frames.

This version has the glasses cleaner completely removed.

Goal
----
Assign every detected face in a debate video to one of:
    person_A = left participant in wide 3-person shots
    person_B = centre/moderator in wide 3-person shots
    person_C = right participant in wide 3-person shots

This script uses two complementary signals:
1) A face embedding network, preferably InsightFace ArcFace, when the frame images exist locally.
2) The face landmarks / pose / bounding boxes already present in the *_visual.pkl file.

No manually labelled examples are required. Reliable camera-composition rules are treated as the main signal.
A small prototype classifier is used only when the rules cannot decide a single-person close-up.

Example
-------
Run from your project root, e.g. Documents/big_data:

    python src/visual_identity_solver.py \
        --pkl data/raw_features/Ventura_vs_Marques_Mendes_November_25_visual.pkl \
        --project-root . \
        --out outputs/visual_identity_A_B_C \
        --use-insightface

Recommended install:

    pip install pandas numpy scikit-learn scipy matplotlib opencv-python tqdm pyarrow
    pip install insightface onnxruntime

InsightFace downloads its model pack on first use. If it is not installed, or images are not found,
the script falls back to a landmark/pose feature model so it can still run from the pickle alone.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import re
import warnings
from dataclasses import dataclass
from itertools import permutations
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Sequence, Tuple

import numpy as np
import pandas as pd

from sklearn.decomposition import PCA
from sklearn.impute import SimpleImputer
from sklearn.metrics import silhouette_score
from sklearn.preprocessing import Normalizer, StandardScaler
from sklearn.semi_supervised import LabelSpreading
from sklearn.cluster import KMeans

try:
    from scipy.optimize import linear_sum_assignment
except Exception:  # pragma: no cover
    linear_sum_assignment = None

try:
    from tqdm import tqdm
except Exception:  # pragma: no cover
    def tqdm(x, **kwargs):  # type: ignore
        return x

try:
    import cv2
except Exception:  # pragma: no cover
    cv2 = None

LABELS = ["person_A", "person_B", "person_C"]
LABEL_TO_INT = {name: i for i, name in enumerate(LABELS)}
INT_TO_LABEL = {i: name for name, i in LABEL_TO_INT.items()}


@dataclass
class Config:
    pkl: Path
    project_root: Path
    out: Path
    frames_root: Path
    fps: float = 1.0
    use_insightface: bool = False
    insightface_model: str = "buffalo_l"
    insightface_ctx_id: int = -1  # -1 CPU, 0 GPU
    insightface_det_size: Tuple[int, int] = (640, 640)
    min_arcface_coverage: float = 0.25
    two_large_sum_area: float = 0.55
    two_touching_gap: float = 0.035
    force_small_two_has_person2: bool = True
    use_first_single_p2_prior: bool = True
    first_single_p2_max_frame: int = 90
    first_single_p2_max_anchors: int = 40
    first_single_p2_min_confidence: float = 0.55
    label_spread_neighbors: int = 21
    pca_dims: int = 40
    constraint_first: bool = True
    early_single_p2_always: bool = True
    early_single_p2_stop_frame: int = 75
    early_single_p2_max_anchors: int = 80
    prototype_temperature: float = 2.25
    min_same_scale_anchors: int = 8
    low_confidence_threshold: float = 0.55
    random_state: int = 42
    max_debug_crops_per_person: int = 60
    annotate_every: int = 30
    save_debug_images: bool = True


def safe_json(obj: Any) -> str:
    def default(o: Any) -> Any:
        if isinstance(o, np.ndarray):
            return o.tolist()
        if isinstance(o, (np.integer,)):
            return int(o)
        if isinstance(o, (np.floating,)):
            return float(o)
        return str(o)
    return json.dumps(obj, default=default, ensure_ascii=False)


def patch_pandas_pickle_compat() -> None:
    """Patch a pandas/pyarrow compatibility edge case often seen with old pickles."""
    try:
        import importlib
        import pyarrow as pa  # noqa: F401
        for module_name in [
            "pandas.core.arrays.string_",
            "pandas.core.arrays.string_arrow",
            "pandas.core.arrays.arrow.array",
        ]:
            mod = importlib.import_module(module_name)
            if hasattr(mod, "pa_version_under10p1"):
                setattr(mod, "pa_version_under10p1", False)
            if module_name == "pandas.core.arrays.arrow.array":
                setattr(mod, "pa", pa)
        import pandas.core.arrays.string_ as string_mod

        def patched_init(self, storage=None, na_value=None):  # type: ignore
            if storage is None or storage not in {"python", "pyarrow", "pyarrow_numpy"}:
                storage = "python"
            self.storage = storage

        string_mod.StringDtype.__init__ = patched_init  # type: ignore[attr-defined]
    except Exception:
        # The normal reader may still work. Do not fail only because patching failed.
        pass


def load_visual_pickle(path: Path) -> pd.DataFrame:
    """Load the visual pickle and force it into a simple object DataFrame."""
    try:
        raw = pd.read_pickle(path)
    except Exception:
        patch_pandas_pickle_compat()
        raw = pd.read_pickle(path)

    if not isinstance(raw, pd.DataFrame):
        raise TypeError(f"Expected a pandas DataFrame in {path}, got {type(raw)}")

    # Convert extension/Arrow/string columns into plain object columns to avoid pickle-version surprises.
    data = {str(c): [v for v in raw[c].to_numpy(dtype=object)] for c in list(raw.columns)}
    df = pd.DataFrame(data)

    required = {"Frame", "Poses", "Fer"}
    missing = required - set(df.columns)
    if missing:
        raise ValueError(f"Missing expected columns {sorted(missing)}. Found columns: {list(df.columns)}")

    df["frame_num"] = df["Frame"].map(parse_frame_num)
    df = df.sort_values("frame_num").reset_index(drop=True)
    df["n_poses"] = df["Poses"].map(lambda x: len(x) if isinstance(x, list) else 0)
    df["n_faces"] = df["Fer"].map(lambda x: sum(1 for f in x if isinstance(f, dict)) if isinstance(x, list) else 0)
    return df


def parse_frame_num(frame_path: Any) -> int:
    m = re.search(r"frame[_-]?(\d+)", str(frame_path))
    return int(m.group(1)) if m else -1


def bbox_to_array(bbox: Any) -> Optional[np.ndarray]:
    try:
        arr = np.asarray(bbox, dtype=float).reshape(-1)
        if arr.size != 4 or not np.isfinite(arr).all():
            return None
        x1, y1, x2, y2 = arr.tolist()
        if x2 <= x1 or y2 <= y1:
            return None
        return np.array([x1, y1, x2, y2], dtype=float)
    except Exception:
        return None


def bbox_area(b: np.ndarray) -> float:
    return max(0.0, float(b[2] - b[0])) * max(0.0, float(b[3] - b[1]))


def bbox_center(b: np.ndarray) -> Tuple[float, float]:
    return float((b[0] + b[2]) / 2.0), float((b[1] + b[3]) / 2.0)


def bbox_iou(a: np.ndarray, b: np.ndarray) -> float:
    ix1, iy1 = max(a[0], b[0]), max(a[1], b[1])
    ix2, iy2 = min(a[2], b[2]), min(a[3], b[3])
    iw, ih = max(0.0, ix2 - ix1), max(0.0, iy2 - iy1)
    inter = iw * ih
    union = bbox_area(a) + bbox_area(b) - inter
    return 0.0 if union <= 0 else float(inter / union)


def center_inside(inner: np.ndarray, outer: np.ndarray) -> bool:
    cx, cy = bbox_center(inner)
    return bool(outer[0] <= cx <= outer[2] and outer[1] <= cy <= outer[3])


def dict_faces(faces: Any) -> List[Dict[str, Any]]:
    if not isinstance(faces, list):
        return []
    return [f for f in faces if isinstance(f, dict) and bbox_to_array(f.get("bbox")) is not None]


def dict_poses(poses: Any) -> List[Dict[str, Any]]:
    if not isinstance(poses, list):
        return []
    return [p for p in poses if isinstance(p, dict) and bbox_to_array(p.get("bbox")) is not None]


def infer_frame_size(df: pd.DataFrame, project_root: Path) -> Tuple[int, int]:
    """Infer frame width/height from an actual image if possible, otherwise from coordinates."""
    if cv2 is not None:
        for frame_path in df["Frame"].head(200):
            img_path = resolve_frame_path(project_root, Path("Frames"), frame_path)
            if img_path is not None:
                img = cv2.imread(str(img_path))
                if img is not None and img.size:
                    h, w = img.shape[:2]
                    return int(w), int(h)

    max_x, max_y = 0.0, 0.0
    for _, row in df.iterrows():
        for p in dict_poses(row["Poses"]):
            b = bbox_to_array(p.get("bbox"))
            if b is not None:
                max_x, max_y = max(max_x, b[2]), max(max_y, b[3])
        for f in dict_faces(row["Fer"]):
            b = bbox_to_array(f.get("bbox"))
            if b is not None:
                max_x, max_y = max(max_x, b[2]), max(max_y, b[3])
            lm = landmarks_array(f)
            if lm is not None and lm.size:
                max_x = max(max_x, float(np.nanmax(lm[:, 0])))
                max_y = max(max_y, float(np.nanmax(lm[:, 1])))
    # Snap to a typical 16:9 frame if coordinates are approximately that size.
    if 1200 <= max_x <= 1300 and 700 <= max_y <= 730:
        return 1280, 720
    return int(math.ceil(max_x)), int(math.ceil(max_y))


def resolve_frame_path(project_root: Path, frames_root: Path, frame_value: Any) -> Optional[Path]:
    s = str(frame_value)
    p = Path(s)
    candidates: List[Path] = []
    if p.is_absolute():
        candidates.append(p)
    candidates.append(project_root / p)

    # If pickle has Frames/debate/frame_001.jpg, this is usually enough:
    if len(p.parts) >= 2 and p.parts[0].lower() == "frames":
        candidates.append(project_root / p)
    else:
        candidates.append(project_root / frames_root / p.name)

    # Also try Frames/<debate_folder>/<filename> when only the basename is stored.
    if len(p.parts) >= 2:
        candidates.append(project_root / frames_root / p.parts[-2] / p.name)

    for c in candidates:
        if c.exists():
            return c
    return None


def landmarks_array(face: Dict[str, Any]) -> Optional[np.ndarray]:
    lm = face.get("landmarks")
    if lm is None:
        return None
    try:
        arr = np.asarray(lm, dtype=float)
        if arr.ndim == 1 and arr.size % 2 == 0:
            arr = arr.reshape((-1, 2))
        if arr.ndim != 2 or arr.shape[1] < 2:
            return None
        arr = arr[:, :2]
        if not np.isfinite(arr).any():
            return None
        return arr
    except Exception:
        return None


def match_face_to_pose(face_bbox: np.ndarray, poses: List[Dict[str, Any]]) -> Tuple[Optional[int], Optional[np.ndarray], float]:
    best_idx, best_bbox, best_score = None, None, -1.0
    for i, p in enumerate(poses):
        pb = bbox_to_array(p.get("bbox"))
        if pb is None:
            continue
        score = bbox_iou(face_bbox, pb)
        if center_inside(face_bbox, pb):
            score += 1.0
        if score > best_score:
            best_idx, best_bbox, best_score = i, pb, score
    if best_score < 0:
        return None, None, 0.0
    return best_idx, best_bbox, float(best_score)


def two_person_large_or_touching(pose_bboxes: Sequence[np.ndarray], width: int, height: int, cfg: Config) -> bool:
    if len(pose_bboxes) != 2:
        return False
    bbs = sorted(pose_bboxes, key=lambda b: bbox_center(b)[0])
    areas = [bbox_area(b) / max(1.0, width * height) for b in bbs]
    gap = (bbs[1][0] - bbs[0][2]) / max(1.0, width)
    return bool(sum(areas) >= cfg.two_large_sum_area or gap <= cfg.two_touching_gap)


def build_detection_table(df: pd.DataFrame, cfg: Config, width: int, height: int) -> pd.DataFrame:
    rows: List[Dict[str, Any]] = []
    for row_i, row in df.iterrows():
        faces = dict_faces(row["Fer"])
        poses = dict_poses(row["Poses"])
        pose_bboxes = [bbox_to_array(p.get("bbox")) for p in poses]
        pose_bboxes = [b for b in pose_bboxes if b is not None]
        is_two_large_touch = two_person_large_or_touching(pose_bboxes, width, height, cfg)
        # Sort face detections left to right for rule generation and stable face_idx.
        face_items = []
        for f in faces:
            fb = bbox_to_array(f.get("bbox"))
            if fb is not None:
                face_items.append((bbox_center(fb)[0], f, fb))
        face_items.sort(key=lambda x: x[0])

        for face_lr_idx, (_, f, fb) in enumerate(face_items):
            pose_idx, pose_bbox, pose_score = match_face_to_pose(fb, poses)
            lm = landmarks_array(f)
            probs = f.get("probabilities", {}) if isinstance(f.get("probabilities", {}), dict) else {}
            row_out: Dict[str, Any] = {
                "det_id": len(rows),
                "row_index": row_i,
                "frame": row["Frame"],
                "frame_num": int(row["frame_num"]),
                "face_idx_lr": int(face_lr_idx),
                "n_faces": int(row["n_faces"]),
                "n_poses": int(row["n_poses"]),
                "is_two_large_touch": bool(is_two_large_touch),
                "face_x1": fb[0], "face_y1": fb[1], "face_x2": fb[2], "face_y2": fb[3],
                "face_cx": bbox_center(fb)[0], "face_cy": bbox_center(fb)[1],
                "face_area_norm": bbox_area(fb) / max(1.0, width * height),
                "face_aspect": (fb[2] - fb[0]) / max(1.0, fb[3] - fb[1]),
                "top_emotion": f.get("top_emotion", ""),
                "emotion_probs_json": safe_json(probs),
                "landmarks": lm,
                "pose_idx": -1 if pose_idx is None else int(pose_idx),
                "pose_match_score": pose_score,
                "pose_bbox": pose_bbox,
                "pose_keypoints": None,
                "weak_label": -1,
                "weak_source": "",
            }
            if pose_idx is not None and 0 <= pose_idx < len(poses):
                p = poses[pose_idx]
                pk = p.get("pose")
                try:
                    row_out["pose_keypoints"] = np.asarray(pk, dtype=float)
                except Exception:
                    row_out["pose_keypoints"] = None
            rows.append(row_out)

    det = pd.DataFrame(rows)
    if det.empty:
        raise ValueError("No usable face detections found in the pickle.")
    return det


def normalized_landmark_feature(lm: Optional[np.ndarray], bbox: np.ndarray) -> np.ndarray:
    """Return normalized 106x2 landmarks flattened. Missing values become NaN."""
    target_points = 106
    out = np.full((target_points, 2), np.nan, dtype=float)
    if lm is None or lm.size == 0:
        return out.reshape(-1)
    n = min(target_points, lm.shape[0])
    x1, y1, x2, y2 = bbox
    scale = max(1.0, max(x2 - x1, y2 - y1))
    norm = (lm[:n, :2] - np.array([(x1 + x2) / 2.0, (y1 + y2) / 2.0])) / scale
    out[:n] = norm
    return out.reshape(-1)


def pose_feature(pose_keypoints: Optional[np.ndarray], pose_bbox: Optional[np.ndarray]) -> np.ndarray:
    """Return normalized 17x3 pose features. Missing values become NaN."""
    out = np.full((17, 3), np.nan, dtype=float)
    if pose_keypoints is None or pose_bbox is None:
        return out.reshape(-1)
    arr = np.asarray(pose_keypoints, dtype=float)
    if arr.ndim != 2 or arr.shape[0] < 1:
        return out.reshape(-1)
    n = min(17, arr.shape[0])
    x1, y1, x2, y2 = pose_bbox
    w, h = max(1.0, x2 - x1), max(1.0, y2 - y1)
    out[:n, 0] = (arr[:n, 0] - x1) / w
    out[:n, 1] = (arr[:n, 1] - y1) / h
    if arr.shape[1] >= 3:
        out[:n, 2] = arr[:n, 2]
    return out.reshape(-1)


def head_orientation_features(lm: Optional[np.ndarray], bbox: np.ndarray) -> Dict[str, float]:
    """Approximate head direction from the 106-point landmarks without relying on exact index names.

    These are weak descriptive features only; they are not used as hard identity rules.
    """
    if lm is None or lm.shape[0] < 20:
        return {"yaw_proxy": np.nan, "down_proxy": np.nan, "frontal_proxy": np.nan}
    x1, y1, x2, y2 = bbox
    w, h = max(1.0, x2 - x1), max(1.0, y2 - y1)
    xs = (lm[:, 0] - x1) / w
    ys = (lm[:, 1] - y1) / h
    # Robust proxy: compare horizontal landmark mass around the centre. Positive means more mass right.
    medx = float(np.nanmedian(xs))
    yaw_proxy = medx - 0.5
    # More lower-half landmark mass often indicates head tilted/down in these 2D landmarks.
    down_proxy = float(np.nanmedian(ys) - 0.5)
    # Frontal faces tend to have balanced left/right landmark spread.
    left_spread = float(np.nanpercentile(xs, 50) - np.nanpercentile(xs, 10))
    right_spread = float(np.nanpercentile(xs, 90) - np.nanpercentile(xs, 50))
    frontal_proxy = -abs(left_spread - right_spread)
    return {"yaw_proxy": yaw_proxy, "down_proxy": down_proxy, "frontal_proxy": frontal_proxy}


def create_base_features(det: pd.DataFrame, width: int, height: int) -> Tuple[np.ndarray, List[str], pd.DataFrame]:
    features: List[np.ndarray] = []
    names: List[str] = []
    aux_rows: List[Dict[str, float]] = []

    # Names for fixed-size blocks.
    lm_names = [f"lm_{i}_{axis}" for i in range(106) for axis in ("x", "y")]
    pose_names = [f"pose_{i}_{axis}" for i in range(17) for axis in ("x", "y", "conf")]
    geom_names = [
        "face_area_norm", "face_aspect", "face_width_norm", "face_height_norm",
        "pose_area_norm", "pose_aspect", "yaw_proxy", "down_proxy", "frontal_proxy",
    ]
    names = lm_names + pose_names + geom_names

    for _, r in det.iterrows():
        fb = np.array([r.face_x1, r.face_y1, r.face_x2, r.face_y2], dtype=float)
        lm = r.landmarks if isinstance(r.landmarks, np.ndarray) else None
        lm_feat = normalized_landmark_feature(lm, fb)
        pb = r.pose_bbox if isinstance(r.pose_bbox, np.ndarray) else None
        pk = r.pose_keypoints if isinstance(r.pose_keypoints, np.ndarray) else None
        p_feat = pose_feature(pk, pb)
        orient = head_orientation_features(lm, fb)
        pose_area = np.nan
        pose_aspect = np.nan
        if pb is not None:
            pose_area = bbox_area(pb) / max(1.0, width * height)
            pose_aspect = (pb[2] - pb[0]) / max(1.0, pb[3] - pb[1])
        geom = np.array([
            r.face_area_norm,
            r.face_aspect,
            (r.face_x2 - r.face_x1) / max(1.0, width),
            (r.face_y2 - r.face_y1) / max(1.0, height),
            pose_area,
            pose_aspect,
            orient["yaw_proxy"],
            orient["down_proxy"],
            orient["frontal_proxy"],
        ], dtype=float)
        features.append(np.concatenate([lm_feat, p_feat, geom]))
        aux_rows.append(orient)
    aux = pd.DataFrame(aux_rows)
    X = np.vstack(features).astype(float)
    return X, names, aux


def extract_insightface_embeddings(det: pd.DataFrame, cfg: Config) -> Optional[np.ndarray]:
    """Extract ArcFace embeddings by matching InsightFace detections to pickle FER boxes."""
    if not cfg.use_insightface:
        return None
    if cv2 is None:
        warnings.warn("opencv-python is not installed; cannot read frames for InsightFace.")
        return None
    try:
        from insightface.app import FaceAnalysis
    except Exception as e:
        warnings.warn(f"InsightFace is not installed or cannot import: {e}")
        return None

    providers = ["CPUExecutionProvider"]
    try:
        app = FaceAnalysis(name=cfg.insightface_model, providers=providers)
        app.prepare(ctx_id=cfg.insightface_ctx_id, det_size=cfg.insightface_det_size)
    except Exception as e:
        warnings.warn(
            f"Could not initialize InsightFace model '{cfg.insightface_model}'. "
            f"If this is first run, connect to the internet so the model can download. Error: {e}"
        )
        return None

    embeddings = np.full((len(det), 512), np.nan, dtype=float)
    grouped = det.groupby("frame", sort=False)
    for frame_value, idxs in tqdm(grouped.groups.items(), desc="InsightFace embeddings"):
        img_path = resolve_frame_path(cfg.project_root, cfg.frames_root, frame_value)
        if img_path is None:
            continue
        img = cv2.imread(str(img_path))
        if img is None:
            continue
        try:
            faces = app.get(img)
        except Exception:
            continue
        if not faces:
            continue
        iboxes = []
        iembs = []
        for f in faces:
            if not hasattr(f, "bbox") or not hasattr(f, "normed_embedding"):
                continue
            b = bbox_to_array(getattr(f, "bbox"))
            emb = np.asarray(getattr(f, "normed_embedding"), dtype=float).reshape(-1)
            if b is not None and emb.size > 0:
                iboxes.append(b)
                iembs.append(emb)
        if not iboxes:
            continue
        for det_idx in idxs:
            r = det.loc[det_idx]
            fb = np.array([r.face_x1, r.face_y1, r.face_x2, r.face_y2], dtype=float)
            scores = [bbox_iou(fb, ib) for ib in iboxes]
            best = int(np.argmax(scores))
            if scores[best] >= 0.05 or center_inside(fb, iboxes[best]) or center_inside(iboxes[best], fb):
                emb = iembs[best]
                if emb.size != embeddings.shape[1]:
                    # Rare: model returns a different dimensionality. Resize matrix once.
                    new = np.full((len(det), emb.size), np.nan, dtype=float)
                    upto = min(embeddings.shape[1], emb.size)
                    new[:, :upto] = embeddings[:, :upto]
                    embeddings = new
                embeddings[det_idx, :emb.size] = emb

    coverage = float(np.isfinite(embeddings).all(axis=1).mean())
    print(f"InsightFace embedding coverage: {coverage:.1%}")
    if coverage < cfg.min_arcface_coverage:
        warnings.warn(
            f"InsightFace coverage {coverage:.1%} is below min_arcface_coverage={cfg.min_arcface_coverage:.1%}. "
            "Falling back to landmark/pose features."
        )
        return None
    return embeddings


def build_model_features(det: pd.DataFrame, base_X: np.ndarray, arc_X: Optional[np.ndarray], cfg: Config) -> np.ndarray:
    """Create the final numeric model matrix."""
    # Impute and scale landmark/pose features.
    base = SimpleImputer(strategy="median").fit_transform(base_X)
    base = StandardScaler().fit_transform(base)

    if arc_X is not None:
        # Use ArcFace as primary identity signal. Fill rare missing embeddings with median.
        arc = SimpleImputer(strategy="median").fit_transform(arc_X)
        arc = Normalizer(norm="l2").fit_transform(arc)
        # Add a small amount of landmark/pose information to help when faces are profile/low quality.
        X = np.hstack([arc * 3.0, base * 0.35])
    else:
        X = base

    # Reduce dimensionality for graph propagation stability and speed.
    n_components = min(cfg.pca_dims, X.shape[1], max(2, X.shape[0] - 1))
    if n_components >= 2 and X.shape[1] > n_components:
        X = PCA(n_components=n_components, random_state=cfg.random_state).fit_transform(X)
    return X


def assign_weak_labels(det: pd.DataFrame, cfg: Config) -> pd.DataFrame:
    """Pseudo-label reliable frames from camera geometry.

    This version is deliberately constraint-heavy:
    - 3 visible faces/people: left-to-right is person_A/person_B/person_C.
    - 2 large/touching people: left/right is person_A/person_C.
    - early single close-ups: default soft anchor for person_B, because in these debate
      videos the moderator usually appears first in single-camera shots.

    The early-single anchors do not overwrite hard multi-person anchors.
    """
    det = det.copy()
    det["weak_label"] = -1
    det["weak_source"] = ""
    det["weak_weight"] = 0.0

    # Hard anchor 1: exactly three faces in wide shots => left-to-right is person A/2/3.
    for frame, group in det.groupby("frame", sort=False):
        if int(group["n_faces"].iloc[0]) == 3 and len(group) == 3:
            ordered = group.sort_values("face_cx")
            for lab, idx in enumerate(ordered.index.tolist()):
                det.at[idx, "weak_label"] = lab
                det.at[idx, "weak_source"] = "3_faces_left_to_right"
                det.at[idx, "weak_weight"] = 1.0

    # Hard anchor 2: two large/touching bodies => person A left, person C right.
    for frame, group in det.groupby("frame", sort=False):
        if int(group["n_faces"].iloc[0]) == 2 and len(group) == 2 and bool(group["is_two_large_touch"].iloc[0]):
            ordered = group.sort_values("face_cx")
            left_idx, right_idx = ordered.index.tolist()
            det.at[left_idx, "weak_label"] = LABEL_TO_INT["person_A"]
            det.at[left_idx, "weak_source"] = "2_large_or_touching_left_right"
            det.at[left_idx, "weak_weight"] = 1.0
            det.at[right_idx, "weak_label"] = LABEL_TO_INT["person_C"]
            det.at[right_idx, "weak_source"] = "2_large_or_touching_left_right"
            det.at[right_idx, "weak_weight"] = 1.0

    # Soft but useful anchor 3: early single close-ups are usually person_B.
    # In the previous version this only activated if person_B anchors were scarce, but person_B
    # anchors from wide shots are tiny faces and are bad prototypes for close-ups. Therefore we
    # always add a limited number of early single close-up anchors unless disabled.
    if cfg.use_first_single_p2_prior and cfg.early_single_p2_always:
        candidates = det[
            (det["n_faces"] == 1)
            & (det["weak_label"] < 0)
            & (det["frame_num"] <= cfg.early_single_p2_stop_frame)
        ].sort_values("frame_num")
        for idx in candidates.head(cfg.early_single_p2_max_anchors).index:
            det.at[idx, "weak_label"] = LABEL_TO_INT["person_B"]
            det.at[idx, "weak_source"] = "early_single_prior_person_B"
            det.at[idx, "weak_weight"] = 0.70

    return det


def softmax_neg_distance(dist: np.ndarray, temperature: float) -> np.ndarray:
    d = np.asarray(dist, dtype=float)
    t = max(1e-6, float(temperature))
    z = -d / t
    z = z - np.nanmax(z, axis=1, keepdims=True)
    e = np.exp(z)
    s = e.sum(axis=1, keepdims=True)
    return np.divide(e, s, out=np.ones_like(e) / e.shape[1], where=s > 0)


def face_scale_bin(area: float) -> str:
    """Rough shot-scale bin from normalized face area."""
    if not np.isfinite(area):
        return "unknown"
    if area < 0.005:
        return "wide"
    if area < 0.018:
        return "medium"
    return "close"


def weighted_mean(X: np.ndarray, w: np.ndarray) -> np.ndarray:
    w = np.asarray(w, dtype=float)
    w = np.where(np.isfinite(w) & (w > 0), w, 1.0)
    return np.average(X, axis=0, weights=w)


def fit_prototype_identity_model(X: np.ndarray, det: pd.DataFrame, cfg: Config) -> Tuple[np.ndarray, np.ndarray, str]:
    """Constraint-first identity probabilities using anchor prototypes.

    Unlike LabelSpreading, this does not let a graph model dominate the rules. It builds one
    prototype per person from the reliable anchors and, when possible, uses prototypes from the
    same shot scale. This matters because wide-shot faces and close-up faces have very different
    landmark/pose noise profiles.
    """
    y = det["weak_label"].to_numpy(dtype=int)
    weights = det.get("weak_weight", pd.Series(np.where(y >= 0, 1.0, 0.0), index=det.index)).to_numpy(dtype=float)
    scale_bins = det["face_area_norm"].map(face_scale_bin).to_numpy()
    probs = np.zeros((len(det), 3), dtype=float)

    # Need at least some anchors for every label. If not, fall back to the old model.
    if len(set(y[y >= 0].tolist())) < 3:
        return fit_identity_model(X, det, cfg)

    global_proto: Dict[int, np.ndarray] = {}
    scale_proto: Dict[Tuple[int, str], np.ndarray] = {}
    for lab in range(3):
        m = y == lab
        if not np.any(m):
            continue
        global_proto[lab] = weighted_mean(X[m], weights[m])
        for sb in ["wide", "medium", "close"]:
            ms = m & (scale_bins == sb)
            if int(ms.sum()) >= cfg.min_same_scale_anchors:
                scale_proto[(lab, sb)] = weighted_mean(X[ms], weights[ms])

    # Distances to global and same-scale prototypes. Same-scale gets more influence.
    all_dist = np.zeros((len(det), 3), dtype=float)
    for i in range(len(det)):
        sb = scale_bins[i]
        for lab in range(3):
            gp = global_proto.get(lab)
            sp = scale_proto.get((lab, sb))
            if gp is None and sp is None:
                all_dist[i, lab] = 1e6
            elif sp is not None and gp is not None:
                all_dist[i, lab] = 0.70 * np.linalg.norm(X[i] - sp) + 0.30 * np.linalg.norm(X[i] - gp)
            elif sp is not None:
                all_dist[i, lab] = np.linalg.norm(X[i] - sp)
            else:
                all_dist[i, lab] = np.linalg.norm(X[i] - gp)

    probs = softmax_neg_distance(all_dist, cfg.prototype_temperature)

    # Anchors should remain anchors. Use near-one-hot probabilities so frame constraints cannot
    # accidentally flip them unless another hard frame rule applies.
    anchor = y >= 0
    for i in np.where(anchor)[0]:
        lab = int(y[i])
        w = float(weights[i]) if np.isfinite(weights[i]) else 1.0
        lock = min(0.995, 0.70 + 0.28 * w)
        probs[i, :] = (1.0 - lock) / 2.0
        probs[i, lab] = lock

    pred = probs.argmax(axis=1)
    return probs, pred, "ConstraintFirstPrototype(same-scale anchors)"

def fit_identity_model(X: np.ndarray, det: pd.DataFrame, cfg: Config) -> Tuple[np.ndarray, np.ndarray, str]:
    """Return per-detection class probabilities, predicted labels, and model name."""
    y = det["weak_label"].to_numpy(dtype=int)
    present = sorted(set(y[y >= 0].tolist()))

    if len(present) == 3 and min([(y == k).sum() for k in present]) >= 3:
        n_neighbors = int(min(cfg.label_spread_neighbors, max(3, len(X) - 1)))
        model = LabelSpreading(kernel="knn", n_neighbors=n_neighbors, alpha=0.20, max_iter=60, n_jobs=-1)
        model.fit(X, y)
        # classes_ should be [0,1,2], but align defensively.
        probs = np.zeros((len(X), 3), dtype=float)
        for col_i, cls in enumerate(model.classes_):
            if int(cls) in (0, 1, 2):
                probs[:, int(cls)] = model.label_distributions_[:, col_i]
        row_sum = probs.sum(axis=1, keepdims=True)
        probs = np.divide(probs, row_sum, out=np.ones_like(probs) / 3.0, where=row_sum > 0)
        pred = probs.argmax(axis=1)
        return probs, pred, "LabelSpreading(knn)"

    # Fallback: exactly 3 clusters, then map clusters to person labels using available anchors.
    km = KMeans(n_clusters=3, n_init=50, random_state=cfg.random_state)
    cl = km.fit_predict(X)
    mapping: Dict[int, int] = {}
    for c in range(3):
        anchor_labels = y[(cl == c) & (y >= 0)]
        if anchor_labels.size:
            vals, counts = np.unique(anchor_labels, return_counts=True)
            mapping[c] = int(vals[np.argmax(counts)])
    # If any clusters remain unmapped, map them by median x position in 3-face frames.
    unused_labels = [i for i in range(3) if i not in mapping.values()]
    unused_clusters = [c for c in range(3) if c not in mapping]
    if unused_clusters:
        med_x = {c: float(np.nanmedian(det.loc[cl == c, "face_cx"])) for c in unused_clusters}
        for c, lab in zip(sorted(unused_clusters, key=lambda cc: med_x[cc]), unused_labels):
            mapping[c] = lab
    pred = np.array([mapping[int(c)] for c in cl], dtype=int)
    # Soft-ish probabilities from distances to centroids.
    d = km.transform(X)
    inv = 1.0 / np.maximum(d, 1e-6)
    probs_by_cluster = inv / inv.sum(axis=1, keepdims=True)
    probs = np.zeros((len(X), 3), dtype=float)
    for c in range(3):
        probs[:, mapping[c]] += probs_by_cluster[:, c]
    return probs, pred, "KMeans(3)+anchor mapping"


def best_distinct_assignment(probs: np.ndarray, allowed_label_sets: Optional[List[Tuple[int, ...]]] = None) -> Tuple[List[int], List[float]]:
    """Assign distinct labels to detections in one frame using probabilities."""
    n = probs.shape[0]
    eps = 1e-9
    if n == 1:
        lab = int(np.argmax(probs[0]))
        return [lab], [float(probs[0, lab])]

    if allowed_label_sets is None:
        allowed_label_sets = list(permutations(range(3), min(n, 3)))
    best_labs, best_score = None, float("inf")
    for labs in allowed_label_sets:
        if len(labs) != min(n, 3):
            continue
        score = 0.0
        for i, lab in enumerate(labs):
            score += -math.log(float(probs[i, lab]) + eps)
        if score < best_score:
            best_score = score
            best_labs = labs
    labs_list = list(best_labs) if best_labs is not None else [int(np.argmax(p)) for p in probs]
    confs = [float(probs[i, labs_list[i]]) for i in range(len(labs_list))]
    if n > 3:
        # Extra detections are likely false positives. Assign their max label but mark later as low confidence.
        for i in range(3, n):
            lab = int(np.argmax(probs[i]))
            labs_list.append(lab)
            confs.append(float(probs[i, lab]) * 0.5)
    return labs_list, confs


def apply_frame_constraints(det: pd.DataFrame, model_probs: np.ndarray, cfg: Config) -> pd.DataFrame:
    """Apply hard debate-camera rules and one-frame duplicate prevention."""
    out = det.copy()
    out["model_person"] = [INT_TO_LABEL[int(i)] for i in model_probs.argmax(axis=1)]
    out["model_confidence"] = model_probs.max(axis=1)
    out["person_label"] = out["model_person"]
    out["confidence"] = out["model_confidence"]
    out["assignment_source"] = "model"
    for k, name in enumerate(LABELS):
        out[f"prob_{name}"] = model_probs[:, k]

    for frame, group in out.groupby("frame", sort=False):
        idxs = group.sort_values("face_cx").index.tolist()
        n = len(idxs)
        if n == 0:
            continue
        n_faces = int(group["n_faces"].iloc[0])
        is_two_large_touch = bool(group["is_two_large_touch"].iloc[0])

        if n_faces == 3 and n == 3:
            labs = [0, 1, 2]
            source = "hard_rule_3_faces_left_to_right"
            confs = [1.0, 1.0, 1.0]
        elif n_faces == 2 and n == 2 and is_two_large_touch:
            labs = [0, 2]
            source = "hard_rule_2_large_or_touching_left_right"
            confs = [1.0, 1.0]
        elif n_faces == 2 and n == 2 and cfg.force_small_two_has_person2:
            # User rule: if two people are smaller, one is person_B and the other is uncertain.
            # Let the model decide which side is person_B and whether the other is person_A or person_C.
            allowed = [(1, 0), (1, 2), (0, 1), (2, 1)]
            probs = model_probs[idxs]
            labs, confs = best_distinct_assignment(probs, allowed)
            source = "soft_rule_2_small_one_is_person_B"
        elif n >= 2:
            probs = model_probs[idxs]
            labs, confs = best_distinct_assignment(probs)
            source = "model_distinct_frame_assignment"
        else:
            probs = model_probs[idxs]
            labs = [int(np.argmax(probs[0]))]
            confs = [float(np.max(probs[0]))]
            source = "model_single"

        for idx, lab, conf in zip(idxs, labs, confs):
            out.at[idx, "person_label"] = INT_TO_LABEL[int(lab)]
            out.at[idx, "confidence"] = float(conf)
            out.at[idx, "assignment_source"] = source

    return out


def smooth_single_person_runs(out: pd.DataFrame) -> pd.DataFrame:
    """Temporal smoothing for consecutive single-person shots."""
    out = out.sort_values(["frame_num", "face_idx_lr"]).copy()
    single = out[out["n_faces"] == 1].copy()
    if single.empty:
        return out

    # Consecutive single-face frames with no gap > 2 seconds form a run.
    run_id = []
    current = -1
    prev = None
    for fn in single["frame_num"].tolist():
        if prev is None or fn - prev > 2:
            current += 1
        run_id.append(current)
        prev = fn
    single["single_run"] = run_id

    for _, run in single.groupby("single_run"):
        if len(run) < 3:
            continue
        probs = run[[f"prob_{name}" for name in LABELS]].to_numpy(dtype=float)
        mean_probs = probs.mean(axis=0)
        lab = int(np.argmax(mean_probs))
        run_conf = float(mean_probs[lab])
        # Only smooth if the run has a reasonably clear majority and not a hard-rule frame.
        if run_conf >= 0.45:
            for idx in run.index:
                if str(out.at[idx, "assignment_source"]).startswith("hard_rule"):
                    continue
                out.at[idx, "person_label"] = INT_TO_LABEL[lab]
                out.at[idx, "confidence"] = max(float(out.at[idx, "confidence"]), run_conf)
                out.at[idx, "assignment_source"] = "temporal_single_run_smoothing"
    return out


def make_outputs(out: pd.DataFrame, df: pd.DataFrame, X: np.ndarray, cfg: Config, width: int, height: int, model_name: str) -> None:
    cfg.out.mkdir(parents=True, exist_ok=True)

    export = out.copy()
    for col in ["landmarks", "pose_bbox", "pose_keypoints"]:
        if col in export.columns:
            export[col] = export[col].map(lambda x: safe_json(x) if x is not None else "")
    for c in ["face_x1", "face_y1", "face_x2", "face_y2", "face_cx", "face_cy", "face_area_norm", "confidence", "model_confidence"]:
        if c in export.columns:
            export[c] = export[c].astype(float).round(6)
    export["review_needed"] = (export["confidence"].astype(float) < cfg.low_confidence_threshold).astype(int)
    export.to_csv(cfg.out / "visual_identity_predictions.csv", index=False)

    # Low-confidence frames to inspect manually first. This is useful because only a few uncertain
    # single-person runs usually determine whether the solution looks correct.
    review_cols = ["frame", "frame_num", "face_idx_lr", "person_label", "confidence", "assignment_source", "model_person", "model_confidence"]
    available_review_cols = [c for c in review_cols if c in export.columns]
    export.loc[export["review_needed"] == 1, available_review_cols].sort_values(["frame_num", "face_idx_lr"]).to_csv(
        cfg.out / "visual_identity_low_confidence_review.csv", index=False
    )

    # Per-frame summary.
    frame_rows: List[Dict[str, Any]] = []
    for frame, group in out.groupby("frame", sort=False):
        labels = sorted(set(group["person_label"].tolist()), key=lambda s: LABEL_TO_INT.get(s, 99))
        row = {
            "frame": frame,
            "frame_num": int(group["frame_num"].iloc[0]),
            "n_faces": int(group["n_faces"].iloc[0]),
            "n_poses": int(group["n_poses"].iloc[0]),
            "labels_present": ",".join(labels),
            "shot_type": shot_type(group),
        }
        for lab in LABELS:
            row[f"has_{lab}"] = int(lab in labels)
        frame_rows.append(row)
    frame_summary = pd.DataFrame(frame_rows).sort_values("frame_num")
    frame_summary.to_csv(cfg.out / "visual_identity_frame_summary.csv", index=False)

    # Consecutive single-person runs. These are the few places where automatic identity is hardest,
    # so this file is usually the fastest way to check whether the result looks right.
    single_runs = []
    singles = out[out["n_faces"] == 1].sort_values("frame_num").copy()
    if not singles.empty:
        run_id = -1
        prev_fn = None
        for idx, rr in singles.iterrows():
            fn = int(rr["frame_num"])
            if prev_fn is None or fn - prev_fn > 2:
                run_id += 1
            singles.at[idx, "single_run"] = run_id
            prev_fn = fn
        for rid, rg in singles.groupby("single_run"):
            label_counts = rg["person_label"].value_counts().to_dict()
            dominant = max(label_counts, key=label_counts.get)
            single_runs.append({
                "single_run": int(rid),
                "start_frame_num": int(rg["frame_num"].min()),
                "end_frame_num": int(rg["frame_num"].max()),
                "n_frames": int(len(rg)),
                "dominant_label": dominant,
                "dominant_share": float(label_counts[dominant] / max(1, len(rg))),
                "mean_confidence": float(rg["confidence"].mean()),
                "assignment_sources": ";".join(sorted(set(map(str, rg["assignment_source"].tolist())))),
                "label_counts": safe_json(label_counts),
            })
    pd.DataFrame(single_runs).to_csv(cfg.out / "visual_identity_single_runs.csv", index=False)

    # Screen-time summary: at 1 fps this is approximately seconds; divide by fps in case frame rate differs.
    person_rows = []
    for lab in LABELS:
        frames = frame_summary.loc[frame_summary[f"has_{lab}"] == 1, "frame_num"].nunique()
        detections = int((out["person_label"] == lab).sum())
        mean_conf = float(out.loc[out["person_label"] == lab, "confidence"].mean()) if detections else np.nan
        person_rows.append({
            "person_label": lab,
            "face_detections": detections,
            "unique_frames_seen": int(frames),
            "approx_screen_seconds": float(frames / cfg.fps),
            "mean_confidence": mean_conf,
        })
    person_summary = pd.DataFrame(person_rows)
    total_seen = person_summary["unique_frames_seen"].sum()
    person_summary["screen_share_of_labelled_faces"] = person_summary["unique_frames_seen"] / max(1, total_seen)
    person_summary.to_csv(cfg.out / "visual_identity_person_summary.csv", index=False)

    anchors = out[out["weak_label"] >= 0].copy()
    anchors["weak_person"] = anchors["weak_label"].map(INT_TO_LABEL)
    anchors[["frame", "frame_num", "face_idx_lr", "weak_person", "weak_source"]].to_csv(cfg.out / "visual_identity_weak_anchors.csv", index=False)

    metadata = {
        "model": model_name,
        "n_frames_in_pickle": int(len(df)),
        "n_face_detections": int(len(out)),
        "frame_width": int(width),
        "frame_height": int(height),
        "weak_anchor_counts": {INT_TO_LABEL[int(k)]: int(v) for k, v in out.loc[out["weak_label"] >= 0, "weak_label"].value_counts().sort_index().items()},
        "assignment_source_counts": out["assignment_source"].value_counts().to_dict(),
        "outputs": [
            "visual_identity_predictions.csv",
            "visual_identity_frame_summary.csv",
            "visual_identity_person_summary.csv",
            "visual_identity_weak_anchors.csv",
            "visual_identity_low_confidence_review.csv",
            "visual_identity_single_runs.csv",
        ],
        "notes": [
            "person_A/person_B/person_C are debate-relative labels, not real names.",
            "Hard rules are generated from reliable multi-person camera compositions; ambiguous frames use model probabilities and constraints.",
        ],
    }
    (cfg.out / "visual_identity_metadata.json").write_text(json.dumps(metadata, indent=2, ensure_ascii=False), encoding="utf-8")

    save_plots(out, X, cfg)
    if cfg.save_debug_images and cv2 is not None:
        save_debug_images(out, cfg)


def shot_type(group: pd.DataFrame) -> str:
    n = len(group)
    n_faces = int(group["n_faces"].iloc[0])
    if n_faces == 3 and n == 3:
        return "three_people_wide"
    if n_faces == 2 and n == 2 and bool(group["is_two_large_touch"].iloc[0]):
        return "two_people_large_or_touching"
    if n_faces == 2 and n == 2:
        return "two_people_small_or_uncertain"
    if n_faces == 1 and n == 1:
        return "single_closeup"
    return "other_or_noisy"


def save_plots(out: pd.DataFrame, X: np.ndarray, cfg: Config) -> None:
    try:
        import matplotlib.pyplot as plt
    except Exception:
        return
    plot_dir = cfg.out / "plots"
    plot_dir.mkdir(parents=True, exist_ok=True)

    counts = out["person_label"].value_counts().reindex(LABELS).fillna(0)
    plt.figure(figsize=(6, 4))
    counts.plot(kind="bar")
    plt.title("Detected face assignments by person")
    plt.ylabel("Face detections")
    plt.tight_layout()
    plt.savefig(plot_dir / "person_detection_counts.png", dpi=150)
    plt.close()

    if X.shape[0] >= 3:
        xy = PCA(n_components=2, random_state=cfg.random_state).fit_transform(X)
        plt.figure(figsize=(7, 5))
        for lab in LABELS:
            mask = out["person_label"].to_numpy() == lab
            plt.scatter(xy[mask, 0], xy[mask, 1], s=8, alpha=0.65, label=lab)
        plt.title("2D PCA of identity features")
        plt.xlabel("PC1")
        plt.ylabel("PC2")
        plt.legend(markerscale=2)
        plt.tight_layout()
        plt.savefig(plot_dir / "identity_feature_pca.png", dpi=150)
        plt.close()

    # Confidence over time.
    tmp = out.groupby("frame_num")["confidence"].mean().reset_index()
    plt.figure(figsize=(9, 4))
    plt.plot(tmp["frame_num"], tmp["confidence"])
    plt.title("Mean assignment confidence over time")
    plt.xlabel("Frame number / second")
    plt.ylabel("Mean confidence")
    plt.tight_layout()
    plt.savefig(plot_dir / "confidence_over_time.png", dpi=150)
    plt.close()


def save_debug_images(out: pd.DataFrame, cfg: Config) -> None:
    img_dir = cfg.out / "debug_annotated_frames"
    crop_dir = cfg.out / "debug_face_crops"
    img_dir.mkdir(parents=True, exist_ok=True)
    for lab in LABELS:
        (crop_dir / lab).mkdir(parents=True, exist_ok=True)

    # Annotated frames: every Nth frame plus hard-rule frames.
    frame_groups = list(out.groupby("frame", sort=False))
    for i, (frame_value, group) in enumerate(tqdm(frame_groups, desc="Saving annotated debug frames")):
        if cfg.annotate_every <= 0:
            break
        if i % cfg.annotate_every != 0 and not str(group["assignment_source"].iloc[0]).startswith("hard_rule"):
            continue
        img_path = resolve_frame_path(cfg.project_root, cfg.frames_root, frame_value)
        if img_path is None:
            continue
        img = cv2.imread(str(img_path))
        if img is None:
            continue
        for _, r in group.iterrows():
            x1, y1, x2, y2 = map(int, [r.face_x1, r.face_y1, r.face_x2, r.face_y2])
            cv2.rectangle(img, (x1, y1), (x2, y2), (0, 255, 0), 2)
            text = f"{r.person_label} {float(r.confidence):.2f}"
            cv2.putText(img, text, (x1, max(20, y1 - 6)), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 0), 2)
        fn = parse_frame_num(frame_value)
        cv2.imwrite(str(img_dir / f"frame_{fn:06d}.jpg"), img)

    # Save high-confidence crops per person for quick manual QA.
    crop_counts = {lab: 0 for lab in LABELS}
    ordered = out.sort_values("confidence", ascending=False)
    for _, r in ordered.iterrows():
        lab = str(r.person_label)
        if lab not in crop_counts or crop_counts[lab] >= cfg.max_debug_crops_per_person:
            continue
        img_path = resolve_frame_path(cfg.project_root, cfg.frames_root, r.frame)
        if img_path is None:
            continue
        img = cv2.imread(str(img_path))
        if img is None:
            continue
        h, w = img.shape[:2]
        x1, y1, x2, y2 = [int(v) for v in [r.face_x1, r.face_y1, r.face_x2, r.face_y2]]
        pad = int(0.20 * max(x2 - x1, y2 - y1))
        x1, y1 = max(0, x1 - pad), max(0, y1 - pad)
        x2, y2 = min(w, x2 + pad), min(h, y2 + pad)
        if x2 <= x1 or y2 <= y1:
            continue
        crop = img[y1:y2, x1:x2]
        out_name = crop_dir / lab / f"frame_{int(r.frame_num):06d}_conf_{float(r.confidence):.2f}.jpg"
        cv2.imwrite(str(out_name), crop)
        crop_counts[lab] += 1
        if all(v >= cfg.max_debug_crops_per_person for v in crop_counts.values()):
            break


def main() -> None:
    parser = argparse.ArgumentParser(description="No-glasses debate person identity assignment from *_visual.pkl")
    parser.add_argument("--pkl", required=True, type=Path, help="Path to *_visual.pkl")
    parser.add_argument("--project-root", default=Path("."), type=Path, help="Project root, e.g. Documents/big_data")
    parser.add_argument("--frames-root", default=Path("Frames"), type=Path, help="Frames folder relative to project root")
    parser.add_argument("--out", default=Path("outputs/visual_identity_A_B_C"), type=Path, help="Output folder")
    parser.add_argument("--fps", default=1.0, type=float, help="Frame sampling rate; assignment says 1 fps")
    parser.add_argument("--use-insightface", action="store_true", help="Use InsightFace ArcFace embeddings from actual frame images")
    parser.add_argument("--insightface-model", default="buffalo_l", help="InsightFace model pack, e.g. buffalo_l")
    parser.add_argument("--insightface-ctx-id", default=-1, type=int, help="-1 CPU, 0 GPU")
    parser.add_argument("--two-large-sum-area", default=0.55, type=float, help="Two-person body area threshold for person_A/person_C rule")
    parser.add_argument("--two-touching-gap", default=0.035, type=float, help="Normalized horizontal gap threshold for touching/near bodies")
    parser.add_argument("--no-small-two-person2", action="store_true", help="Do not enforce that small 2-person frames contain person_B")
    parser.add_argument("--no-first-single-prior", action="store_true", help="Do not use early single shots as weak person_B anchors")
    parser.add_argument("--old-label-spreading", action="store_true", help="Use the older LabelSpreading graph model instead of the constraint-first prototype model")
    parser.add_argument("--early-single-p2-stop-frame", default=75, type=int, help="Last frame/sec used for early single person_B prior")
    parser.add_argument("--early-single-p2-max-anchors", default=80, type=int, help="Max early single close-up anchors assigned to person_B")
    parser.add_argument("--prototype-temperature", default=2.25, type=float, help="Softmax temperature for prototype distances")
    parser.add_argument("--low-confidence-threshold", default=0.55, type=float, help="Rows below this confidence are written to the review CSV")
    parser.add_argument("--annotate-every", default=30, type=int, help="Save one annotated debug frame every N frame groups; <=0 disables")
    parser.add_argument("--no-debug-images", action="store_true", help="Do not write annotated frames/crops")
    args = parser.parse_args()

    cfg = Config(
        pkl=args.pkl,
        project_root=args.project_root,
        frames_root=args.frames_root,
        out=args.out,
        fps=args.fps,
        use_insightface=args.use_insightface,
        insightface_model=args.insightface_model,
        insightface_ctx_id=args.insightface_ctx_id,
        two_large_sum_area=args.two_large_sum_area,
        two_touching_gap=args.two_touching_gap,
        force_small_two_has_person2=not args.no_small_two_person2,
        use_first_single_p2_prior=not args.no_first_single_prior,
        constraint_first=not args.old_label_spreading,
        early_single_p2_stop_frame=args.early_single_p2_stop_frame,
        early_single_p2_max_anchors=args.early_single_p2_max_anchors,
        prototype_temperature=args.prototype_temperature,
        low_confidence_threshold=args.low_confidence_threshold,
        annotate_every=args.annotate_every,
        save_debug_images=not args.no_debug_images,
    )
    cfg.project_root = cfg.project_root.resolve()
    if not cfg.pkl.is_absolute():
        cfg.pkl = (cfg.project_root / cfg.pkl).resolve()
    if not cfg.out.is_absolute():
        cfg.out = (cfg.project_root / cfg.out).resolve()

    print(f"Loading visual pickle: {cfg.pkl}")
    df = load_visual_pickle(cfg.pkl)
    width, height = infer_frame_size(df, cfg.project_root)
    print(f"Loaded {len(df)} frame rows. Inferred frame size: {width}x{height}")
    print("Frame composition counts:")
    print(pd.crosstab(df["n_poses"], df["n_faces"]))

    det = build_detection_table(df, cfg, width, height)
    print(f"Usable face detections: {len(det)}")

    base_X, base_names, aux = create_base_features(det, width, height)
    det = pd.concat([det.reset_index(drop=True), aux.reset_index(drop=True)], axis=1)

    arc_X = extract_insightface_embeddings(det, cfg)
    X = build_model_features(det, base_X, arc_X, cfg)

    det = assign_weak_labels(det, cfg)
    anchor_counts = det.loc[det["weak_label"] >= 0, "weak_label"].value_counts().sort_index()
    print("Weak anchors:", {INT_TO_LABEL[int(k)]: int(v) for k, v in anchor_counts.items()})

    if cfg.constraint_first:
        probs, pred, model_name = fit_prototype_identity_model(X, det, cfg)
    else:
        probs, pred, model_name = fit_identity_model(X, det, cfg)
    print(f"Identity model: {model_name}")
    out = apply_frame_constraints(det, probs, cfg)
    out = smooth_single_person_runs(out)
    print("Final assignments:")
    print(out["person_label"].value_counts().reindex(LABELS).fillna(0).astype(int))

    try:
        if len(set(out["person_label"])) == 3 and len(out) > 10:
            sil = silhouette_score(X, out["person_label"].map(LABEL_TO_INT).to_numpy())
            print(f"Silhouette score using final labels: {sil:.3f}")
    except Exception:
        pass

    make_outputs(out, df, X, cfg, width, height, model_name)
    print(f"Done. Outputs written to: {cfg.out}")


if __name__ == "__main__":
    main()
