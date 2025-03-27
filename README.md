# Vehicle Tracking and Theft Alert System

## Overview
This project implements a vehicle tracking and theft alert system using an Arduino microcontroller, GPS module, and GSM module (SIM800L). The system allows users to toggle theft mode via a button, receive real-time GPS location updates, and send alerts via SMS in case of theft.

## Features
- **Theft Mode Activation**: Toggle theft mode using a push button.
- **Ignition Status Monitoring**: Monitor the ignition state of the vehicle.
- **GPS Location Tracking**: Fetch real-time GPS coordinates.
- **SMS Alerts**: Send SMS alerts when the vehicle is stolen or when location is requested.
- **LCD Display**: Display the current mode (Theft/User) and ignition status on an LCD.

## Hardware Requirements
- Arduino Board (e.g., Arduino Uno)
- GPS Module (e.g., Neo-6M)
- GSM Module (e.g., SIM800L)
- Liquid Crystal Display (LCD) 16x2
- Push Buttons (for Theft Mode and Ignition)
- Breadboard and Jumper Wires

## Open the project in the Arduino IDE.

3. Install the required libraries if not already installed:
- `SoftwareSerial`
- `TinyGPS++`
- `LiquidCrystal`

## Connect the hardware components as per the Pin Configuration.

![image](https://github.com/user-attachments/assets/155124f8-3df0-4d1c-ad16-34a7b03081d5)


## Upload the code to your Arduino board.

## Usage
1. **Toggle Theft Mode**: Press the designated button to switch between Theft Mode and User Mode. The current mode will be displayed on the LCD.
2. **Ignition Control**: Use another button to turn the ignition ON/OFF. The ignition status will also be shown on the LCD.
3. **Request Location**: Send an SMS with the keyword "LOCATION" from an authorized number to receive the current GPS coordinates.
4. **Theft Alert**: If theft mode is activated and ignition is turned ON, an SMS alert will be sent to the emergency number if theft is detected.

## Code Explanation
The code includes several key functions:
- `setup()`: Initializes serial communication, sets pin modes, and configures the GSM module for SMS.
- `loop()`: Continuously checks for button presses, processes incoming SMS messages, fetches GPS data, and updates the LCD display.
- `checkButtonPress()`: Toggles theft mode based on button press.
- `checkIgnitionToggle()`: Toggles ignition state based on button press.
- `getGPSData()`: Retrieves GPS coordinates from the GPS module.
- `sendSMS()`: Sends an SMS message to a specified number.

## Acknowledgments
- [Arduino](https://www.arduino.cc/)
- [TinyGPS++ Library](https://github.com/mikalhart/TinyGPSPlus)
- [ResearchPaper](https://drive.google.com/file/d/1ZRaERi79drfxl2U_C5F14e4ZHzzDZDvd/view)
