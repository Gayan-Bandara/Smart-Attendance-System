from moviepy import *
from PIL import Image, ImageDraw, ImageFont
import numpy as np
import os

# ─── Settings ────────────────────────────────────────────────────────────────
OUTPUT_W, OUTPUT_H = 1920, 1080
FPS = 30
OUTPUT_FILE = "Smart_Attendance_Demo.mp4"
FOLDER = "demo videos"

# ─── Helper: PIL image -> MoviePy ImageClip ──────────────────────────────────
def pil_to_clip(pil_img, duration):
    arr = np.array(pil_img.convert("RGB"))
    return ImageClip(arr, duration=duration)

# ─── Helper: Load a font, fall back gracefully ──────────────────────────────
def load_font(size):
    for name in ["arialbd.ttf", "Arial_Bold.ttf", "DejaVuSans-Bold.ttf", "DejaVuSans.ttf"]:
        try:
            return ImageFont.truetype(name, size)
        except:
            pass
    return ImageFont.load_default()

# ─── Make a full-frame title card ────────────────────────────────────────────
def make_title_card(line1, line2="", duration=3, bg=(15, 15, 30)):
    img = Image.new("RGB", (OUTPUT_W, OUTPUT_H), bg)
    draw = ImageDraw.Draw(img)
    f1 = load_font(90)
    f2 = load_font(52)

    # centre line1
    bbox = draw.textbbox((0, 0), line1, font=f1)
    w1 = bbox[2] - bbox[0]
    y1 = OUTPUT_H // 2 - 80
    draw.text(((OUTPUT_W - w1) / 2, y1), line1, font=f1, fill=(255, 255, 255))

    # centre line2
    if line2:
        bbox2 = draw.textbbox((0, 0), line2, font=f2)
        w2 = bbox2[2] - bbox2[0]
        y2 = y1 + 110
        draw.text(((OUTPUT_W - w2) / 2, y2), line2, font=f2, fill=(150, 200, 255))

    # accent bar
    draw.rectangle([(OUTPUT_W // 2 - 250, y1 - 20), (OUTPUT_W // 2 + 250, y1 - 12)],
                   fill=(100, 160, 255))
    return pil_to_clip(img, duration)

# ─── Make a lower-third label overlay ────────────────────────────────────────
def make_lower_third(title, subtitle, video_clip):
    duration = video_clip.duration
    img = Image.new("RGBA", (OUTPUT_W, OUTPUT_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    f1 = load_font(52)
    f2 = load_font(36)

    box_x1, box_y1 = 60, OUTPUT_H - 160
    box_x2, box_y2 = 900, OUTPUT_H - 30

    draw.rectangle([(box_x1, box_y1), (box_x2, box_y2)], fill=(15, 15, 30, 210))
    draw.rectangle([(box_x1, box_y1), (box_x1 + 8, box_y2)], fill=(100, 160, 255, 255))
    draw.text((box_x1 + 25, box_y1 + 12), title, font=f1, fill=(255, 255, 255, 255))
    draw.text((box_x1 + 25, box_y1 + 72), subtitle, font=f2, fill=(150, 200, 255, 255))

    overlay_arr = np.array(img)
    overlay_clip = ImageClip(overlay_arr, duration=duration)
    return CompositeVideoClip([video_clip, overlay_clip])

# ─── Normalize a raw clip to 1920×1080 ───────────────────────────────────────
def normalize(path):
    clip = VideoFileClip(path)

    # If portrait (taller than wide), place on black 16:9 canvas
    if clip.h > clip.w:
        scale = min(OUTPUT_W / clip.w, OUTPUT_H / clip.h)
        resized = clip.resized(scale)
        bg = ColorClip(size=(OUTPUT_W, OUTPUT_H), color=(0, 0, 0), duration=clip.duration)
        x_off = (OUTPUT_W - resized.w) // 2
        y_off = (OUTPUT_H - resized.h) // 2
        final = CompositeVideoClip([bg, resized.with_position((x_off, y_off))])
    else:
        final = clip.resized((OUTPUT_W, OUTPUT_H))

    return final.with_fps(FPS)

# ─── Build the sections ───────────────────────────────────────────────────────
videos = sorted([
    os.path.join(FOLDER, f)
    for f in os.listdir(FOLDER) if f.endswith(".mp4")
])

v1, v2, v3, v4 = [normalize(v) for v in videos]

labels = [
    ("Demo 1: System Overview",      "Hardware terminal generating rotating QR codes"),
    ("Demo 2: Admin Dashboard",       "ThingsBoard course management & Rule Engine"),
    ("Demo 3: Student Check-in",      "Google login → select course → scan QR code"),
    ("Demo 4: Attendance Confirmed",  "Firebase verification & real-time confirmation"),
]

sections = []
for clip, (title, subtitle) in zip([v1, v2, v3, v4], labels):
    tagged = make_lower_third(title, subtitle, clip)
    sections.append(tagged)

# ─── Title card ───────────────────────────────────────────────────────────────
intro = make_title_card("Smart Attendance System",
                        "Anti-Cheat IoT Network  |  ESP32 + ThingsBoard + Firebase",
                        duration=4)

outro = make_title_card("Thanks for watching!",
                        "ESP32  •  ThingsBoard  •  Firebase  •  Google Auth",
                        duration=3, bg=(10, 30, 10))

# ─── Concatenate everything ───────────────────────────────────────────────────
all_clips = [intro] + sections + [outro]
final = concatenate_videoclips(all_clips, method="compose")

print(f"Total duration: {final.duration:.1f}s")
print("Rendering video (this may take a few minutes)...")
final.write_videofile(OUTPUT_FILE, fps=FPS, codec="libx264",
                      audio_codec="aac", threads=4,
                      logger="bar")
print(f"\nDone! Saved as {OUTPUT_FILE}")
