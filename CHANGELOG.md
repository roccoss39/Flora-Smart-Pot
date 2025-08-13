# Changelog

All notable changes to the Flora Smart Pot project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Project documentation (README.md, LICENSE, CHANGELOG.md)
- Requirements documentation

### Changed
- Standardized all comments to English language
- Improved code organization and documentation

### Fixed
- Fixed .gitignore path for secrets.h file
- Removed unnecessary test files

## [1.3.0] - 2025-01-XX

### Added
- Enhanced alarm system with priority-based beeping
- Improved LED status management
- Better error handling and logging

### Changed
- Refactored alarm manager for better modularity
- Improved pump control with PWM support
- Enhanced configuration management

### Fixed
- Fixed alarm sound control via Blynk
- Improved sensor reading reliability

## [1.2.0] - 2024-XX-XX

### Added
- Deep sleep power management
- Button wake-up functionality
- Battery monitoring with low voltage alerts
- Configurable measurement intervals

### Changed
- Optimized power consumption
- Improved WiFi connection handling

### Fixed
- Fixed memory leaks in continuous mode
- Improved sensor calibration stability

## [1.1.0] - 2024-XX-XX

### Added
- Blynk cloud integration
- Real-time data transmission
- Remote pump control via mobile app
- Virtual pin configuration for Blynk

### Changed
- Restructured code into modular components
- Improved configuration system

### Fixed
- Fixed WiFi reconnection issues
- Improved sensor reading accuracy

## [1.0.0] - 2024-XX-XX

### Added
- Initial release
- Basic soil moisture monitoring
- Water level detection (5 levels)
- Automatic pump control
- DHT11 temperature/humidity sensor
- WiFi configuration portal
- Local alarm system
- LED status indicators

### Features
- Soil moisture sensor with calibration
- Multi-level water detection
- PWM pump control
- Environmental monitoring
- Audio/visual alarms
- WiFi connectivity
- Configuration persistence

---

## Legend

- **Added** for new features
- **Changed** for changes in existing functionality
- **Deprecated** for soon-to-be removed features
- **Removed** for now removed features
- **Fixed** for any bug fixes
- **Security** for vulnerability fixes