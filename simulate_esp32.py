import qrcode
import tkinter as tk
from PIL import Image, ImageTk
import random

def generate_qr():
    # Generate a random 8-digit token just like the ESP32
    token = str(random.randint(10000000, 99999999))
    print(f"Generating new token: {token}")
    
    # Create the QR Code
    qr = qrcode.QRCode(
        version=3,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=15,
        border=4,
    )
    qr.add_data(token)
    qr.make(fit=True)
    
    # Create an image from the QR Code instance
    img = qr.make_image(fill_color="black", back_color="white")
    
    # Convert to PhotoImage for Tkinter
    tk_img = ImageTk.PhotoImage(img)
    
    # Update the label
    label.config(image=tk_img)
    label.image = tk_img # Keep a reference!
    
    # Update the text label
    text_label.config(text=f"Token: {token}")
    
    # Schedule the next generation in 5 seconds (5000 ms)
    root.after(5000, generate_qr)

# Setup Tkinter Window
root = tk.Tk()
root.title("ESP32 QR Code Simulator")
root.geometry("500x550")
root.configure(bg="white")

# Setup Labels
label = tk.Label(root, bg="white")
label.pack(pady=20)

text_label = tk.Label(root, text="Starting...", font=("Arial", 18, "bold"), bg="white")
text_label.pack()

# Start the loop
generate_qr()
root.mainloop()
