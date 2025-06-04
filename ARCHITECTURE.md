# ESP32 Sentinel System Architecture

## Overview

ESP32 Sentinel is an IoT environmental monitoring system that uses FreeRTOS for multitasking and real-time processing. The system collects data from multiple sensors, processes the information, and transmits it via network (WiFi/LTE) to cloud services.

## System Architecture

### Hardware Configuration

The system supports multiple ESP32-based development boards:
- **T-A7670G**: ESP32-S3 with LTE modem
- **T-SIM7670G**: ESP32-S3 with 4G/LTE capability
- **ESP32 DevKit**: Standard ESP32 development board (without LTE)

### FreeRTOS Task Structure

The system is built around a modular task architecture where each sensor and system function has its own FreeRTOS task:

```
Main Task (setup)
├── Network Status Task
├── Sensor Tasks
│   ├── DHT Task (temperature/humidity)
│   ├── Accelerometer Task (motion)
│   ├── Gas Task (MQ-2 gas sensor)
│   ├── Battery Task (battery status)
│   ├── Bluetooth Task (BLE communication)
│   └── GPS Task (positioning)
├── Processing Task (data processing)
└── Communication Task (network transmission)
```

#### Task Priorities and Configuration

| Task | Priority | Stack Size | Frequency |
|------|----------|------------|-----------|
| Network Status | 3 | 4096 | 30s interval |
| DHT Sensor | 2 | 4096 | 10s interval |
| Accelerometer | 2 | 4096 | 5s interval |
| Gas Sensor | 2 | 4096 | 15s interval |
| Battery | 1 | 2048 | 60s interval |
| Bluetooth | 2 | 8192 | Continuous |
| GPS | 2 | 8192 | 30s interval |
| Processing | 3 | 8192 | Event-driven |
| Communication | 3 | 8192 | Event-driven |

### Data Flow and Communication Patterns

#### 1. Sensor Data Collection
```
Sensor Tasks → sensor_message_t → dataQueue → Processing Task
```

Each sensor task:
- Reads sensor data at specified intervals
- Packages data in `sensor_message_t` structure
- Sends to `dataQueue` (FreeRTOS Queue)
- Uses timeout handling for error management

#### 2. Data Processing Flow
```
dataQueue → Processing Task → Aggregation → Communication Queue
```

Processing Task:
- Receives data from all sensor tasks
- Aggregates sensor data into complete messages
- Validates and normalizes data
- Sends to Communication Task

#### 3. Network Communication
```
Communication Task → Network Layer → WiFi/LTE → Cloud Services
```

Communication Task:
- Manages network connections (WiFi primary, LTE backup)
- Implements HTTP/HTTPS communication
- Sends data to external APIs
- Handles authentication and error management

### Data Structures

#### sensor_message_t
Main data structure for communication between tasks:

```cpp
typedef struct {
    sensor_data_t data;      // Actual sensor values
    sensor_valid_t valid;    // Validity flags
    uint32_t timestamp;
    uint8_t source_task;
} sensor_message_t;
```

#### sensor_data_t
Contains all possible sensor values:
- Temperature and humidity (DHT22/DHT11)
- Accelerometer data (X, Y, Z axes)
- Gas levels (MQ-2 sensor)
- Battery status (voltage, charge level)
- GPS position (latitude, longitude, altitude, speed)
- Bluetooth device information

### Synchronization and Resource Management

#### Mutexes and Semaphores
- **modemMutex**: Protects access to LTE/GSM modem
- **networkEventMutex**: Synchronizes network status

#### Event Groups
- **networkEventGroup**: Communicates network connection status
- **systemEventGroup**: Handles system events

#### Queues
- **dataQueue**: Main communication channel for sensor data
- **communicationQueue**: Buffers data for network transmission

### Network Architecture

#### Connection Strategy
1. **WiFi Primary**: Attempts WiFi connection first
2. **LTE Fallback**: Switches to LTE on WiFi failure
3. **Intelligent Reconnection**: Automatic reconnection
4. **Connection Monitoring**: Continuous status monitoring

#### Protocols and APIs
- **HTTP/HTTPS**: RESTful API communication
- **TLS/SSL**: Secure data transmission
- **JSON**: Data serialization format
- **Authentication**: API key-based authentication

### Error Handling and Robustness

#### Watchdog and Recovery
- **Task Watchdog**: Monitors task health
- **Automatic Recovery**: Restarts hanging tasks
- **System Reset**: Complete system restoration on critical failures

#### Logging and Diagnostics
- **Thread-safe Logging**: Safe logging from all tasks
- **Serial Output**: Debug information via UART
- **Error Codes**: Structured error reporting

### Power Management

#### Power Management
- **Deep Sleep**: Power saving mode between measurements
- **CPU Frequency Scaling**: Dynamic frequency regulation
- **Peripheral Power Control**: Selective power supply to sensors

#### Battery Monitoring
- **Voltage Monitoring**: Continuous battery voltage monitoring
- **Low Battery Alerts**: Warnings at low battery levels
- **Charging Status**: Detection of charging process

### Configuration and Customization

#### Hardware Abstraction
- **Board Detection**: Automatic hardware platform identification
- **Pin Mapping**: Flexible pin configuration
- **Sensor Interfaces**: Abstracted sensor interfaces

### Performance and Scalability

#### Memory Management
- **Stack Optimization**: Optimized stack sizes per task
- **Heap Monitoring**: Memory usage monitoring
- **Memory Pools**: Efficient memory allocation

#### Real-time Characteristics
- **Deterministic Timing**: Predictable response times
- **Priority Inheritance**: Avoids priority inversion
- **Interrupt Handling**: Efficient interrupt handling

## Development Guide

### Adding New Sensors
1. Create sensor driver in `src/sensors/`
2. Implement task in `src/tasks/`
3. Define task header in `include/tasks/`
4. Add to `main.cpp` task creation
5. Update `SensorData.h` for new data types

### Testing and Deployment
1. **Unit Testing**: Individual sensor and task testing
2. **Integration Testing**: Full system testing
3. **Performance Testing**: Resource usage validation
4. **Field Testing**: Real-world deployment validation

### Configuration Management
- **Board-specific Configuration**: Hardware-dependent settings
- **Environment Configuration**: Development/production settings
- **Sensor Calibration**: Calibration parameters and procedures
