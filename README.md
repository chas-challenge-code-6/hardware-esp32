# Sentinel ESP32 Hardware Firmware

This repository contains the firmware and hardware documentation for the Sentinel project, based on LilyGO ESP32-S3 and compatible boards. The system is designed for sensor data acquisition, wireless communication, and remote monitoring in challenging environments.

## Features

- **Multi-sensor support:**
  - Accelerometer (MPU9250/MPU6500)
  - Temperature & Humidity (DHT22)
  - Gas detection (MQ2)
  - Battery monitoring
- **Wireless communication:**
  - LTE/4G (TinyGSM)
  - Bluetooth Low Energy (NimBLE)
  - GPS (TinyGPSPlus)
- **Modular FreeRTOS tasks** for sensor reading, communication, and processing
- **3D-printable case design** (see `Prototype-design/Case-design/Format-for-3D-printing/`)
- **PlatformIO** project for easy build and deployment

## Hardware Requirements

- LilyGO T-SIM7670G S3 or T-A7670 (ESP32-S3/ESP32)
- DHT22 sensor
- MQ2 gas sensor
- MPU9250/MPU6500 accelerometer
- LiPo battery (with voltage divider for monitoring)
- Optional: Polar H9 heart rate strap (Bluetooth)

See [LilyGO T-A76XX repo](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX) and `include/utilities.h` for supported board variants and pinouts.

## Software Dependencies

- [PlatformIO](https://platformio.org/)
- Arduino framework
- Libraries (see `platformio.ini`):
  - DHT sensor library
  - MQUnifiedsensor
  - NimBLE-Arduino
  - CustomJWT
  - MPU9250_WE
  - TinyGSM (in `lib/`)
  - TinyGPSPlus (in `lib/`)

## Getting Started

1. **Clone this repository**
2. **Install PlatformIO** (VSCode extension or CLI)
3. **Connect your ESP32 board**
4. **Configure board and secrets:**
   - Edit `platformio.ini` to select your board/environment
   - Copy `include/secrets.h.default` to `include/secrets.h` and configure your network credentials
   - Review `include/config.h` for additional system configuration options
5. **Build and upload:**

   ```sh
   pio run -t upload
   ```

6. **Monitor serial output:**

   ```sh
   pio device monitor
   ```

## Directory Structure

- `src/` - Main application source code (tasks, sensors, network)
- `include/` - Header files and configuration
- `lib/` - External libraries (TinyGSM, TinyGPSPlus)
- `Prototype-design/Case-design/Format-for-3D-printing/` - 3D-printable case files (STL)
- `docs/` - Documentation, diagrams, and pinouts
- `scripts/` - API developing scripts

## Documentation & Resources

- [LilyGO T-A76XX hardware repo](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX)
- [TinyGSM library](https://github.com/vshymanskyy/TinyGSM)
- [TinyGPSPlus library](http://arduiniana.org/libraries/tinygpsplus/)
- 3D case: `Prototype-design/Case-design/Format-for-3D-printing/Base-prototype-v1/Project_Sentinel_Base_Prototype v.1.stl`
- [Class diagram](docs/classes.svg)
- Pinout: [T-SIM7670G-S3](docs/T-SIM7670G-S3-LILYGO_1.jpg), [T-A7670](docs/T-A7670-LILYGO_1.jpg)

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
