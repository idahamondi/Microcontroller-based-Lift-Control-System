# Microcontroller-based Lift Control System

## Overview
Three-floor lift control system with emergency handling, input validation, and remote monitoring via Blynk.

## Hardware Requirements
- Arduino/ESP32
- 16x2 LCD Display
- Relays (4x) - Up, Down, Door Open, Door Close
- Pushbuttons (7x) - Floor 1-3 (outside), Floor 1-3 (inside), Emergency Stop
- LEDs (5x) - Floor indicators, Emergency, Moving
- Buzzer
- Sensors - Floor position (3x), Door open/close, Obstruction, Weight
- ESP32 WiFi module (for Blynk integration)

## Pin Configuration
- Buttons: Pins 2-8
- Relays: Pins 9-12
- LEDs: Pins 13-17
- Buzzer: Pin 18
- Sensors: Pins 19-25
- LCD: Pins 26-31
- Power monitoring: A0

## Setup Instructions
1. Install required libraries:
   - LiquidCrystal
   - BlynkSimpleEsp32
   - WiFi (ESP32)

2. Configure WiFi and Blynk:
   - Replace `YOUR_BLYNK_AUTH_TOKEN` with your Blynk token
   - Replace `YOUR_WIFI_SSID` with your WiFi name
   - Replace `YOUR_WIFI_PASSWORD` with your WiFi password

3. Blynk Virtual Pins:
   - V0: Current Floor
   - V1: Status (MOVING/STOPPED/EMERGENCY)
   - V2: Target Floor
   - V3: Door Status
   - V10: Remote Floor Call (Input)
   - V11: Emergency Stop (Input)

4. Upload code to ESP32/Arduino

## Features
- Three-floor operation
- Call prioritization
- Emergency stop with buzzer and LED
- Overload detection
- Obstruction detection
- Power failure handling
- Input validation and debouncing
- Remote monitoring via Blynk
- LCD status display

## Operation
- Press floor buttons to call lift
- System prioritizes requests based on direction
- Emergency stop button immediately stops lift
- Overload and obstruction sensors prevent unsafe operation
- Power failure automatically lowers lift to ground floor

