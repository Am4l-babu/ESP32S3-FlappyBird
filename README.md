# 🐦 FLAPPY BIRD — Touch Edition for ESP32-S3

> A fully-featured Flappy Bird game for the **Xiao ESP32-S3** with OLED display, touch controls, and spatial audio via I2S MAX98357A amplifier. Play with a single capacitive touch sensor and enjoy smooth gameplay on a 128×64 OLED screen.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-red)

---

## 📋 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Installation](#installation)
- [Build & Upload](#build--upload)
- [Controls & Gameplay](#controls--gameplay)
- [Game States](#game-states)
- [Audio System](#audio-system)
- [Customization](#customization)
- [Troubleshooting](#troubleshooting)
- [Performance Notes](#performance-notes)
- [Contributing](#contributing)

---

## ✨ Features

### Core Gameplay
- **Smooth Physics**: Realistic gravity and flap mechanics optimized for ESP32-S3
- **Procedurally Generated Pipes**: Infinite scrolling obstacle course with random gap positioning
- **Score Tracking**: High score persistence during session with new score notifications
- **Particle Death Animation**: Explosive bird burst with physics on collision

### Visual Effects
- **128×64 SSD1306 OLED**: Crisp monochrome graphics with optimized rendering
- **Animated Title Screen**: Floating bird with twinkling starfield background
- **Scrolling Star Parallax**: Multi-speed background with depth effect
- **Pulsing UI Elements**: Dynamic prompts and animated countdown timers
- **Transition Animations**: Scan-line wipe + spiral star burst between screens

### Audio System
- **I2S Digital Audio Output**: 16-bit stereo via MAX98357A amplifier
- **Sound Effects**:
  - `sfxFlap()` - Wing flap (480 Hz, 28 ms)
  - `sfxScore()` - Point earned (880 Hz → 1320 Hz duet)
  - `sfxDead()` - Collision descending glissando (550 Hz → 180 Hz)
  - `sfxJingle()` - Boot jingle (C–E–G–C major chord)
- **Dynamic Envelope**: Attack/Release modulation for natural tone shaping

### Smart Features
- **5-Second Idle Auto-Return**: Automatically return to title screen after 5 seconds of inactivity
- **Idle Countdown Bar**: Visual feedback before auto-return
- **Edge Detection**: Single-press flap + hold for rapid flapping
- **Responsive Touch Sampling**: Non-blocking capacitive input via TTP223 sensor

---

## 🔧 Hardware Requirements

### Microcontroller
- **Xiao ESP32-S3** (by Seeed Studio)
  - 32-bit dual-core RISC-V processor
  - 8 MB PSRAM
  - Support for I2S audio output
  - USB-C for programming and power

### Display
- **SSD1306 OLED Module** (128×64 pixels)
  - I2C communication (0x3C default address)
  - 3.3V logic compatible
  - ~50 mA typical current draw

### Touch Input
- **TTP223 Capacitive Touch Sensor Module** (or equivalent)
  - Single-pad capacitive touch
  - Default Mode A: HIGH on touch (active high)
  - ~5 mA current draw
  - Adjustable sensitivity via onboard potentiometer

### Audio
- **MAX98357A I2S Audio Amplifier**
  - 3W @ 4Ω speaker output
  - I2S digital audio input (16-bit, 16 kHz sampling)
  - 3.3V compatible
  - Integrated Class D amplifier

### Power Supply
- USB-C from Xiao ESP32-S3 (5V input)
- Internal 3.3V regulator for logic
- Recommend external 5V adapter for reliable audio output

### Optional
- **Speaker**: 4Ω, 3W (e.g., 40 mm × 40 mm chassis speaker)
- **Breadboard and Jumper Wires** for rapid prototyping

---

## 🔌 Wiring Diagram

```
XIAO ESP32-S3 PIN LAYOUT (Top View)
┌─────────────────────────────────┐
│ 5V  GND D0  D1  D2  D3  D4  D5 │ ← Power Rail
│                                 │
│ 3V3 RST D10 D9 D8  D7  D6  USB │ ← USB/Reset
└─────────────────────────────────┘
```

### Connection Table

| Component       | Pin Name | GPIO | Xiao Label | Notes                    |
|-----------------|----------|------|------------|--------------------------|
| **OLED SDA**    | GPIO5    | 5    | D4         | I2C Data (0x3C address)  |
| **OLED SCL**    | GPIO6    | 6    | D5         | I2C Clock                |
| **TTP223 OUT**  | GPIO3    | 3    | D2         | Active HIGH on touch     |
| **MAX98357A DIN** | GPIO2   | 2    | D1         | I2S Data Out             |
| **MAX98357A LRC** | GPIO4   | 4    | D3         | I2S Word Select (LEFT)   |
| **MAX98357A BCLK** | GPIO7  | 7    | D8         | I2S Bit Clock            |
| **GND**         | —        | —    | GND (×2)   | Common ground for all    |
| **5V (opt)**    | —        | —    | 5V         | External power (optional)|
| **3V3**         | —        | —    | 3V3        | 3.3V logic rail          |

### Breadboard Wiring Example

```
Xiao ESP32-S3     I2C Devices         I2S Audio
─────────────     ───────────         ─────────
  GND ─────┬──→ GND (OLED, TTP223)   GND
           │
  D4 (GPIO5)─→ SDA (OLED @ 0x3C)
  D5 (GPIO6)─→ SCL (OLED @ 0x3C)
  D2 (GPIO3)─→ OUT (TTP223)
  
  D1 (GPIO2)──→ DIN (MAX98357A)
  D3 (GPIO4)──→ LRC (MAX98357A)
  D8 (GPIO7)──→ BCLK (MAX98357A)
  GND ───────→ GND (MAX98357A)
  
  [Speaker] ←──┤ Out L+/R+ (MAX98357A)
               ├──┤ Out L-/R- → GND
```

### Wiring Notes
- **I2C Pull-ups**: Most SSD1306 modules have built-in 10kΩ pull-ups; verify before adding external resistors
- **Audio GND**: Use the same GND as logic for clean audio reference
- **MAX98357A Shutdown**: Connect SD+ pin to 3.3V for always-on operation
- **TTP223 Sensitivity**: Adjust onboard potentiometer for touch responsiveness if needed

---

## 📦 Installation

### Prerequisites
- **PlatformIO** installed (VS Code extension or CLI)
- **Xiao ESP32-S3** connected via USB-C
- **Arduino IDE Framework** (auto-installed via PlatformIO)

### 1. Clone the Repository

```bash
git clone https://github.com/Am4l-babu/ESP32S3-FlappyBird.git
cd ESP32S3-FlappyBird
```

### 2. Install Dependencies

PlatformIO automatically installs libraries from `platformio.ini`:
- **Adafruit SSD1306** v2.5.7+ (display driver)
- **Adafruit GFX** v1.11.9+ (graphics library)

No manual library installation needed—PlatformIO handles it!

### 3. Configure the Board

In **PlatformIO Home** → **Platforms** → **Espressif 32**, ensure:
- Target board: `esp32-s3-devkitc-1`
- Framework: Arduino
- Monitor speed: 115200 baud

---

## 🚀 Build & Upload

### Option 1: PlatformIO VS Code Extension (Recommended)

```bash
# Build the project
Ctrl+Shift+B  → Select "PlatformIO: Build"

# Upload to ESP32-S3 (erase first)
Ctrl+Shift+U  → Select "PlatformIO: Upload"

# Open Serial Monitor
Ctrl+Shift+I  → Select "PlatformIO: Monitor"
```

### Option 2: PlatformIO CLI

```bash
# Build
platformio run --environment esp32-s3-devkitc-1

# Upload (with automatic erase)
platformio run --target upload --environment esp32-s3-devkitc-1

# Monitor serial output
platformio device monitor --baud 115200
```

### Option 3: Manual Erase & Format (Optional)

If you encounter issues with OTA or outdated firmware:

```bash
# Using esptool.py
python -m pip install esptool

# Erase entire flash
esptool.py --chip esp32s3 --port COM3 erase_flash

# Format with empty filesystem
esptool.py --chip esp32s3 --port COM3 -b 460800 write_flash --fs-type littlefs 0x0 blank.bin

# Then re-upload via PlatformIO
platformio run --target upload --environment esp32-s3-devkitc-1
```

**Port Selection**: On Windows, check `Device Manager` for `COM3`, `COM4`, etc.

---

## 🎮 Controls & Gameplay

### Basic Controls

| Input                    | Action                                |
|--------------------------|---------------------------------------|
| **TOUCH (single press)**  | Flap bird upward (immediate)          |
| **TOUCH (hold)**          | Rapid flapping (~2 flaps per second)  |
| No input for 5 sec        | Auto-return to title screen           |

### Game Physics

- **Gravity**: 0.36 units/frame² (downward acceleration)
- **Flap Power**: -3.8 units/frame (instantaneous upward velocity)
- **Max Upward Velocity**: -6.5 units/frame
- **Max Downward Velocity**: +7.0 units/frame
- **Collision Detection**: Pixel-perfect AABB bounding box vs. pipe gaps

### Scoring

- **+1 Point**: Each successfully passed pipe (when bird clears gap)
- **High Score**: Tracked during session; displayed on score screen
- **"* NEW BEST *"**: Animation triggers if current score beats high score

---

## 🎯 Game States

### 1. **TITLE Screen** (ST_TITLE)
- Animated floating bird with wing flap
- Twinkling starfield background (parallax scrolling)
- Pulsing "[ TOUCH TO PLAY ]" prompt
- Idle countdown bar (appears after 2 seconds of inactivity)
- **Duration**: Up to 5 seconds idle → auto-advance to gameplay

### 2. **PLAYING** (ST_PLAYING)
- Active gameplay with pipes scrolling left
- Real-time score display (top-left corner)
- Velocity indicator bar (right edge, vertical)
- Animated ground dashes (scrolling pattern)
- **Death Condition**: Bird touches pipe or falls below ground level

### 3. **DEAD** (ST_DEAD)
- Particle explosion effect (12 particles with gravity)
- Background pipes remain visible for context
- "DEAD!" text flashes after animation settles
- "TOUCH for score" prompt
- **Auto-Advance**: 5 seconds → Score screen (no touch needed)
- **Manual Advance**: Touch sensor → Score screen

### 4. **SCORE Screen** (ST_SCORE)
- Trophy icon (ASCII-art style)
- Animated score count-up (frames /3 accumulation)
- High score display ("BEST: X")
- "* NEW BEST *" banner (if new record achieved)
- Pulsing "[ TOUCH TO RETRY ]" prompt
- Idle countdown bar
- **Touch**: Restart game (new round)
- **Idle (5 sec)**: Return-to-Title animation

### Return-to-Title Transition Animation
- **Phase 1**: Horizontal scan-line wipe (white fills downward, 12 ms per line)
- **Phase 2**: Spiral star burst (6-point spiral clearing screen, 14 ms per frame)
- **Phase 3**: "FLAPPY" text slide-in from left (20 ms per frame)

---

## 🔊 Audio System

### I2S Configuration

```c
I2S_MODE_MASTER | I2S_MODE_TX    // Master transmit mode
Sample Rate: 16000 Hz              // 16 kHz for compact audio
Bits: 16-bit signed integers        // Full dynamic range
DMA Buffers: 4 × 256 samples       // ~64 ms latency
```

### Tone Generation

Dynamic synthesized tones using single-cycle sine wave synthesis with attack/release envelope:

```c
void tone(float freq, int ms, float vol) {
  // Attack: 3 ms ramp-up
  // Release: 8 ms ramp-down
  // Total duration: ms milliseconds
  // Volume: 0.0–1.0 (0.3 default for safe output)
}
```

### Sound Effects Library

| Sound      | Frequency | Duration | Volume | Use Case            |
|------------|-----------|----------|--------|---------------------|
| `sfxFlap()` | 480 Hz   | 28 ms    | 0.18   | Wing flap           |
| `sfxScore()` | 880→1320 | 55+70 ms | 0.28   | Point scored        |
| `sfxDead()` | 550→180  | 240 ms   | 0.35   | Collision           |
| `sfxJingle()` | C–E–G–C | 80×4 ms  | default | Boot sequence       |

### Audio Output Specs

- **Amplitude**: 16-bit signed (±32767 range)
- **I2S Clock**: 16 kHz × 32 bits × 2 channels = 1.024 MHz BCLK
- **LRC (Word Select)**: 16 kHz for stereo alternation
- **Headroom**: Automatic soft clipping at ±0.95 to prevent distortion

---

## ⚙️ Customization

### Adjusting Game Difficulty

Edit `src/main.cpp`:

```c
// Make gravity stronger (fall faster)
#define GRAVITY     0.50f      // Default: 0.36f

// Increase flap power (easier to rise)
#define FLAP_V     -4.5f       // Default: -3.8f

// Change pipe gap width
#define PIPE_GAP_HALF 14       // Default: 11 (gap = 22 pixels)

// Adjust pipe speed
#define PIPE_SPD       3       // Default: 2 (pixels per frame)
```

### Modifying Audio Feedback

```c
// Customize sound volumes
void sfxFlap()  { tone(480, 28, 0.25f); }  // Louder flap
void sfxScore() { 
  tone(880, 55, 0.30f); delay(10); 
  tone(1320, 70, 0.30f);  // Louder score
}

// Add custom jingles
void sfxCustom() { 
  tone(261, 100);  // C
  tone(329, 100);  // E
  delay(50);
}
```

### Changing Idle Timeout

```c
#define IDLE_TIMEOUT_MS  7000   // Change to 7 seconds (default: 5000)
```

### Modifying Screen Elements

```c
// Change star count for parallax effect
for (int i = 0; i < 30; i++)  // Default: 20 stars
  stars[i] = { ... };

// Adjust bird sprite (5×5 pixel bitmap)
const uint8_t SPR_UP[5]   = {0x0E, 0x1F, 0x1E, 0x1F, 0x0C};
const uint8_t SPR_DOWN[5] = {0x0C, 0x1F, 0x1E, 0x1F, 0x0E};
```

---

## 🐛 Troubleshooting

### No Display Output

**Symptom**: OLED remains blank after upload

**Solutions**:
1. Verify I2C address: Check OLED address (typically `0x3C`)
   ```c
   // In setup():
   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {  // Try 0x3D
     Serial.println("OLED not found!");
   }
   ```

2. Check wiring: Verify SDA (GPIO5/D4) and SCL (GPIO6/D5)
3. Check pull-ups: Wire 4.7kΩ resistors from SDA/SCL to 3.3V if display is unresponsive
4. Erase and re-upload: Use `esptool.py` to erase flash completely

### No Audio Output

**Symptom**: Speaker remains silent; no SFX during gameplay

**Solutions**:
1. Verify I2S pin connections (BCLK=GPIO7, LRC=GPIO4, DIN=GPIO2)
2. Check MAX98357A power (3.3V to VCC, GND connected)
3. Verify SD+ pin is tied to 3.3V (enables amplifier)
4. Check speaker polarity (+ to Out L+, − to Out L−)
5. Serial monitor test:
   ```c
   Serial.println("Audio test...");
   sfxJingle();
   ```

### Touch Sensor Not Responding

**Symptom**: Flap doesn't work when touching sensor pad

**Solutions**:
1. Check GPIO3 (D2) wiring to TTP223 OUT
2. Verify TTP223 power (GND connected, 3.3V or 5V to VCC)
3. Adjust TTP223 sensitivity: Turn onboard potentiometer slowly until touch responds
4. Test with Serial output:
   ```c
   Serial.println(digitalRead(TOUCH_PIN));  // Should print 0→1 on touch
   ```

### Game Stutters or Lags

**Symptom**: Framerate inconsistent; bird movement jerky

**Solutions**:
1. Reduce particle count: `#define` change in particle system (line ~200)
2. Disable non-critical animations (e.g., comment out `updateStars()`)
3. Monitor heap usage in Serial output
4. Rebuild with `-O3` optimization (edit `platformio.ini`):
   ```ini
   build_flags = -O3 -DCORE_DEBUG_LEVEL=0
   ```

### Upload Fails: "Timed Out Waiting for Packet"

**Symptom**: Upload disconnects mid-process

**Solutions**:
1. Reduce upload speed in `platformio.ini`:
   ```ini
   upload_speed = 115200  # Default: 460800
   ```
2. Use shorter USB cable (high-speed USB preferred)
3. Disable USB hubs; connect directly to PC
4. Erase and reflash with PlatformIO `erase` task first

### High Score Not Persisting

**Note**: This is by design—high score resets on power cycle. To add EEPROM persistence:

```c
#include <EEPROM.h>
#define EEPROM_ADDR 0

void setup() {
  EEPROM.begin(4);
  hiScore = EEPROM.readInt(EEPROM_ADDR);
}

void saveHighScore() {
  EEPROM.writeInt(EEPROM_ADDR, hiScore);
  EEPROM.commit();
}
```

---

## 📊 Performance Notes

### Memory Usage
- **Flash**: ~400 KB (firmware + libraries)
- **RAM**: ~60 KB during gameplay (PSRAM for audio buffers)
- **PSRAM Available**: 8 MB (sufficient for all assets)

### Frame Timing
- **Title Screen**: 35 FPS (28 ms per frame)
- **Gameplay**: 50 FPS (20 ms per frame)
- **Death Animation**: 50 FPS (20 ms per frame)
- **Score Screen**: 40 FPS (25 ms per frame)

### Power Consumption
- **Idle (Title)**: ~120 mA (OLED + MCU)
- **Gameplay**: ~150 mA (OLED + MCU + I2S active)
- **With Audio**: ~180 mA (peak during SFX playback)

### Optimization Techniques Used
- **Single-buffered display updates** (avoids double-buffer RAM)
- **DMA-based I2S audio** (offloads synthesis from CPU)
- **Bitfield-based sprite data** (5×5 bird = 25 bits packed into 5 bytes)
- **Inline physics calculations** (no complex floating-point operations)

---

## 🤝 Contributing

Contributions are welcome! Areas for enhancement:

- [ ] Add high score EEPROM persistence
- [ ] Implement difficulty levels
- [ ] Create custom bird/pipe skins
- [ ] Add WiFi high score leaderboard
- [ ] Optimize audio synthesis for lower latency
- [ ] Support landscape OLED orientation
- [ ] Multi-player mode (via IR or BLE)

### Development Workflow

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Test thoroughly on hardware
4. Submit pull request with clear description

---

## 📄 License

This project is licensed under the **MIT License** — see `LICENSE` file for details.

---

## 📝 Credits

- **Hardware**: Xiao ESP32-S3 by Seeed Studio
- **Display Driver**: Adafruit SSD1306 library
- **Graphics**: Adafruit GFX library
- **Game Inspiration**: Original Flappy Bird by Dong Nguyen
- **Audio Synthesis**: I2S driver by Espressif Systems

---

## 🔗 Links

- **Repository**: [GitHub](https://github.com/Am4l-babu/ESP32S3-FlappyBird)
- **PlatformIO Docs**: [platformio.org](https://docs.platformio.org)
- **Xiao ESP32-S3 Docs**: [Seeed Wiki](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- **Adafruit SSD1306**: [GitHub](https://github.com/adafruit/Adafruit_SSD1306)
- **MAX98357A Datasheet**: [Maxim](https://datasheets.maximintegrated.com/en/ds/MAX98357A.pdf)

---

## ⭐ Star This Repo!

If you enjoy this project, please give it a star! Your support motivates continued development.

---

**Last Updated**: April 23, 2026  
**Version**: 1.0.0  
**Status**: Stable ✓
