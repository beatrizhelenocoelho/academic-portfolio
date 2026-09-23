from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

# ------------------------------------------------------------
# SETTINGS
# ------------------------------------------------------------

DATA_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = DATA_DIR / "main_faces_on_screen_over_time"
OUTPUT_DIR.mkdir(exist_ok=True)

OVERLAP_THRESHOLD = 0.55
MIN_RELATIVE_FACE_SIZE = 0.25
# A face is kept only if its area is at least 25% of the largest face in that frame.


# ------------------------------------------------------------
# BOX FUNCTIONS
# ------------------------------------------------------------

def box_area(box):
    if box is None or len(box) < 4:
        return 0.0

    x1, y1, x2, y2 = box
    width = max(0.0, float(x2 - x1))
    height = max(0.0, float(y2 - y1))
    return width * height


def box_iou(box_a, box_b):
    ax1, ay1, ax2, ay2 = box_a
    bx1, by1, bx2, by2 = box_b

    inter_x1 = max(ax1, bx1)
    inter_y1 = max(ay1, by1)
    inter_x2 = min(ax2, bx2)
    inter_y2 = min(ay2, by2)

    inter_w = max(0.0, inter_x2 - inter_x1)
    inter_h = max(0.0, inter_y2 - inter_y1)

    intersection = inter_w * inter_h
    union = box_area(box_a) + box_area(box_b) - intersection

    if union <= 0:
        return 0.0

    return intersection / union


def remove_overlapping_faces(faces):
    if not isinstance(faces, list) or len(faces) == 0:
        return []

    valid_faces = [
        face for face in faces
        if face.get("bbox", None) is not None and len(face.get("bbox", [])) >= 4
    ]

    valid_faces = sorted(
        valid_faces,
        key=lambda face: box_area(face["bbox"]),
        reverse=True
    )

    kept = []

    for face in valid_faces:
        current_box = face["bbox"]

        duplicate = False

        for kept_face in kept:
            if box_iou(current_box, kept_face["bbox"]) >= OVERLAP_THRESHOLD:
                duplicate = True
                break

        if not duplicate:
            kept.append(face)

    return kept


def keep_main_faces(faces):
    """
    Keeps only the visually important faces in a frame.

    Step 1: remove overlapping duplicate detections.
    Step 2: remove tiny/background faces.
    """
    faces = remove_overlapping_faces(faces)

    if len(faces) == 0:
        return []

    areas = [box_area(face["bbox"]) for face in faces]
    largest_area = max(areas)

    if largest_area <= 0:
        return []

    main_faces = [
        face for face in faces
        if box_area(face["bbox"]) >= MIN_RELATIVE_FACE_SIZE * largest_area
    ]

    return main_faces


# ------------------------------------------------------------
# PROCESS FILES
# ------------------------------------------------------------

visual_files = sorted(DATA_DIR.glob("*_visual.pkl"))

print("Visual files found:", len(visual_files))

all_rows = []

for file_path in visual_files:
    video_name = file_path.stem.replace("_visual", "")
    print("Processing:", video_name)

    df = pd.read_pickle(file_path)

    rows = []

    for frame_index, row in df.iterrows():
        faces = row["Fer"]

        raw_faces = len(faces) if isinstance(faces, list) else 0
        main_faces = keep_main_faces(faces)

        rows.append({
            "video": video_name,
            "frame_index": frame_index,
            "time_seconds": frame_index,
            "raw_faces": raw_faces,
            "main_faces": len(main_faces),
            "removed_faces": raw_faces - len(main_faces),
        })

    video_df = pd.DataFrame(rows)
    all_rows.append(video_df)

    # --------------------------------------------------------
    # PLOT MAIN FACES ONLY
    # --------------------------------------------------------

    plt.figure(figsize=(14, 4))
    plt.plot(
        video_df["time_seconds"] / 60,
        video_df["main_faces"],
        linewidth=1
    )

    plt.title(f"Main Faces on Screen Over Time\n{video_name}")
    plt.xlabel("Time (minutes)")
    plt.ylabel("Number of main faces")
    plt.ylim(bottom=0)
    plt.tight_layout()

    out_path = OUTPUT_DIR / f"{video_name}_main_faces_on_screen_over_time.png"
    plt.savefig(out_path, dpi=200, bbox_inches="tight")
    plt.close()

    print("Saved:", out_path)


# ------------------------------------------------------------
# SAVE RESULTS
# ------------------------------------------------------------

all_counts = pd.concat(all_rows, ignore_index=True)

csv_path = OUTPUT_DIR / "main_face_counts.csv"
all_counts.to_csv(csv_path, index=False)

summary = all_counts.groupby("video").agg(
    total_frames=("frame_index", "count"),
    raw_faces=("raw_faces", "sum"),
    main_faces=("main_faces", "sum"),
    removed_faces=("removed_faces", "sum"),
)

summary["percent_removed"] = summary["removed_faces"] / summary["raw_faces"] * 100

summary_path = OUTPUT_DIR / "main_face_filter_summary.csv"
summary.to_csv(summary_path)

print("\nFilter summary:")
print(summary)

print("\nSaved:")
print(csv_path)
print(summary_path)

print("\nDone.")
print("All outputs saved in:", OUTPUT_DIR)