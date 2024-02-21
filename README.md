# Logger

## Table of Contents
- [About](#about)
- [Getting Started](#getting-started)
- [Description](#description)
- [Usage](#usage)
- [Performance](#performance-evaluation)
- [Contributing](#contributing)
- [Authors](#authors)
- [Repo Activity](#repo-activity)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## About
This project is a data acquisition system built using an ESP8266 microcontroller board. The system incorporates an ADS1115 analog-to-digital converter, an SSD1306 display, and three buttons for user interaction. It is designed to store data on an SD card or send it to a pc for further analysis.

### Features

- Acquisition of analog sensor data using ADS1115 ADC
- Real-time display of sensor readings on SSD1306 OLED display
- User interaction through three buttons for control and configuration
- Storage of acquired data on an SD card for offline access
- Transmission of data to a computer for centralized analysis


## Getting Started



### Prerequisites

- Arduino IDE or Visual Studio Code (with PlatformIO)
- ESP32 board manager for Arduino IDE
- Libraries:
  - Adafruit ADS1X15 (for ADS1115)
  - Adafruit SSD1306 (for SSD1306 display)
  - SD (for SD card storage)
  - <other libraries as required>

### Installation using Arduino IDE

- Clone or download the project repository from GitHub.
- Open the Arduino IDE and install the ESP32 board manager if not already installed.
- Connect the hardware components as per the provided circuit diagram.
- Install the required libraries using the Arduino Library Manager.
- Upload the code to the ESP32 board using the Arduino IDE.
- Monitor the serial output to ensure successful connection to the sensors.
- Verify the functionality by checking the sensor readings on the SSD1306 display and the data storage on the SD card.

### Installation using Visual Studio Code

- Clone or download the project repository from GitHub.
- Open VS and compile the software (changing the details on platform.ini according to your hardware).
- Connect the hardware components as per the provided circuit diagram.
- Upload the code to the ESP32 board using the upload tool.
- Monitor the serial output to ensure successful connection to the sensors.
- Verify the functionality by checking the sensor readings on the SSD1306 display and the data storage on the SD card.

## Description

## Usage
A few examples of useful commands and/or tasks.

## Performance Evaluation
Discuss how the performance of the logger was evaluated.

## Contributing

Contributions to this project are welcome. If you encounter any issues or have ideas for improvements, please submit a pull request or open an issue on the project repository.

## ✍️ Authors
- [@mrheltic](https://github.com/mrheltic)
- [@Phersax](https://github.com/Phersax)
- [@dav01luffy](https://github.com/dav01luffy)

## Repo Activity

![Alt](https://repobeats.axiom.co/api/embed/d41f5a09ed4fdf91b97787200df89b38aa1a0af6.svg "Repobeats analytics image")

## License
Details about the license.

## Acknowledgments
Any acknowledgments (optional).