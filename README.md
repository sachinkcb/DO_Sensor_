# DO_Sensor_

This project showcases the successful integration of a Dissolved Oxygen (DO) sensor with an ESP32 microcontroller on a Pico board. It demonstrates expertise in embedded systems and IoT by seamlessly incorporating various communication protocols and technologies for efficient data management, real-time data transmission, and remote monitoring.

# Features

Sensor Integration: The project integrates a Dissolved Oxygen (DO) sensor with the ESP32 microcontroller, enabling precise measurements of oxygen levels in a liquid medium.

Communication Protocols: Multiple communication protocols, including UART, SPI, I2C, EEPROM, and RTC, have been implemented to facilitate seamless communication with the sensor and ensure efficient data management.

Wi-Fi Connectivity: The ESP32 is equipped with Wi-Fi connectivity, allowing real-time data transmission and remote monitoring. The project leverages FreeRTOS for task scheduling and multitasking, ensuring responsive and reliable data handling.

HTTP for JSON Data Updates: Data is transmitted using the HTTP protocol in JSON format, making it easy to parse and use. This allows for real-time updates of DO measurements, which can be invaluable for industrial decision-making.

Device Configuration System: The project includes a unique device configuration system that enables users to customize settings and parameters according to their specific requirements.

Offline Data Retention: To enhance data accuracy and ensure data is not lost in case of connectivity issues, the project includes a backup storage mechanism for offline data retention.

# Getting Started

Follow these steps to get started with the project:

Hardware Setup: Connect the Dissolved Oxygen (DO) sensor to the ESP32 microcontroller on the Pico board following the provided pinout and connection instructions.

Software Setup: Clone this GitHub repository to your development environment.

Configure Wi-Fi: Edit the Wi-Fi configuration settings in the project to connect the ESP32 to your Wi-Fi network for remote monitoring and data transmission.

Compile and Upload: Use the provided build instructions to compile the code and upload it to the ESP32 microcontroller.

Access Data: Once the code is uploaded and running, access the real-time DO sensor data through the specified HTTP endpoint. You can also configure device settings remotely.
