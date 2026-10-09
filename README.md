# P-TECH Smart Syringe Pump

[![Platform](https://img.shields.io/badge/Platform-ESP32-E7352C?logo=espressif)](https://www.espressif.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![Motion](https://img.shields.io/badge/Motion-AccelStepper-blue)](https://www.airspayce.com/mikem/arduino/AccelStepper/)
[![MQTT](https://img.shields.io/badge/MQTT-HiveMQ%20Cloud-6600CC)](https://www.hivemq.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Engineering%20Development-orange)](#project-status)

> **P-TECH Smart Syringe Pump** is an ESP32-based programmable syringe-pump controller with closed-loop engineering diagnostics, calibrated volumetric control, smooth stepper acceleration/deceleration, local web control, MQTT telemetry, load-cell/gravimetric verification, occlusion detection, persistent calibration, history, alarms, graphs, validation tools, and an engineering-focused dashboard.

**Repository:** https://github.com/ptech8000/Smart_Syringe_Pump

---

## ⚠️ Safety / Intended Use

**NOT FOR USE ON PATIENTS.** This project is intended for educational, laboratory, engineering, and bench-testing purposes only. It is not a medical device and has not been validated, certified, or approved for clinical use.

The firmware contains motion, limit-switch, current-monitoring, alarm, and authentication features, but these do **not** make the system suitable for life-critical applications.

---

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [System Architecture](#system-architecture)
- [Hardware](#hardware)
- [Pin Configuration](#pin-configuration)
- [Motion and Syringe Calibration](#motion-and-syringe-calibration)
- [Firmware State Machine](#firmware-state-machine)
- [Web Dashboard](#web-dashboard)
- [Graphs and Analysis](#graphs-and-analysis)
- [Safety and Fault Handling](#safety-and-fault-handling)
- [Occlusion Detection](#occlusion-detection)
- [Load Cell and Gravimetric Verification](#load-cell-and-gravimetric-verification)
- [MQTT Interface](#mqtt-interface)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Compiling and Uploading](#compiling-and-uploading)
- [First Startup](#first-startup)
- [Operating Workflow](#operating-workflow)
- [Increasing Infusion Speed](#increasing-infusion-speed)
- [Calibration Workflow](#calibration-workflow)
- [Project Structure](#project-structure)
- [Troubleshooting](#troubleshooting)
- [Project Status](#project-status)
- [Roadmap](#roadmap)
- [License](#license)

---

## Overview

The Smart Syringe Pump combines a NEMA 17 stepper motor, A4988 driver, ESP32 controller, syringe travel calibration, physical limit switches, motor-current monitoring, HX711 load-cell support, Wi-Fi, MQTT, and a responsive browser dashboard.

The current firmware is built around **true acceleration/deceleration using `AccelStepper::run()`** rather than constant-speed stepping. This allows the pump to ramp into motion, decelerate during a normal completion, decelerate before a pause, and accelerate again when resuming.

The controller also exposes a local engineering interface at:

```text
http://smartsyringe.local
```

when mDNS is available on the local network.

---

## Key Features

### Motion control

- NEMA 17 stepper motor control through A4988.
- Acceleration and deceleration using AccelStepper.
- Smooth pause with controlled deceleration.
- Smooth resume from the paused position.
- Immediate hard STOP for emergency/operator intervention.
- Dedicated homing sequence.
- Homing timeout and maximum-travel protection.
- HOME and MAX limit switches.
- Position-based volumetric control.

### Syringe control

- Calibrated 60 mL travel model.
- Default calibration: **12,463 steps = 60 mL**.
- Default conversion: **207.7167 steps/mL**.
- Configurable calibration stored in ESP32 non-volatile preferences.
- Configurable maximum volume and flow-rate safety limits.

### Monitoring

- Motor-current ADC monitoring.
- Sustained positive current-rise occlusion detection.
- HX711 load-cell support.
- Gravimetric verification.
- Flow-rate and delivered-volume telemetry.
- Position telemetry.
- Alarm history.
- Infusion history.
- Engineering diagnostics.

### Web interface

- ESP32-hosted web dashboard.
- Login-protected dashboard.
- Responsive desktop/tablet/mobile layout.
- Dashboard status monitoring.
- Current infusion control.
- Safety Center.
- Calibration / Setup.
- Calibration Wizard.
- Gravimetric Test.
- Infusion History.
- Alarm History.
- Engineering Diagnostics.
- Graphs & Analysis.
- Validation & Test Center.
- Stability Test.
- Audit Log.
- Release Qualification.
- Interactive graph zoom, pan, reset, and touch-oriented interaction.

### Connectivity

- Wi-Fi.
- mDNS hostname: `smartsyringe.local`.
- Secure MQTT client support.
- HiveMQ Cloud-compatible TLS configuration.
- Local HTTP API for dashboard control and telemetry.

---

## System Architecture

```text
                         ┌──────────────────────────┐
                         │       Web Browser        │
                         │ PC / Laptop / Tablet /   │
                         │          Phone           │
                         └────────────┬─────────────┘
                                      │ HTTP
                                      ▼
                         ┌──────────────────────────┐
                         │      ESP32 Web Server    │
                         │  Dashboard + REST API    │
                         │  Auth + mDNS             │
                         └────────────┬─────────────┘
                                      │
                 ┌────────────────────┼────────────────────┐
                 │                    │                    │
                 ▼                    ▼                    ▼
        ┌────────────────┐   ┌────────────────┐   ┌────────────────┐
        │ Motion Engine  │   │ Safety Engine  │   │ Data Engine    │
        │ AccelStepper   │   │ Limits/Alarm   │   │ HX711/ADC      │
        │ NEMA17/A4988   │   │ Occlusion      │   │ History/Logs   │
        └───────┬────────┘   └───────┬────────┘   └───────┬────────┘
                │                    │                    │
                ▼                    ▼                    ▼
        ┌──────────────┐     ┌──────────────┐     ┌──────────────┐
        │ Syringe      │     │ Limit        │     │ Load Cell    │
        │ Actuator     │     │ Switches     │     │ HX711        │
        └──────────────┘     └──────────────┘     └──────────────┘
                                      │
                                      ▼
                              ┌───────────────┐
                              │ MQTT / Cloud  │
                              │ Telemetry     │
                              └───────────────┘
```

---

## Hardware

### Core controller

| Component | Purpose |
|---|---|
| ESP32 | Main controller, Wi-Fi, web server and MQTT client |
| NEMA 17 | Syringe actuator |
| A4988 | Stepper motor driver |
| Syringe / linear mechanism | Fluid displacement |
| HOME limit switch | Establishes reference position |
| MAX limit switch | Physical travel protection |
| HX711 + load cell | Gravimetric measurement |
| Current-sense circuit | Motor-load/occlusion monitoring |

### Mechanical concept

```text
[NEMA 17]
    │
    ▼
[Coupler]
    │
    ▼
[Lead Screw]
    │
    ▼
[Linear Plunger / Carriage]
    │
    ▼
[Syringe]
```

The firmware's default theoretical mechanical parameters are a **1.25 mm lead-screw pitch**, **200 full steps/revolution**, and **16× microstepping**. The actual volumetric conversion is based on the calibrated travel value rather than relying only on theoretical geometry.

---

## Pin Configuration

| Function | ESP32 GPIO |
|---|---:|
| A4988 STEP | GPIO 25 |
| A4988 DIR | GPIO 26 |
| A4988 ENABLE | GPIO 27 |
| HOME limit switch | GPIO 32 |
| MAX limit switch | GPIO 33 |
| HX711 DOUT | GPIO 16 |
| HX711 SCK | GPIO 17 |
| Motor current ADC | GPIO 34 |

### A4988 enable logic

The driver enable pin is **active LOW**.

```text
GPIO 27 LOW  → Driver enabled
GPIO 27 HIGH → Driver disabled
```

Always provide appropriate motor-driver power, common ground, current limiting, and motor wiring for the specific A4988/NEMA 17 combination.

---

## Motion and Syringe Calibration

The current firmware defines the calibrated syringe travel as:

```text
FULL / HOME position = 0 steps
EMPTY position       = 12,463 steps
Maximum volume       = 60 mL
```

Therefore:

```text
60 mL / 12,463 steps
≈ 207.7167 steps/mL
```

The firmware converts target volume to motor position using the calibrated `calibratedStepsPerML` value.

### Important

The theoretical mechanical calculation and the calibrated volumetric calculation are separate. This is intentional: real mechanisms have manufacturing tolerances, backlash, compression, syringe variation, and coupling errors.

For experimental work, calibrate the actual assembled mechanism rather than assuming the theoretical value is exact.

---

## Firmware State Machine

The firmware defines these operating states:

```text
STATE_IDLE
STATE_HOMING
STATE_PRIMING
STATE_INFUSING
STATE_PAUSED
STATE_OCCLUDED
STATE_COMPLETE
STATE_ALARM
```

Typical flow:

```text
             ┌─────────────┐
             │     IDLE    │
             └──────┬──────┘
                    │ HOME
                    ▼
             ┌─────────────┐
             │   HOMING    │
             └──────┬──────┘
                    │
                    ▼
             ┌─────────────┐
             │   PRIMING   │
             └──────┬──────┘
                    │ START
                    ▼
             ┌─────────────┐
             │  INFUSING   │◄─────────────┐
             └───┬─────┬───┘              │
                 │     │                  │ RESUME
              PAUSE   FAULT               │
                 │     ▼                  │
                 ▼  OCCLUDED              │
             ┌─────────────┐              │
             │   PAUSED    │──────────────┘
             └──────┬──────┘
                    │ STOP
                    ▼
                  IDLE
```

A normal infusion completion enters `STATE_COMPLETE`. A serious safety condition enters an alarm/fault state and requires operator intervention.

---

## Web Dashboard

The dashboard is embedded directly in the firmware through `dashboard_html.h`. This keeps the user interface on the ESP32 rather than requiring a separate web server.

### Main sections

| Section | Purpose |
|---|---|
| Dashboard | Live pump state and primary metrics |
| Current Infusion | Run configuration and active infusion control |
| Safety Center | Limit switches, safety state and faults |
| Calibration / Setup | Engineering configuration |
| Calibration Wizard | Guided calibration workflow |
| Gravimetric Test | Compare commanded and measured volume |
| Infusion History | Historical infusion records |
| Alarm History | Recorded alarms/faults |
| Engineering Diagnostics | Raw engineering measurements |
| Graphs & Analysis | Historical/live data visualization |
| Validation & Test Center | Structured verification workflows |
| Stability Test | Extended operation testing |
| Audit Log | Operator/system activity records |
| Release Qualification | Engineering release checks |

### Access

After Wi-Fi connection, try:

```text
http://smartsyringe.local
```

If mDNS is unavailable, use the ESP32's IP address:

```text
http://<ESP32-IP>
```

---

## Graphs and Analysis

The dashboard includes a dedicated **Graphs & Analysis** section for engineering data visualization.

The graph interface is designed for progressive use across desktop and touch devices and supports:

- Interactive zoom.
- Pan.
- Reset view.
- Touch interaction/pinch-oriented zoom.
- Current/telemetry visualization.
- Engineering analysis of pump behavior.

This is useful for examining acceleration behavior, current changes, delivery trends, and other validation data without needing a separate desktop application.

---

## Safety and Fault Handling

The firmware implements multiple layers of protection.

### Hard STOP

STOP is deliberately different from PAUSE.

**STOP:**

- Immediately interrupts motion.
- Clears the current run.
- Can clear an occlusion fault.
- Forces re-homing before another controlled run when required.

**PAUSE:**

- Requests controlled deceleration.
- Stops after the motion profile has safely slowed.
- Preserves the current position.
- Allows a smooth RESUME from the paused position.

### Homing protection

Homing includes:

- Timeout protection.
- Maximum travel protection.
- Safe interruption.
- Invalidating `homed` if homing is interrupted.
- Prevention of inappropriate HOME commands while the pump is actively infusing, paused, or already homing.

---

## Occlusion Detection

The motor-current monitor is designed to identify a sustained increase in motor load rather than treating every current fluctuation as an occlusion.

The firmware uses:

- Current sampling.
- Exponential moving average filtering.
- A learned normal-running baseline.
- Positive current-rise detection.
- Sustained threshold duration.
- Startup/grace period.
- Baseline adaptation during normal operation.

Relevant default parameters include:

```cpp
const int OCCLUSION_DELTA_COUNTS = 400;
const unsigned long OCCLUSION_HOLD_MS = 500;
const unsigned long OCCLUSION_GRACE_MS = 1500;
const unsigned long OCCLUSION_LEARN_MS = 1500;
const float CURRENT_EMA_ALPHA = 0.1;
const float OCCLUSION_BASELINE_ALPHA = 0.01;
```

These values should be experimentally tuned for the actual motor, driver, mechanism, syringe, current-sense circuit, and load.

---

## Load Cell and Gravimetric Verification

The HX711 interface provides an optional independent measurement of fluid delivery.

The gravimetric workflow can compare:

```text
Commanded volume
       vs.
Measured mass
       ↓
Mass / density
       ↓
Measured volume
       ↓
Volume error (%)
```

The firmware stores configurable:

- HX711 scale factor.
- Fluid density.
- Initial mass.
- Final mass.
- Calculated measured volume.
- Error percentage.
- Test result.

This allows the pump's commanded displacement to be checked against an external measurement rather than relying solely on motor position.

---

## MQTT Interface

The MQTT base topic is:

```text
ptech/syringepump/sp01
```

### Topic layout

| Topic | Purpose |
|---|---|
| `ptech/syringepump/sp01/cmd/#` | Command namespace |
| `ptech/syringepump/sp01/status` | Current pump status |
| `ptech/syringepump/sp01/event` | Events and alarms |
| `ptech/syringepump/sp01/telemetry/flowrate` | Flow rate telemetry |
| `ptech/syringepump/sp01/telemetry/volume_delivered` | Delivered volume |
| `ptech/syringepump/sp01/telemetry/motor_current` | Motor-current telemetry |
| `ptech/syringepump/sp01/telemetry/position` | Position telemetry |
| `ptech/syringepump/sp01/telemetry/loadcell` | Load-cell telemetry |

The firmware uses `PubSubClient` and `WiFiClientSecure` for MQTT communication.

> Keep MQTT credentials and certificates out of public repositories. Use a local `Secrets.h` containing real credentials only on the development machine/device.

---

## Software Requirements

### Recommended environment

- Arduino IDE 1.8.19 or a compatible Arduino IDE environment.
- ESP32 Arduino core.
- USB data cable.
- ESP32 development board.
- Required third-party libraries listed below.

### Libraries

Install the following libraries through Arduino Library Manager where available:

```text
AccelStepper
PubSubClient
HX711
ArduinoJson
```

The firmware also uses ESP32/Arduino platform libraries including:

```text
WiFi
ESPmDNS
WiFiClientSecure
WebServer
Preferences
```

These are provided by the ESP32 Arduino core.

---

## Installation

Clone the repository:

```bash
git clone https://github.com/ptech8000/Smart_Syringe_Pump.git
cd Smart_Syringe_Pump
```

Or download the repository as a ZIP from GitHub.

Open:

```text
syringe_pump_v3.ino
```

in Arduino IDE.

Keep all project header files in the same sketch directory:

```text
syringe_pump_v3.ino
PumpTypes.h
Secrets.h
dashboard_html.h
login_page.h
```

---

## Configuration

### 1. Configure secrets

Edit `Secrets.h` locally:

```cpp
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST     = "YOUR_HIVEMQ_CLUSTER";
const uint16_t MQTT_PORT  = 8883;
const char* MQTT_USERNAME = "YOUR_MQTT_USERNAME";
const char* MQTT_PASSWORD = "YOUR_MQTT_PASSWORD";
```

Add the correct CA certificate in `HIVEMQ_ROOT_CA`.

**Do not publish real Wi-Fi, MQTT passwords, API keys, or private certificates.**

### 2. Configure dashboard credentials

The firmware currently contains local web-dashboard credentials in `syringe_pump_v3.ino`.

Change them before deploying:

```cpp
const char* WEB_AUTH_USER = "CHANGE_ME";
const char* WEB_AUTH_PASSWORD = "CHANGE_ME";
```

The dashboard authentication is intended for trusted LAN/VPN use. It is **not** a replacement for TLS or a hardened public-internet security architecture.

### 3. Review engineering limits

Important parameters include:

```cpp
const float MAX_VOLUME_ML = 60.0;
const float MAX_RATE_ML_HR = 300.0;
const float MAX_STEPS_PER_SEC = 1500.0;
const float INFUSION_ACCEL = 80.0;
```

Do not increase limits without verifying the mechanical system, motor torque, driver configuration, syringe mechanism, current sensing, and physical safety margins.

---

## Compiling and Uploading

### Arduino IDE

1. Install the ESP32 board package.
2. Select the appropriate ESP32 board.
3. Select the correct COM port.
4. Install the required libraries.
5. Place all project files in the sketch folder.
6. Fill in `Secrets.h`.
7. Compile/Verify.
8. Connect the ESP32 by USB.
9. Upload.
10. Open Serial Monitor and inspect startup/network messages.

### Recommended serial monitor

Use the baud rate defined by the firmware's `Serial.begin(...)` configuration. If you change the firmware baud rate, update the Serial Monitor accordingly.

---

## First Startup

A recommended first-run sequence is:

```text
Power on
   ↓
Check wiring
   ↓
Verify HOME/MAX switches
   ↓
Connect Wi-Fi
   ↓
Open dashboard
   ↓
HOME the mechanism
   ↓
Verify position = 0
   ↓
Run a low-speed dry mechanical test
   ↓
Verify direction
   ↓
Verify STOP
   ↓
Verify PAUSE / RESUME
   ↓
Verify current monitoring
   ↓
Perform calibration
   ↓
Run gravimetric verification
   ↓
Only then increase operating speed
```

Never begin with a high-speed or high-volume test on an unverified mechanism.

---

## Operating Workflow

### Normal controlled infusion

1. Power the system.
2. Verify the mechanism is mechanically safe.
3. Open the dashboard.
4. Home the pump.
5. Configure target volume.
6. Configure flow rate.
7. Confirm syringe calibration.
8. Start the infusion.
9. Monitor state, position, current and delivered volume.
10. Pause or stop when required.
11. Review the infusion record after completion.

### Pause / Resume

Pause is intended for controlled interruption:

```text
INFUSING
   ↓ PAUSE
controlled deceleration
   ↓
PAUSED
   ↓ RESUME
controlled acceleration
   ↓
INFUSING
```

### Stop

Use STOP when immediate motion interruption is required.

After a safety-related stop, re-home before relying on position-based volumetric control.

---

## Increasing Infusion Speed

The firmware limits flow rate and step speed. The primary configuration parameters are:

```cpp
const float MAX_RATE_ML_HR = 300.0;
const float MAX_STEPS_PER_SEC = 1500.0;
const float INFUSION_ACCEL = 80.0;
```

Do **not** simply increase all three values.

A safe engineering approach is:

1. Verify the current calibration.
2. Test at a low flow rate.
3. Confirm the NEMA 17 does not skip steps.
4. Confirm the A4988 current limit is correctly configured.
5. Check mechanical binding and syringe friction.
6. Monitor motor-current behavior.
7. Increase flow rate gradually.
8. Validate delivered volume gravimetrically.
9. Check that acceleration is adequate for the increased speed.
10. Re-test the complete operating range.

If the pump is losing steps, increasing speed further will reduce—not improve—actual volumetric accuracy.

---

## Calibration Workflow

The recommended calibration process is empirical.

### Step 1 — Home

Establish the known full/home reference position.

### Step 2 — Measure actual travel

Determine the real linear displacement corresponding to the syringe's usable volume.

### Step 3 — Establish volumetric conversion

The firmware currently starts from:

```text
12,463 steps / 60 mL
```

### Step 4 — Perform a measured delivery

Command a known volume and collect the output using an appropriate measurement method.

### Step 5 — Gravimetric verification

Use the load cell and fluid density to estimate actual delivered volume.

### Step 6 — Update calibration

Adjust the calibrated steps-per-mL value based on measured results.

### Step 7 — Repeat

Perform multiple tests at different volumes and flow rates. A single calibration point is not enough to establish system performance across the entire operating range.

---

## Project Structure

```text
Smart_Syringe_Pump/
│
├── syringe_pump_v3.ino   # Main ESP32 firmware
├── PumpTypes.h            # Pump state-machine definitions
├── Secrets.h              # Local Wi-Fi/MQTT/TLS configuration
├── dashboard_html.h       # Embedded responsive web dashboard
├── login_page.h           # Embedded login page
├── LICENSE                # MIT license
└── README.md              # Project documentation
```

### `syringe_pump_v3.ino`

Contains the main firmware, motion control, networking, MQTT, web API, calibration, diagnostics, history, alarms, current monitoring, load-cell support, and pump state management.

### `PumpTypes.h`

Keeps the `PumpState` enum in a separate header so Arduino's `.ino` prototype generation sees the type before generated function declarations.

### `Secrets.h`

Stores local credentials and the MQTT CA certificate. Keep real credentials private.

### `dashboard_html.h`

Contains the embedded browser dashboard, including responsive layout and engineering tools.

### `login_page.h`

Contains the embedded login interface used before access to the dashboard.

---

## Troubleshooting

### Dashboard does not open

Try the ESP32 IP address instead of mDNS:

```text
http://<ESP32-IP>
```

Check:

- ESP32 is connected to Wi-Fi.
- Computer/phone is on the same network.
- mDNS is supported by the client network.
- HTTP server started successfully.

### `smartsyringe.local` does not resolve

mDNS availability depends on the client operating system, network, and router.

Use the ESP32 IP address as a fallback.

### MQTT does not connect

Verify:

- MQTT hostname.
- Port, normally TLS `8883` for HiveMQ Cloud configurations.
- Username/password.
- CA certificate.
- ESP32 system time/TLS prerequisites.
- Wi-Fi connectivity.

### Motor moves in the wrong direction

Check the motor wiring and direction logic. For homing, review:

```cpp
const int HOME_DIRECTION = -1;
```

If the home motion is reversed relative to the switch, configure the direction appropriately and test at low speed.

### Pump loses steps

Possible causes:

- Excessive speed.
- Excessive acceleration.
- Mechanical binding.
- Insufficient motor torque.
- Incorrect A4988 current limit.
- Poor power supply.
- Excessive syringe/plunger friction.
- Mechanical misalignment.

### Occlusion alarm triggers too easily

Review:

```cpp
OCCLUSION_DELTA_COUNTS
OCCLUSION_HOLD_MS
OCCLUSION_GRACE_MS
OCCLUSION_LEARN_MS
CURRENT_EMA_ALPHA
OCCLUSION_BASELINE_ALPHA
```

Tune using measured motor-current data rather than guessing.

### Volume is inaccurate

Check:

1. Home repeatability.
2. Stepper direction.
3. Mechanical backlash.
4. Lead-screw pitch.
5. Syringe dimensions.
6. Calibrated steps/mL.
7. Missed steps.
8. Syringe/plunger friction.
9. Gravimetric measurement.
10. Fluid density used for conversion.

---

## Project Status

**Engineering development / bench-test stage.**

The repository currently contains a substantial integrated firmware/dashboard implementation, including motion control, local UI, MQTT connectivity, calibration, diagnostics, histories, safety functions, graphs, validation tooling, and gravimetric verification support.

It should still be treated as an engineering development project rather than a certified medical or clinical device.

---

## Roadmap

Potential future development areas include:

- [ ] Formal hardware schematic and PCB revision.
- [ ] Dedicated emergency-stop hardware independent of firmware.
- [ ] More extensive automated calibration routines.
- [ ] Automated repeatability/accuracy test reports.
- [ ] Persistent configuration versioning.
- [ ] Stronger session/authentication architecture.
- [ ] TLS-secured dashboard deployment where required.
- [ ] Hardware watchdog and brownout/recovery validation.
- [ ] Motor-step verification / encoder feedback.
- [ ] Expanded sensor redundancy.
- [ ] Automated CI compile checks.
- [ ] Formal release qualification documentation.
- [ ] Mechanical assembly documentation.
- [ ] Production-oriented PCB and enclosure.

---

## Development Notes

This project intentionally separates three different engineering concepts:

### 1. Commanded motion

How many motor steps the controller requests.

### 2. Position-derived volume

How those steps map to syringe displacement using calibration.

### 3. Independent measurement

What the load cell/gravimetric system reports.

Keeping these concepts separate makes it possible to identify whether an error originates in firmware, calibration, mechanics, or fluid measurement.

---

## Contributing

Contributions, testing feedback, hardware improvements, and engineering suggestions are welcome.

For significant changes:

1. Create a feature branch.
2. Describe the hardware/software assumptions.
3. Test motion at low speed first.
4. Document calibration changes.
5. Document safety implications.
6. Submit a pull request with test results.

Never submit real credentials or private certificates.

---

## License

This project is released under the **MIT License**. See [`LICENSE`](LICENSE) for the complete license text.

Copyright © 2026 P-TECH.

---

## Author / Project

**P-TECH — Precision Technology, Engineering & Creative Hardware**

- GitHub: https://github.com/ptech8000
- Project: https://github.com/ptech8000/Smart_Syringe_Pump

For engineering/project enquiries:

- Email: ptech8000@gmail.com
- Phone: 09069001906

---

> **P-TECH Smart Syringe Pump** — precision motion, measurable delivery, engineering-focused control.
