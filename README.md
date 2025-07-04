# 🌈 Emotional Lamp – Mood-Sensing Interactive Light System

**Emotional Lamp** is a smart, interactive ambient lighting project powered by an ESP32 microcontroller. It detects changes in temperature, light, and motion to infer human-like moods and expresses them through dynamic NeoPixel animations, OLED emoji displays, and soothing melodies. It’s both a fun and therapeutic project that turns environmental data into a delightful emotional experience.

---

## 🧠 Overview
The Emotional Lamp senses the world around it and reflects an emotional state using:

✨ Vibrant LED animations (rainbow, strobe, pulse, fade)  
📺 OLED emojis and motivational quotes  
🔊 Mood-based melodies using a buzzer  
🔘 A manual override button to cycle moods  
💾 Persistent memory using EEPROM to remember your last mood  

Perfect for personal spaces, therapy rooms, and mindfulness corners!

---

## 🎯 Key Features
| Feature | Description |
|--------|-------------|
| 🎭 Smart Mood Detection | Combines temperature, light, and motion to determine mood |
| 🌈 Mood-Based Lighting | NeoPixel ring displays mood-specific effects (rainbow, fade, flash) |
| 🔊 Melodic Feedback | Buzzer plays a unique melody for each emotional state |
| 🖥 OLED Mood Display | Shows mood name, emoji, real-time sensor data, and calming quotes |
| 🔘 Manual Mood Control | Cycle moods with a pushbutton for testing or preference |
| 💾 EEPROM Storage | Saves the current mood across reboots |
| 👋 Startup Animation | Displays a welcoming animation on boot |

---

## 🎨 Moods & Expressions
| Mood       | LED Animation   | Emoji | Melody       | Quote                          |
|------------|------------------|--------|--------------|---------------------------------|
| Happy      | Rainbow swirl    | 🙂     | High notes   | Keep smiling!                  |
| Sad        | Gentle pulse     | ☹️     | Low notes    | It's okay to feel low.         |
| Angry      | Flashing red     | ><     | Sharp buzz   | Take a deep breath.            |
| Curious    | LED sweep        | ❓     | Rising tones | Explore the unknown.           |
| Excited    | Fast strobe      | !!     | Fast notes   | Feel the energy!               |
| Sleepy     | Soft fade        | Zz     | Calm notes   | Time to rest.                  |
| Calm       | Steady glow      | ~      | Soft tones   | Stay peaceful.                 |
| Anxious    | Quick flash      | ??     | Alert tones  | You're doing great.            |
| Grateful   | Gentle wave      | <3     | Warm notes   | Appreciate the little things.  |
| Focused    | Subtle fade      | ^^     | Steady tones | Stay on track.                 |
| Relaxed    | Pulse & fade     | ~~     | Soft tones   | Just breathe.                  |
| Energetic  | Flash + strobe   | **     | Loud tones   | Channel your power!            |

---

## 🧰 Hardware Requirements
- ✅ ESP32 Dev Board
- ✅ NeoPixel Ring (WS2812 – 8 LEDs)
- ✅ SSD1306 OLED Display (128x64 I2C)
- ✅ MPU6050 Accelerometer + Gyroscope
- ✅ TMP36 Analog Temperature Sensor (or NTC)
- ✅ LDR (or potentiometer in Wokwi sim)
- ✅ Piezo Buzzer
- ✅ Push Button
- ✅ Jumper Wires, Breadboard, USB Cable

---

## 💻 Software Requirements
Arduino IDE (or PlatformIO)  
Required Libraries:
- `Adafruit NeoPixel`
- `Adafruit SSD1306`
- `Adafruit GFX`
- `Adafruit MPU6050`
- `Adafruit Sensor`
- `EEPROM` (built-in for ESP32)

---

## 🚀 Getting Started
### 🧪 Run It in Simulation
Try it on Wokwi Simulator – no hardware required!  
🔗 [Live Demo](https://wokwi.com/projects/435401762538160129)

### 💡 Upload to Real Hardware
- Wire up your components (check pin definitions in the code).
- Install the required libraries via the Arduino Library Manager.
- Upload `emotional_lamp_code.ino` to your ESP32 board.
- Power on and enjoy the emotional experience!

---

## 🔍 How It Works
### Sensor Input:
- **LDR** reads ambient light
- **TMP36** reads temperature
- **MPU6050** reads acceleration (motion detection)

### Mood Inference:
- `loop()` includes simple rule-based AI logic to infer mood based on combined sensor data.

### Reaction:
- Based on `currentMood`, the system:
  - Plays a specific melody via `playMelody()`
  - Animates the NeoPixel ring with various visual patterns
  - Shows mood name, emoji, quote, and sensor data on OLED
  - Prints mood + quote to Serial Monitor

### User Input:
- Pressing the **push button** cycles through mood states.
- Mood index is stored using EEPROM.

---

## 📈 Future Improvements
- 🔵 Wi-Fi/Bluetooth mobile app integration
- 📊 Mood history logging to cloud/SD
- 🗣 Smart assistant voice control
- 🖼 Support for RGBW or larger LED matrices
- 🎼 Real-time music sync

---

## 💡 Inspiration
Emotional Lamp is more than a tech demo—it's a blend of mental wellness, art, and embedded design. It's a thoughtful project to promote mindfulness, emotional awareness, and creative exploration through technology.

---

## 🤝 Contributing
Pull requests are welcome! Feel free to add:
- New moods
- More animations
- Sound enhancements
- GUI/mobile control interface

---

## 🧪 Demo & Simulation
🎮 Try it Live on Wokwi: (https://wokwi.com/projects/435401762538160129)
