# Contributing to Flora Smart Pot

Thank you for your interest in contributing to the Flora Smart Pot project! 🌱

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Submitting Changes](#submitting-changes)
- [Coding Standards](#coding-standards)
- [Testing](#testing)

## 📜 Code of Conduct

This project adheres to a code of conduct. By participating, you are expected to uphold this code:

- Be respectful and inclusive
- Focus on what is best for the community
- Show empathy towards other community members
- Be constructive in discussions

## 🚀 Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) installed
- ESP32 development board
- Basic knowledge of C++ and Arduino framework
- Git for version control

### Development Environment

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-username/flora-smart-pot.git
   cd flora-smart-pot
   ```

2. **Install dependencies**
   ```bash
   pio lib install
   ```

3. **Configure secrets**
   ```bash
   cp include/secrets.h.example include/secrets.h
   # Edit secrets.h with your Blynk credentials
   ```

## 🛠️ Development Setup

### Hardware Setup
- Connect ESP32 according to pin configuration in README.md
- Ensure all sensors are properly wired
- Test basic functionality before making changes

### Software Setup
- Use PlatformIO for development
- Configure your IDE with C++ formatting
- Enable serial monitor for debugging

## 🔄 Making Changes

### Branch Naming
- `feature/description` - for new features
- `bugfix/description` - for bug fixes
- `docs/description` - for documentation updates
- `refactor/description` - for code refactoring

### Commit Messages
Follow conventional commit format:
```
type(scope): description

[optional body]

[optional footer]
```

Examples:
- `feat(sensor): add soil moisture calibration`
- `fix(pump): resolve PWM control issue`
- `docs(readme): update pin configuration`

## 📤 Submitting Changes

### Pull Request Process

1. **Create a branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**
   - Write clean, documented code
   - Follow coding standards
   - Add tests if applicable

3. **Test your changes**
   - Build and test on hardware
   - Verify no regressions
   - Check serial output

4. **Commit your changes**
   ```bash
   git add .
   git commit -m "feat(component): add new feature"
   ```

5. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create Pull Request**
   - Provide clear description
   - Reference any related issues
   - Include testing information

### Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Refactoring

## Testing
- [ ] Tested on hardware
- [ ] No compilation errors
- [ ] Serial output verified

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
```

## 📝 Coding Standards

### C++ Style Guide

**Naming Conventions:**
- Functions: `camelCase` (e.g., `readSensorData()`)
- Variables: `camelCase` (e.g., `sensorValue`)
- Constants: `UPPER_CASE` (e.g., `MAX_RETRY_COUNT`)
- Classes: `PascalCase` (e.g., `SensorManager`)

**Code Formatting:**
- Use 4 spaces for indentation
- Maximum line length: 100 characters
- Always use braces for control structures
- Add comments for complex logic

**File Organization:**
- Header files in `include/` directory
- Implementation files in `src/` directory
- One class per file when possible
- Include guards in all headers

### Documentation

**Function Documentation:**
```cpp
/**
 * @brief Brief description of function
 * @param paramName Description of parameter
 * @return Description of return value
 */
```

**Code Comments:**
- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Comment complex algorithms
- Explain "why" not just "what"

## 🧪 Testing

### Hardware Testing
- Test on actual ESP32 hardware
- Verify all sensor readings
- Check pump operation
- Test WiFi connectivity
- Verify Blynk integration

### Code Validation
- No compilation warnings
- Memory usage within limits
- No memory leaks
- Proper error handling

### Testing Checklist
- [ ] Compiles without errors/warnings
- [ ] Hardware functionality verified
- [ ] Serial output clean and informative
- [ ] Power consumption acceptable
- [ ] WiFi connection stable
- [ ] Blynk communication working

## 🐛 Reporting Issues

### Bug Reports
Include:
- ESP32 board type and version
- PlatformIO version
- Steps to reproduce
- Expected vs actual behavior
- Serial output logs
- Hardware configuration

### Feature Requests
Include:
- Clear description of feature
- Use case and benefits
- Proposed implementation approach
- Compatibility considerations

## 📚 Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [Blynk Documentation](https://docs.blynk.io/)
- [Arduino Style Guide](https://www.arduino.cc/en/Reference/StyleGuide)

## 🤝 Community

- Be patient with review process
- Help others in discussions
- Share knowledge and experiences
- Respect different skill levels

Thank you for contributing to Flora Smart Pot! 🌿