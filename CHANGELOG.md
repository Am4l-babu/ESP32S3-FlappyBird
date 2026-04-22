# Changelog

All notable changes to the **ESP32-S3 Flappy Bird** project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0] - 2026-04-23

### Added
- **Initial Release**: Complete Flappy Bird game for Xiao ESP32-S3
- **Display Features**:
  - 128×64 SSD1306 OLED support via I2C
  - Animated title screen with floating bird and starfield
  - Smooth gameplay rendering at 50 FPS
  - Particle explosion death animation
  - Scan-line wipe + spiral transition effects
  
- **Audio System**:
  - I2S digital audio output (16-bit, 16 kHz)
  - MAX98357A audio amplifier support
  - 4 unique sound effects with attack/release envelopes
  - Dynamic synthesis-based tone generation
  
- **Touch Controls**:
  - TTP223 capacitive touch sensor support
  - Edge detection for single-press vs. hold flapping
  - Responsive input with debouncing
  
- **Game Mechanics**:
  - Realistic physics (gravity, velocity constraints)
  - Procedurally generated obstacle pipes
  - Score tracking with high score display
  - Idle timeout with auto-return to title (5 seconds)
  - Smooth collision detection
  
- **Documentation**:
  - Comprehensive README with wiring diagrams
  - Detailed hardware requirements guide
  - Troubleshooting section
  - Customization examples
  - Performance notes and optimization tips

### Technical Details
- **Platform**: ESP32-S3 (Arduino framework via PlatformIO)
- **Libraries**: Adafruit SSD1306, Adafruit GFX, ESP-IDF I2S driver
- **Flash Usage**: ~400 KB
- **RAM Usage**: ~60 KB gameplay
- **Frame Rates**: Title 35 FPS, Gameplay 50 FPS

---

## [Unreleased] - Future Releases

### Planned Features
- [ ] EEPROM-backed high score persistence
- [ ] Difficulty levels (easy/normal/hard)
- [ ] Custom sprite packs (different birds/pipes)
- [ ] WiFi leaderboard integration
- [ ] Improved audio synthesis (lower latency)
- [ ] Landscape OLED orientation support
- [ ] Multi-touch support for future games
- [ ] BLE connectivity for mobile companion app

### Potential Improvements
- [ ] Power optimization for battery operation
- [ ] Sleep modes during idle
- [ ] Over-the-air (OTA) firmware updates
- [ ] Game pause/resume functionality
- [ ] Difficulty scaling based on score
- [ ] Haptic feedback via vibration motor
- [ ] Background music loop during gameplay
- [ ] Multiple game modes (survival, time attack)

---

## Version History

| Version | Date       | Status   | Notes                              |
|---------|------------|----------|-----------------------------------|
| 1.0.0   | 2026-04-23 | Stable   | Initial release                    |

---

## Migration Guide

### Upgrading from Older Versions
No prior versions exist. This is the initial release!

---

## Known Issues

### None Reported
This is the initial stable release. Please report any issues via GitHub Issues.

---

## Support

For bugs, feature requests, or questions:
- **GitHub Issues**: [ESP32S3-FlappyBird/issues](https://github.com/Am4l-babu/ESP32S3-FlappyBird/issues)
- **GitHub Discussions**: [ESP32S3-FlappyBird/discussions](https://github.com/Am4l-babu/ESP32S3-FlappyBird/discussions)

---

**Last Updated**: April 23, 2026
