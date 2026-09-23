import numpy as np
import pandas as pd


# ---------------------------------------------------------------------
# Default annotation coordinate system
# ---------------------------------------------------------------------

DEFAULT_FRAME_WIDTH = 1280
DEFAULT_FRAME_HEIGHT = 720


# ---------------------------------------------------------------------
# Bounding-box helpers
# ---------------------------------------------------------------------

def bbox_to_float(bbox):
    return [float(v) for v in bbox]


def bbox_area(bbox):
    x1, y1, x2, y2 = bbox_to_float(bbox)
    return max(0, x2 - x1) * max(0, y2 - y1)


def bbox_area_normalized(
    bbox,
    frame_width=DEFAULT_FRAME_WIDTH,
    frame_height=DEFAULT_FRAME_HEIGHT,
):
    return bbox_area(bbox) / (frame_width * frame_height)


def bbox_center_normalized(
    bbox,
    frame_width=DEFAULT_FRAME_WIDTH,
    frame_height=DEFAULT_FRAME_HEIGHT,
):
    x1, y1, x2, y2 = bbox_to_float(bbox)

    cx = ((x1 + x2) / 2) / frame_width
    cy = ((y1 + y2) / 2) / frame_height

    return cx, cy


def bbox_intersection_area(box_a, box_b):
    ax1, ay1, ax2, ay2 = bbox_to_float(box_a)
    bx1, by1, bx2, by2 = bbox_to_float(box_b)

    inter_x1 = max(ax1, bx1)
    inter_y1 = max(ay1, by1)
    inter_x2 = min(ax2, bx2)
    inter_y2 = min(ay2, by2)

    inter_w = max(0, inter_x2 - inter_x1)
    inter_h = max(0, inter_y2 - inter_y1)

    return inter_w * inter_h


def bbox_iou(box_a, box_b):
    inter_area = bbox_intersection_area(box_a, box_b)

    area_a = bbox_area(box_a)
    area_b = bbox_area(box_b)

    union = area_a + area_b - inter_area

    if union == 0:
        return 0.0

    return inter_area / union


def bbox_overlap_over_smaller_box(box_a, box_b):
    """
    Measures how much of the smaller box is covered by the larger one.

    This catches cases where:
    - a small duplicate box is almost completely inside a larger box
    - IoU is not high enough because the large box is much bigger
    """
    inter_area = bbox_intersection_area(box_a, box_b)

    area_a = bbox_area(box_a)
    area_b = bbox_area(box_b)

    smaller_area = min(area_a, area_b)

    if smaller_area == 0:
        return 0.0

    return inter_area / smaller_area


def detection_confidence(detection):
    """
    Confidence for sorting duplicate boxes.
    """
    if "class_conf" in detection:
        return float(detection.get("class_conf", 0.0))

    if "top_emotion" in detection and "probabilities" in detection:
        emotion = detection.get("top_emotion")
        probs = detection.get("probabilities", {})
        return float(probs.get(emotion, 0.0))

    return 0.0


def detection_sort_score(detection, prefer_larger=False):
    """
    For people, larger boxes are usually better when removing duplicates.
    For faces, confidence is usually enough.
    """
    conf = detection_confidence(detection)

    if prefer_larger:
        return bbox_area(detection["bbox"])

    return conf


def remove_duplicate_detections(
    detections,
    iou_threshold=0.90,
    containment_threshold=0.90,
    prefer_larger=False,
):
    """
    Conservative duplicate removal.

    Removes a detection if:
    1. IoU with another kept box is very high; OR
    2. 90%+ of the smaller box is inside the larger one.

    This fixes cases where the same person gets one large box and one smaller
    duplicate box.
    """

    detections = detections if isinstance(detections, list) else []

    if len(detections) <= 1:
        return detections

    detections_sorted = sorted(
        detections,
        key=lambda det: detection_sort_score(det, prefer_larger=prefer_larger),
        reverse=True,
    )

    kept = []

    for detection in detections_sorted:
        duplicate = False

        for kept_detection in kept:
            iou = bbox_iou(detection["bbox"], kept_detection["bbox"])
            containment = bbox_overlap_over_smaller_box(
                detection["bbox"],
                kept_detection["bbox"],
            )

            if iou >= iou_threshold or containment >= containment_threshold:
                duplicate = True
                break

        if not duplicate:
            kept.append(detection)

    return kept


# ---------------------------------------------------------------------
# Interpreter filters
# ---------------------------------------------------------------------

def is_interpreter_person_bbox(
    bbox,
    frame_width=DEFAULT_FRAME_WIDTH,
    frame_height=DEFAULT_FRAME_HEIGHT,
):
    """
    Safer interpreter filter for person/pose boxes.

    Only removes boxes very close to the screen edges.
    """

    cx, cy = bbox_center_normalized(
        bbox,
        frame_width=frame_width,
        frame_height=frame_height,
    )

    area = bbox_area_normalized(
        bbox,
        frame_width=frame_width,
        frame_height=frame_height,
    )

    left_interpreter = (
        cx < 0.22
        and cy > 0.38
        and area < 0.16
    )

    right_interpreter = (
        cx > 0.84
        and cy > 0.30
        and area < 0.16
    )

    return left_interpreter or right_interpreter


def is_interpreter_face_bbox(
    bbox,
    frame_width=DEFAULT_FRAME_WIDTH,
    frame_height=DEFAULT_FRAME_HEIGHT,
):
    """
    Safer interpreter filter for face boxes.
    """

    cx, cy = bbox_center_normalized(
        bbox,
        frame_width=frame_width,
        frame_height=frame_height,
    )

    area = bbox_area_normalized(
        bbox,
        frame_width=frame_width,
        frame_height=frame_height,
    )

    left_interpreter_face = (
        cx < 0.22
        and cy > 0.35
        and area < 0.035
    )

    right_interpreter_face = (
        cx > 0.84
        and cy > 0.25
        and area < 0.035
    )

    return left_interpreter_face or right_interpreter_face


def remove_interpreters_from_detections(
    poses,
    faces,
    frame_width=DEFAULT_FRAME_WIDTH,
    frame_height=DEFAULT_FRAME_HEIGHT,
):
    poses = poses if isinstance(poses, list) else []
    faces = faces if isinstance(faces, list) else []

    clean_poses = [
        person for person in poses
        if not is_interpreter_person_bbox(
            person["bbox"],
            frame_width=frame_width,
            frame_height=frame_height,
        )
    ]

    clean_faces = [
        face for face in faces
        if not is_interpreter_face_bbox(
            face["bbox"],
            frame_width=frame_width,
            frame_height=frame_height,
        )
    ]

    return clean_poses, clean_faces


# ---------------------------------------------------------------------
# Confidence helpers
# ---------------------------------------------------------------------

def get_top_emotion(face):
    return face.get("top_emotion", None)


def get_top_emotion_confidence(face):
    top_emotion = face.get("top_emotion", None)
    probabilities = face.get("probabilities", {})

    if top_emotion is None:
        return np.nan

    return float(probabilities.get(top_emotion, np.nan))


def is_high_confidence_pose(person, min_pose_confidence=0.30):
    confidence = float(person.get("class_conf", 0.0))
    return confidence >= min_pose_confidence


def is_high_confidence_face(face, min_face_emotion_confidence=0.00):
    confidence = get_top_emotion_confidence(face)

    if np.isnan(confidence):
        return False

    return confidence >= min_face_emotion_confidence


def remove_low_confidence_detections(
    poses,
    faces,
    min_pose_confidence=0.30,
    min_face_emotion_confidence=0.00,
):
    poses = poses if isinstance(poses, list) else []
    faces = faces if isinstance(faces, list) else []

    clean_poses = [
        person for person in poses
        if is_high_confidence_pose(
            person,
            min_pose_confidence=min_pose_confidence,
        )
    ]

    clean_faces = [
        face for face in faces
        if is_high_confidence_face(
            face,
            min_face_emotion_confidence=min_face_emotion_confidence,
        )
    ]

    return clean_poses, clean_faces


# ---------------------------------------------------------------------
# Apply cleaning to a whole dataframe
# ---------------------------------------------------------------------

def clean_visual_dataframe(
    df,
    frame_width=DEFAULT_FRAME_WIDTH,
    frame_height=DEFAULT_FRAME_HEIGHT,
    poses_col="Poses",
    faces_col="Fer",
    min_pose_confidence=0.30,
    min_face_emotion_confidence=0.00,
    person_iou_threshold=0.90,
    person_containment_threshold=0.90,
    face_iou_threshold=0.90,
    face_containment_threshold=0.95,
):
    """
    Safer visual cleaning.

    Steps:
    1. Remove clear interpreter detections.
    2. Remove very low-confidence person detections.
    3. Do not delete faces based on emotion confidence by default.
    4. Remove duplicate boxes using high IoU or high containment.
    """

    df = df.copy()

    df["num_people_raw"] = df[poses_col].apply(
        lambda x: len(x) if isinstance(x, list) else 0
    )

    df["num_faces_raw"] = df[faces_col].apply(
        lambda x: len(x) if isinstance(x, list) else 0
    )

    # Step 1: remove interpreters
    interpreter_results = df.apply(
        lambda row: remove_interpreters_from_detections(
            row[poses_col],
            row[faces_col],
            frame_width=frame_width,
            frame_height=frame_height,
        ),
        axis=1,
    )

    df["Poses_No_Interpreter"] = interpreter_results.apply(lambda x: x[0])
    df["Fer_No_Interpreter"] = interpreter_results.apply(lambda x: x[1])

    df["num_people_after_interpreter_filter"] = df["Poses_No_Interpreter"].apply(len)
    df["num_faces_after_interpreter_filter"] = df["Fer_No_Interpreter"].apply(len)

    # Step 2: confidence filtering
    confidence_results = df.apply(
        lambda row: remove_low_confidence_detections(
            row["Poses_No_Interpreter"],
            row["Fer_No_Interpreter"],
            min_pose_confidence=min_pose_confidence,
            min_face_emotion_confidence=min_face_emotion_confidence,
        ),
        axis=1,
    )

    df["Clean_Poses_Before_Dedup"] = confidence_results.apply(lambda x: x[0])
    df["Clean_Fer_Before_Dedup"] = confidence_results.apply(lambda x: x[1])

    df["num_people_before_dedup"] = df["Clean_Poses_Before_Dedup"].apply(len)
    df["num_faces_before_dedup"] = df["Clean_Fer_Before_Dedup"].apply(len)

    # Step 3: duplicate removal
    df["Clean_Poses"] = df["Clean_Poses_Before_Dedup"].apply(
        lambda detections: remove_duplicate_detections(
            detections,
            iou_threshold=person_iou_threshold,
            containment_threshold=person_containment_threshold,
            prefer_larger=True,
        )
    )

    df["Clean_Fer"] = df["Clean_Fer_Before_Dedup"].apply(
        lambda detections: remove_duplicate_detections(
            detections,
            iou_threshold=face_iou_threshold,
            containment_threshold=face_containment_threshold,
            prefer_larger=False,
        )
    )

    df["num_people_clean"] = df["Clean_Poses"].apply(len)
    df["num_faces_clean"] = df["Clean_Fer"].apply(len)

    # Removed by interpreter filter
    df["removed_people_interpreter"] = (
        df["num_people_raw"] - df["num_people_after_interpreter_filter"]
    )

    df["removed_faces_interpreter"] = (
        df["num_faces_raw"] - df["num_faces_after_interpreter_filter"]
    )

    # Removed by confidence filter
    df["removed_people_low_confidence"] = (
        df["num_people_after_interpreter_filter"] - df["num_people_before_dedup"]
    )

    df["removed_faces_low_confidence"] = (
        df["num_faces_after_interpreter_filter"] - df["num_faces_before_dedup"]
    )

    # Removed by duplicate filter
    df["removed_people_duplicates"] = (
        df["num_people_before_dedup"] - df["num_people_clean"]
    )

    df["removed_faces_duplicates"] = (
        df["num_faces_before_dedup"] - df["num_faces_clean"]
    )

    # Total removed
    df["removed_people_total"] = df["num_people_raw"] - df["num_people_clean"]
    df["removed_faces_total"] = df["num_faces_raw"] - df["num_faces_clean"]

    df["removed_people"] = df["removed_people_total"]
    df["removed_faces"] = df["removed_faces_total"]

    return df


# ---------------------------------------------------------------------
# Clean emotion helpers
# ---------------------------------------------------------------------

def largest_clean_face(clean_faces):
    clean_faces = clean_faces if isinstance(clean_faces, list) else []

    if len(clean_faces) == 0:
        return None

    return max(
        clean_faces,
        key=lambda face: bbox_area_normalized(face["bbox"])
    )


def largest_clean_face_emotion(clean_faces):
    face = largest_clean_face(clean_faces)

    if face is None:
        return None

    return get_top_emotion(face)


def largest_clean_face_emotion_confidence(clean_faces):
    face = largest_clean_face(clean_faces)

    if face is None:
        return np.nan

    return get_top_emotion_confidence(face)


def largest_clean_face_area(clean_faces):
    face = largest_clean_face(clean_faces)

    if face is None:
        return np.nan

    return bbox_area_normalized(face["bbox"])


def add_clean_emotion_columns(df):
    df = df.copy()

    df["largest_clean_face_emotion"] = df["Clean_Fer"].apply(
        largest_clean_face_emotion
    )

    df["largest_clean_face_emotion_confidence"] = df["Clean_Fer"].apply(
        largest_clean_face_emotion_confidence
    )

    df["largest_clean_face_area"] = df["Clean_Fer"].apply(
        largest_clean_face_area
    )

    return df