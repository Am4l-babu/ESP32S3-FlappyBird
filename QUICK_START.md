# 🚀 Quick Start Guide

Get your **Flappy Bird** game running on ESP32-S3 in 5 minutes!

---

## Step 1: Clone the Repository

```bash
git clone https://github.com/Am4l-babu/ESP32S3-FlappyBird.git
cd ESP32S3-FlappyBird
```

---

## Step 2: Install PlatformIO

### Option A: VS Code Extension (Easiest)
1. Open **Visual Studio Code**
2. Go to Extensions (`Ctrl+Shift+X`)
3. Search for **"PlatformIO"** and install the official extension
4. Reload VS Code

### Option B: Command Line
```bash
pip install platformio
```

---

## Step 3: Wire the Hardware

Connect your components following this quick reference:

```
Xiao ESP32-S3 → OLED SSD1306 (I2C @ 0x3C)
├─ D4 (GPIO5) → SDA
├─ D5 (GPIO6) → SCL
└─ GND → GND

Xiao ESP32-S3 → TTP223 Touch Sensor
├─ D9 (GPIO8) → OUT
└─ GND → GND

Xiao ESP32-S3 → MAX98357A Audio Amplifier
├─ D1 (GPIO2) → DIN
├─ D3 (GPIO4) → LRC
├─ D8 (GPIO7) → BCLK
└─ GND → GND
```

For full wiring details, see [README.md](README.md#wiring-diagram).

---

## Step 4: Connect the Board

1. Plug **Xiao ESP32-S3** into your PC via USB-C
2. Windows: Check `Device Manager` for COM port (e.g., `COM3`)
3. PlatformIO should auto-detect the port

---

## Step 5: Build & Upload

### In VS Code:
```
Ctrl+Shift+B  →  Select "PlatformIO: Build"
Ctrl+Shift+U  →  Select "PlatformIO: Upload"
```

### Via Command Line:
```bash
platformio run --target upload --environment esp32-s3-devkitc-1
```

**Wait** for the upload to complete (~30 seconds).

---

## Step 6: Play!

1. OLED should display the **FLAPPY BIRD** title screen
2. Touch the TTP223 sensor pad to start playing
3. Keep touching to flap upward and avoid pipes

**That's it!** 🎉

---

## Troubleshooting

### No Display?
- Check OLED SDA/SCL wiring (D4/D5)
- Verify I2C address: `0x3C` or `0x3D`
- See [README.md#troubleshooting](README.md#troubleshooting)

### No Audio?
- Verify I2S connections (DIN/LRC/BCLK)
- Check MAX98357A power (3.3V)
- See [README.md#troubleshooting](README.md#troubleshooting)

### Touch Not Working?
- Check GPIO3 (D2) wiring
- Adjust TTP223 potentiometer for sensitivity
- See [README.md#troubleshooting](README.md#troubleshooting)

---

## Next Steps

- **Customize difficulty**: Edit `GRAVITY`, `FLAP_V`, `PIPE_GAP_HALF` in `src/main.cpp`
- **Adjust audio**: Modify `sfxFlap()`, `sfxScore()`, etc.
- **View full documentation**: Read [README.md](README.md)

---

**Questions?** Open an issue on [GitHub](https://github.com/Am4l-babu/ESP32S3-FlappyBird/issues)

Enjoy your game! 🐦✨
