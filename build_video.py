import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont
import requests
import io
import base64
import json
import time

def get_mermaid_image(code):
    data = {"code": code, "mermaid": '{"theme": "default"}'}
    json_str = json.dumps(data)
    encoded = base64.urlsafe_b64encode(json_str.encode('utf-8')).decode('utf-8')
    url = f"https://mermaid.ink/img/{encoded}"
    try:
        resp = requests.get(url, timeout=10)
        if resp.status_code == 200:
            return Image.open(io.BytesIO(resp.content)).convert("RGBA")
    except Exception as e:
        print(f"Failed to fetch image: {e}")
    return None

diag1 = """flowchart LR
    subgraph Hardware
        direction TB
        TokenGen["Token Generator"] --> ESP32["ESP32 Microcontroller"]
        ESP32 --> Display["TFT Display"]
    end
    subgraph Smartphone
        direction TB
        Camera["Camera Scanner"] --> WebApp["Web Application"]
    end
    ESP32 -- "1. MQTT (Push active_token)" --> TB_Cloud["ThingsBoard"]
    Display -. "2. Dynamic QR Code" .-> Camera
    WebApp -- "3. POST Telemetry" --> TB_Cloud
"""
diag2 = """flowchart TD
    Start(["Power ON"]) --> WiFi["Connect to WiFi & MQTT"]
    WiFi --> GenToken["Generate Random 8-digit Token"]
    GenToken --> DrawQR["Render Token as QR Code on TFT"]
    DrawQR --> SyncTB["Publish Token to ThingsBoard<br>Client Attributes"]
    SyncTB --> Wait["Delay 5 Seconds"]
    Wait --> GenToken
"""
diag3 = """flowchart TD
    Scan["Student Scans QR Code"] --> POST["Send telemetry to ThingsBoard"]
    POST --> TB_Logic["Rule Engine: Match Token?"]
    TB_Logic -- "YES" --> Save["Save Timeseries"]
    Save --> API["Push success to Firebase"]
    API -.-> Poll["Web App Polls Firebase"]
    Poll --> Confirm{"Confirmation found?"}
    Confirm -- "YES" --> Success["Display: Success"]
"""

# Video settings
width, height = 1080, 1920
fps = 30
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter('Project_Showcase.mp4', fourcc, fps, (width, height))

# Load font
try:
    font_large = ImageFont.truetype("arialbd.ttf", 60)
    font_medium = ImageFont.truetype("arial.ttf", 40)
except:
    font_large = ImageFont.load_default()
    font_medium = ImageFont.load_default()

def draw_text_centered(draw, text, font, fill, y):
    bbox = draw.textbbox((0, 0), text, font=font)
    w = bbox[2] - bbox[0]
    draw.text(((width - w) / 2, y), text, font=font, fill=fill)

def create_frame(bg_color, text_main, text_sub, img=None):
    frame = Image.new('RGB', (width, height), bg_color)
    draw = ImageDraw.Draw(frame)
    
    main_fill = (255, 255, 255) if bg_color == (20, 20, 20) else (30, 30, 30)
    sub_fill = (200, 200, 200) if bg_color == (20, 20, 20) else (100, 100, 100)
    
    draw_text_centered(draw, text_main, font_large, main_fill, height//8)
    if text_sub:
        draw_text_centered(draw, text_sub, font_medium, sub_fill, height//8 + 80)
        
    if img:
        target_w = width - 100
        ratio = target_w / float(img.width)
        target_h = int(img.height * ratio)
        img_resized = img.resize((target_w, target_h), Image.LANCZOS)
        
        paste_x = 50
        paste_y = (height - target_h) // 2
        bg = Image.new("RGB", (target_w, target_h), (255,255,255))
        bg.paste(img_resized, (0,0), mask=img_resized)
        frame.paste(bg, (paste_x, paste_y))
        
    return cv2.cvtColor(np.array(frame), cv2.COLOR_RGB2BGR)

print("Fetching diagrams...")
img1 = get_mermaid_image(diag1)
img2 = get_mermaid_image(diag2)
img3 = get_mermaid_image(diag3)

print("Rendering video frames...")
scenes = [
    (4, (20, 20, 20), "Smart Attendance System", "Anti-Cheat IoT Network", None),
    (6, (245, 245, 245), "System Architecture", "ESP32 -> ThingsBoard -> Firebase", img1),
    (6, (245, 245, 245), "Autonomous Hardware", "Offline QR Code Generation", img2),
    (6, (245, 245, 245), "Cloud Logic", "ThingsBoard Rule Engine Validation", img3),
    (4, (20, 20, 20), "Secure. Real-time.", "Ready for deployment.", None)
]

for duration, bg, t1, t2, img in scenes:
    print(f"Rendering scene: {t1}")
    frame_data = create_frame(bg, t1, t2, img)
    for _ in range(int(duration * fps)):
        out.write(frame_data)

out.release()
print("Video saved as Project_Showcase.mp4")
