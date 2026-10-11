import cv2
import os

folder = "demo videos"
files = sorted([f for f in os.listdir(folder) if f.endswith('.mp4')])

for f in files:
    path = os.path.join(folder, f)
    cap = cv2.VideoCapture(path)
    fps = cap.get(cv2.CAP_PROP_FPS)
    frame_count = cap.get(cv2.CAP_PROP_FRAME_COUNT)
    w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    duration = frame_count / fps if fps > 0 else 0
    cap.release()
    print(f"{f}: {w}x{h}, {fps:.1f}fps, {duration:.1f}s")
