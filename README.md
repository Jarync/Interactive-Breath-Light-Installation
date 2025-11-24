# Interactive Breath-Light Installation

![Platform](https://img.shields.io/badge/Platform-Arduino%20UNO-blue.svg)
![Sensor](https://img.shields.io/badge/Sensor-Microphone%20%28Analog%29-orange.svg)
![Output](https://img.shields.io/badge/Output-8x%20WS2812B%20Strips-green.svg)
![License](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey.svg)

## 📖 Introduction

**Interactive Breath-Light Installation** is an art-tech project that visualizes the invisible rhythm of human breathing. By blowing into a microphone sensor, users act as the "life force" of the machine.

The system captures airflow intensity and translates it into a dynamic light show across an array of **8 LED strips**. The stronger the breath, the brighter and faster the lights pulsate. The setup uses a **grouped control strategy**, where multiple physical strips are synchronized to create a massive, cohesive visual impact using minimal microcontroller resources.

### ✨ Key Features
* **Real-time Breath Detection:** Utilizes an analog sound sensor to detect airflow intensity with a custom sliding average filter (`slidingAverageFilterTime`).
* **Grouped Synchronization:** Controls **8 physical LED strips** via 3 logical data channels, creating a large-scale synchronized animation effect.
* **Dynamic Feedback:**
    * **Brightness:** Louder/Stronger breath -> Brighter light (50-255 levels).
    * **Speed:** Stronger breath -> Faster breathing cycle (500ms-2000ms).
* **Adaptive Calibration:** Automatically samples ambient noise on startup to establish a dynamic baseline (`signalMax`/`signalMin`).
* **System Stability:** Integrated hardware Watchdog Timer (`avr/wdt.h`) to prevent system freezes during long-term operation.

---

## 🛠️ Hardware & Bill of Materials (BOM)

Based on the circuit design, the following components are required:

| Component | Type | Quantity | Notes |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | Arduino Uno R3 | 1 | Central Processing Unit |
| **Sensor** | Analog Sound Sensor Module | 1 | e.g., KY-037 or KY-038 (High Sensitivity) |
| **LEDs** | WS2812B LED Strip Segments | **8** | Total 8 strips distributed across 3 groups |
| **Resistors** | 330Ω or 470Ω | 3 | For data line signal integrity |
| **Power Supply** | 5V DC Power Supply | 1 | Amperage depends on total LED count (Rec: 5V 10A+) |
| **Wiring** | Breadboard & Jumpers | 1 Set | Power distribution and signal routing |

*(Note: While the prototype can run on USB power for low brightness/few LEDs, an external 5V power supply is strictly recommended for driving 8 full strips to prevent brownouts.)*

---

## 🔌 Circuit Diagram

The diagram below illustrates the "One-to-Many" connection strategy. The **8 strips** are physically divided into 3 logical groups to share data pins.

<img width="11736" height="11065" alt="microphone_light" src="https://github.com/user-attachments/assets/6e5c9691-3964-4862-935e-dc898ce2d723" />

### Pin Configuration
* **Microphone Sensor:**
    * `A0` -> Arduino `A0` (Analog Signal)
    * `G` -> `GND`
    * `+` -> `5V`
* **LED Data Lines (Grouped Control):**
    * Pin `D6` -> Controls Group 1
    * Pin `D5` -> Controls Group 2
    * Pin `D3` -> Controls Group 3

---

## 💻 How It Works (The Logic)

1.  **Signal Acquisition:** The microphone reads raw analog values representing air pressure/sound.
2.  **Calibration:** On startup, the system samples ambient noise for 1 second (`acquisitionTime`) to determine the noise floor.
3.  **Filtering:** A `slidingAverageFilter` smooths out the jittery analog readings to prevent LED flickering.
4.  **Integration:** The `triangleValue` logic accumulates signal strength over time to distinguish a sustained "breath" from a short "noise spike".
5.  **Mapping & Rendering:**
    * The calculated `airflowValue` (10-150) is mapped to LED brightness and pulse speed.
    * A non-blocking state machine (`state 0/1`) handles the smooth fade-in and fade-out animation for all 3 channels simultaneously.

---

## ©️ Intellectual Property & License

**Copyright © 2025 Chen Junxu. All Rights Reserved.**

### ⚠️ Disclaimer (Prototype Use Only)
This repository contains **prototype firmware** developed for educational and artistic demonstration purposes. It is not a commercial product.

### 📜 Usage Policy (CC BY-NC 4.0)
This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International License**.

1.  **Non-Commercial Use:** You are free to use and modify this code for personal or academic projects.
2.  **No Commercial Deployment:** Strictly prohibited for commercial use without permission.
3.  **Attribution:** Please credit the author (**Chen Junxu**) when using this work.
