# Facebook Reel Script: The Anti-Cheat Smart Attendance System
**Format:** Vertical (9:16)
**Target Length:** 45 - 60 Seconds
**Vibe:** Fast-paced, tech-focused, engaging.

---

### [0:00 - 0:05] THE HOOK
* **Visual:** Fast, zoomed-in shot of someone holding a printed piece of paper with a QR code (or a screenshot on another phone). They scan it with the web app, and the screen instantly flashes a big red **"❌ QR Code Expired or Invalid!"**.
* **Text on Screen:** Students keep faking attendance...
* **Voiceover:** Students were using old screenshots to fake their class attendance... so I built an IoT system to completely stop them.

### [0:05 - 0:15] THE HARDWARE
* **Visual:** Cut to a slick, close-up shot of your ESP32 and TFT display on the breadboard. Speed up the video slightly so the viewer can clearly see the QR code and the token numbers at the bottom changing rapidly.
* **Text on Screen:** ESP32 Autonomous Terminal
* **Voiceover:** This tiny ESP32 terminal generates a brand new, encrypted QR code completely offline every 5 seconds. 

### [0:15 - 0:25] THE SOFTWARE (STUDENT APP)
* **Visual:** Screen-recording of a phone. The user clicks "Sign in with Google", selects a course from a dropdown, and scans the *live* ESP32 screen. The app pauses on "Verifying..." then pops green **"✅ Attendance Confirmed"**.
* **Text on Screen:** Firebase + Google Auth
* **Voiceover:** To check in, students log into a custom Firebase web app using Google. When they scan the live code, the app blindly fires it to the cloud, bypassing strict mobile CORS limits. 

### [0:25 - 0:35] THE BRAIN (THINGSBOARD)
* **Visual:** Pan across your ThingsBoard Rule Engine flowchart on your laptop monitor. Point your finger at the yellow "Verify Token Match" script node.
* **Text on Screen:** ThingsBoard IoT Rule Engine
* **Voiceover:** The secret sauce is the ThingsBoard Rule Engine. It strictly compares the scanned code to the terminal's live active token in real-time. If it's a screenshot? Instant rejection.

### [0:35 - 0:45] THE RESULT & OUTRO
* **Visual:** Show the ThingsBoard Admin Dashboard with the Timeseries table actively populating with the student's name, email, and course. Give a thumbs up to the camera!
* **Text on Screen:** Secure. Autonomous. Real-time.
* **Voiceover:** Attendance is logged perfectly, autonomously, and securely. Hit follow if you want the full breakdown of how I coded it!

---
**Suggested Music/Audio:** A fast-paced, modern electronic beat or a trending tech-tutorial audio track on Facebook/Instagram.
**Suggested Hashtags:** #IoT #ESP32 #ThingsBoard #Firebase #Coding #TechProjects #Engineering #SmartSystem
