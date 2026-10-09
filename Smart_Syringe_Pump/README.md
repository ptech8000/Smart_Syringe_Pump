# Smart Syringe Pump

ESP32 syringe pump with a NEMA 17 stepper (A4988 driver), HX711 load cell, MQTT telemetry (HiveMQ Cloud) and a web page served by the ESP32.

## Files

- `Smart_Syringe_Pump.ino` - firmware
- `dashboard_html.h` - dashboard page
- `login_page.h` - login page
- `PumpTypes.h`, `Secrets.h` - not included here; keep them in the same sketch folder (Wi-Fi and MQTT credentials go in `Secrets.h`)

## Web page

Open `http://smartsyringe.local` and log in with `WEB_AUTH_USER` / `WEB_AUTH_PASSWORD` from the `.ino`. Pages: dashboard, current infusion, safety, calibration, gravimetric test, histories, diagnostics, graphs, validation, stability test, audit log. The page uses plain HTTP, so use it on a trusted network only.

## Settings in the firmware

- `HOME_DIRECTION` - change the sign if HOME drives away from the switch.
- `HOMING_MAX_TRAVEL_FACTOR` - limits how far HOME may travel (multiple of the 12463-step stroke).
- `LIMIT_DEBOUNCE_MS` - a limit switch counts only after 20 ms LOW. Wire switches between the pin and GND (NO contact).
- `OCCLUSION_DETECTION_ENABLED` - currently `false`; motor current is still measured and shown.

Bench testing only. Not for use on patients.
