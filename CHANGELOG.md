# Changelog

## Week 1
*Initial project setup and core architecture*

### **Major Features**
- **Project Initialization**: Set up ESP32-based IoT environmental monitoring system
- **Core Architecture**: Established FreeRTOS task-based architecture
- **Hardware Support**: Added support for LilyGO T-A7670/T-SIM7670G boards
- **Basic Sensor Integration**: Initial DHT22, MQ-2 gas sensor, and accelerometer support

### **Technical Improvements**
- Set up PlatformIO build system with multiple board configurations
- Implemented thread-safe serial communication utilities
- Created modular sensor driver architecture
- Added basic WiFi connectivity support

### **Configuration**
- Initial `config.h` with sensor pin mappings
- Basic `platformio.ini` with ESP32-S3 configurations
- Hardware abstraction layer for different board variants

---

## Week 2
*Enhanced sensor support and data structures*

### **Major Features**
- **Accelerometer Task**: Implemented motion detection and fall detection algorithms
- **Gas Sensor Task**: Added MQ-2 gas sensor with calibration and PPM readings
- **Battery Monitoring**: Created battery voltage monitoring and low-power alerts
- **Data Structures**: Designed comprehensive `sensor_message_t` for task communication

### **Technical Improvements**
- Implemented FreeRTOS queues for inter-task communication
- Added proper task priorities (Critical: 4, High: 3, Medium: 2, Low: 1)
- Created sensor validation and error handling mechanisms
- Added debug output with conditional compilation

### **Bug Fixes**
- Fixed I2C initialization timing issues
- Resolved accelerometer calibration problems
- Corrected gas sensor analog reading stability

---

## Week 3
*Network connectivity and data transmission*

### **Major Features**
- **LTE Connectivity**: Integrated TinyGSM library for cellular communication
- **WiFi Fallback**: Implemented intelligent WiFi-first, LTE-backup strategy
- **HTTP Communication**: Added RESTful API communication with JSON payload
- **Network Status Task**: Created network monitoring and connection management

### **Technical Improvements**
- Added `Network` class for unified connectivity management
- Implemented network event groups for task synchronization
- Created communication task for data transmission
- Added proper modem initialization sequences

### **Bug Fixes**
- Fixed modem power-on timing issues
- Resolved GPRS connection stability problems
- Corrected network reconnection logic

---

## Week 4
*Wireless sensor integration and data processing*

### **Major Features**
- **Bluetooth LE**: Integrated Polar H9 heart rate monitor support
- **Processing Task**: Created data aggregation and processing pipeline
- **Enhanced Fall Detection**: Improved accelerometer-based fall detection
- **Data Validation**: Enhanced sensor data validation and error handling

### **Technical Improvements**
- Implemented NimBLE stack for Bluetooth communication
- Created processing task for sensor data aggregation
- Enhanced error handling and recovery mechanisms
- Improved task coordination and data flow

### **Bug Fixes**
- Fixed Bluetooth connection stability issues
- Corrected processing task data validation
- Resolved inter-task communication timing issues

---

## Week 5
*Algorithm improvements and performance optimization*

### **Major Features**
- **Step Detection**: Implemented orientation-independent step counting algorithm
- **Smart Data Transmission**: Added delta thresholds and periodic updates
- **Authentication System**: Integrated backend JWT authentication
- **Advanced Fall Detection**: Enhanced with orientation-aware detection

### **Technical Improvements**
- Optimized step detection using acceleration magnitude deviation
- Implemented smart gas sensor data transmission (delta + periodic)
- Added backend authentication with token refresh
- Enhanced memory management and stack optimization

### **Bug Fixes**
- Fixed step detection false positives when stationary
- Resolved gas sensor false triggers from vibrations
- Corrected authentication token expiry handling

---

## Week 6
*Robust network communication and data integrity*

### **Major Features**
- **Network Event Synchronization**: Fixed global network object usage across tasks
- **LTE Connection Reliability**: Resolved "no network available" issues
- **Communication Task Optimization**: Used `const char*` for better performance
- **Enhanced HTTP Communication**: Improved error handling and retry logic

### **Technical Improvements**
- Fixed communication task to use global network instance
- Optimized string handling with `const char*` instead of `String` objects
- Enhanced LTE connection verification using only GPRS status
- Improved network reconnection and fallback mechanisms

### **Bug Fixes**
- **CRITICAL**: Fixed LTE connectivity issues - communication tasks now properly detect network
- Resolved local vs global network object synchronization problems
- Fixed `modem.isNetworkConnected()` reliability issues
- Corrected network event group bit management

---

## Week 7
*GPS positioning system and project finalization*

### **Major Features**
- **GPS Tracking**: Added GPS positioning with TinyGPS++ library
- **GPS Network Synchronization**: GPS task now waits for network before initialization
- **Comprehensive Documentation**: Created detailed `ARCHITECTURE.md` and `README.md`
- **Configuration Management**: Proper float constants and type safety
- **Final System Integration**: All tasks properly coordinated and tested

### **Technical Improvements**
- Added GPS task with network-dependent initialization
- Implemented GPS task network dependency and proper synchronization
- Added proper `f` suffix to all float constants in `config.h`
- Enhanced documentation with system architecture diagrams
- Finalized task priorities and resource allocation

### **Bug Fixes**
- Resolved GPS cold start timing problems
- Fixed GPS task to wait for network availability before initialization
- Corrected GPS data validation and error handling

### **Documentation**
- **ARCHITECTURE.md**: Complete system architecture documentation
- **README.md**: Updated with new features and algorithms
- **Code Comments**: Enhanced Doxygen documentation throughout
- **Development Guide**: Instructions for extending the system
