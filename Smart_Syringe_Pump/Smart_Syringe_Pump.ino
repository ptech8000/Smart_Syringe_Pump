/*
 * Smart Syringe Pump
 * ESP32 + A4988 + NEMA 17, HX711 load cell, MQTT (HiveMQ) and a local web page.
 *
 * Pins: STEP 25, DIR 26, ENABLE 27 (active LOW), HOME switch 32, MAX switch 33,
 *       HX711 DOUT 16 / SCK 17, motor current ADC 34.
 * Travel: 0 steps = syringe full (HOME), 12463 steps = 60 mL discharged.
 * MQTT base topic: syringepump/sp01
 *
 * Bench testing only. Not for use on patients.
 */

#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <HX711.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "PumpTypes.h"
#include "Secrets.h"

// Device configuration

const char* DEVICE_ID = "sp01";
const char* MDNS_HOSTNAME = "smartsyringe";
bool mdnsStarted = false;

// Web dashboard login
const char* WEB_AUTH_USER = "Toriseju";
const char* WEB_AUTH_PASSWORD = "toriseju101";

// Syringe calibration

const float SYRINGE_MAX_VOLUME_ML = 60.0;
const long SYRINGE_FULL_POSITION_STEPS = 0;
const long SYRINGE_EMPTY_POSITION_STEPS = 12463;

const float CALIBRATED_STEPS_PER_ML_DEFAULT =
    (float)SYRINGE_EMPTY_POSITION_STEPS / SYRINGE_MAX_VOLUME_ML;

float calibratedStepsPerML = CALIBRATED_STEPS_PER_ML_DEFAULT;

// Theoretical mechanical information

const float LEAD_SCREW_PITCH_MM = 1.25;
const int STEPS_PER_REV = 200;
const int MICROSTEPS = 16;
const float SYRINGE_BARREL_ID_MM = 90.6;

// Safety limits

const float MAX_VOLUME_ML = 60.0;
const float MAX_RATE_ML_HR = 300.0;
const float MAX_STEPS_PER_SEC = 1500.0;
const bool REQUIRE_HOMING = true;

// Infusion acceleration (steps/s^2). Lower is smoother, higher is faster but more vibration.
const float INFUSION_ACCEL = 80.0;

// Homing

// Lower speed and acceleration give quieter, smoother homing.
// Raise them for faster homing if the motor runs quietly.
const float HOMING_SPEED_STEPS_S = 800.0;
const float HOMING_ACCEL = 400.0;
const unsigned long HOMING_TIMEOUT_MS = 240000;

// Maximum homing travel as a multiple of the calibrated syringe travel.
// After power-up the step counter starts at 0 wherever the carriage really is,
// so homing must be allowed to travel further than one full syringe stroke.
const float HOMING_MAX_TRAVEL_FACTOR = 2.0;

// Direction the motor must turn to travel toward the HOME switch (GPIO 32).
//  -1 = negative step direction (default)
//  +1 = positive step direction
// If HOME makes the motor run away from the switch, change this sign.
const int HOME_DIRECTION = -1;

// PAUSE decelerates smoothly; STOP is an immediate stop.
const unsigned long PAUSE_TIMEOUT_MS = 10000;

// Occlusion

// Occlusion detection is turned OFF. The pump will not raise an occlusion
// alarm or stop on a motor-current rise. Set to true to turn it back on.
const bool OCCLUSION_DETECTION_ENABLED = false;
const int OCCLUSION_DELTA_COUNTS = 400;

// Current must remain above the threshold for this long.
const unsigned long OCCLUSION_HOLD_MS = 500;

// Time allowed for the motor to start before current monitoring begins.
const unsigned long OCCLUSION_GRACE_MS = 1500;

// Learn normal RUNNING current after the grace period.
const unsigned long OCCLUSION_LEARN_MS = 1500;
const unsigned long CURRENT_SAMPLE_MS = 10;
const float CURRENT_EMA_ALPHA = 0.1;

// Slow baseline adaptation during normal operation.
const float OCCLUSION_BASELINE_ALPHA = 0.01;
const unsigned long CURRENT_DEBUG_INTERVAL_MS = 500;

// Network

const unsigned long WIFI_RETRY_MS = 5000;
const unsigned long MQTT_RETRY_MS = 5000;
const unsigned long OFFLINE_PAUSE_MS = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 500;

// Pins

#define PIN_STEP 25
#define PIN_DIR 26
#define PIN_ENABLE 27

#define PIN_LIMIT_HOME 32
#define PIN_LIMIT_MAX 33

#define PIN_HX711_DOUT 16
#define PIN_HX711_SCK 17

#define PIN_CURRENT_ADC 34

// Theoretical calculations

float theoreticalStepsPerMM() {
  return (STEPS_PER_REV * MICROSTEPS) / LEAD_SCREW_PITCH_MM;
}

float theoreticalStepsPerML() {
  float barrelAreaMM2 = PI * pow(SYRINGE_BARREL_ID_MM / 2.0, 2);

  float mLPerMM = barrelAreaMM2 / 1000.0;

  return theoreticalStepsPerMM() / mLPerMM;
}

// Calibrated volume functions

long volumeToSteps(float volumeML) {
  if (volumeML < 0.0) {
    volumeML = 0.0;
  }

  if (volumeML > SYRINGE_MAX_VOLUME_ML) {
    volumeML = SYRINGE_MAX_VOLUME_ML;
  }

  return lroundf(volumeML * calibratedStepsPerML);
}

float positionToVolume(long position) {
  if (position < SYRINGE_FULL_POSITION_STEPS) {
    position = SYRINGE_FULL_POSITION_STEPS;
  }

  if (position > SYRINGE_EMPTY_POSITION_STEPS) {
    position = SYRINGE_EMPTY_POSITION_STEPS;
  }

  return (float)position / calibratedStepsPerML;
}

long volumeToPosition(float volumeML) {
  return volumeToSteps(volumeML);
}

// State

volatile PumpState currentState = STATE_IDLE;

String alarmMessage = "";
bool homed = false;

// Stepper

AccelStepper stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);

// Hx711

HX711 scale;

// Network

WiFiClientSecure wifiClient;

PubSubClient mqtt(wifiClient);

WebServer webServer(80);

// Infusion variables

float SPM = calibratedStepsPerML;
float targetVolumeML = 0.0;
float deliveredVolumeML = 0.0;
float flowRateMLPerHr = 0.0;
long totalStepsForRun = 0;
long stepsCompleted = 0;
long infusionStartPos = 0;
long infusionTargetPosition = 0;

// Pause variables

bool pauseRequested = false;
unsigned long pauseStartMs = 0;

// Current / occlusion

float currentFiltered = 0;
float currentBaseline = 0;
unsigned long lastCurrentSampleMs = 0;
unsigned long occlusionSinceMs = 0;
unsigned long graceStartMs = 0;
unsigned long occlusionLearnStartMs = 0;
unsigned long lastCurrentDebugMs = 0;
bool occlusionBaselineLocked = false;

// Homing

unsigned long homingStartMs = 0;

// Connectivity

unsigned long offlineSinceMs = 0;
unsigned long lastWifiTryMs = 0;
unsigned long lastMqttTryMs = 0;
unsigned long lastTelemetryMs = 0;

// Local web dashboard

String lastWebEvent = "system ready";
unsigned long lastWebEventMs = 0;
bool webServerStarted = false;

// Saved configuration

Preferences pumpPrefs;

float hx711ScaleFactor = 1.0f;
float fluidDensityGPerML = 1.0f;
const float MIN_CALIBRATED_STEPS_PER_ML = 50.0f;
const float MAX_CALIBRATED_STEPS_PER_ML = 1000.0f;

// Infusion history

const uint8_t MAX_INFUSION_HISTORY = 20;

struct InfusionRecord {
  uint32_t id;
  unsigned long startMs;
  unsigned long endMs;
  float targetML;
  float pumpDeliveredML;
  float rateMLHr;
  bool gravimetric;
  float densityGPerML;
  float initialGrams;
  float finalGrams;
  float measuredML;
  float errorPct;
  char result[24];
};

InfusionRecord infusionHistory[MAX_INFUSION_HISTORY];
uint8_t infusionHistoryCount = 0;
int activeInfusionHistory = -1;
uint32_t nextInfusionId = 1;

// Alarm history

const uint8_t MAX_ALARM_HISTORY = 30;

struct AlarmRecord {
  unsigned long uptimeMs;
  char state[16];
  char message[96];
};

AlarmRecord alarmHistory[MAX_ALARM_HISTORY];
uint8_t alarmHistoryCount = 0;

// Gravimetric test

bool gravimetricActive = false;
float gravimetricInitialGrams = 0.0f;
float gravimetricFinalGrams = 0.0f;
float gravimetricMeasuredML = 0.0f;
float gravimetricErrorPct = 0.0f;
float gravimetricDensityGPerML = 1.0f;
float gravimetricTargetML = 0.0f;
String gravimetricResult = "idle";
unsigned long gravimetricStartMs = 0;

// Cached load cell / current diagnostics

float loadCellGrams = 0.0f;
bool loadCellReady = false;
unsigned long lastLoadCellSampleMs = 0;
const unsigned long LOAD_CELL_SAMPLE_MS = 250;
int currentRaw = 0;

// Config / history helpers

void loadPersistentConfig();
bool saveConfig();
void addAlarmHistory(const String& stateName, const String& message);
int addInfusionHistory(float targetML, float rateMLHr, bool gravimetric);
void finalizeInfusionHistory(const char* result);
void finalizeGravimetricTest(const char* result);
void sampleLoadCell();

// Mqtt topics

String topicBase = String("syringepump/") + DEVICE_ID;

String topicCmd = topicBase + "/cmd/#";
String topicStatus = topicBase + "/status";
String topicEvent = topicBase + "/event";
String topicTelFlow = topicBase + "/telemetry/flowrate";
String topicTelVol = topicBase + "/telemetry/volume_delivered";
String topicTelCur = topicBase + "/telemetry/motor_current";
String topicTelPos = topicBase + "/telemetry/position";
String topicTelLoad = topicBase + "/telemetry/loadcell";

// Forward declarations

void serviceConnections();

void mqttCallback(char* topic, byte* payload, unsigned int length);

void publishTelemetry();

void publishStatus();

void publishEvent(const String& msg);

void enterState(PumpState s, const String& reason = "");

bool startInfusion(float volumeML, float rateMLPerHr, String& err);

void homeAxis();

void driverEnable(bool on);

void sampleCurrent();

void captureCurrentBaseline();

bool checkOcclusion();

void completeInfusion();

void stopMotionImmediately();

void resetRunParameters();

bool pauseMotion();

bool resumeMotion();

void updateInfusionProgress();

void setupWebServer();
void serviceWebServer();
void handleRoot();
void handleLogin();
void handleLogout();
void handleApiStatus();
void handleApiCommand();
void handleApiSetup();
void handleApiSaveConfig();
void handleApiGravimetricStart();
void handleApiHistoryInfusions();
void handleApiHistoryAlarms();
void handleApiClearHistory();
void handleApiDiagnostics();
void handleNotFound();
bool requireWebAuth();
String makeSessionToken();
void sendApiResponse(bool ok, const String& message);
void executeLocalCommand(const String& commandPath, const String& payload = "");
String buildLocalStatusJson();
String buildSetupJson();
String buildDiagnosticsJson();

// Limit switches count as pressed only after staying LOW for LIMIT_DEBOUNCE_MS (filters motor noise).

const unsigned long LIMIT_DEBOUNCE_MS = 20;
unsigned long homeLowSinceMs = 0;
unsigned long maxLowSinceMs = 0;

// For use while the motor is running (non-blocking).
bool limitPressedDebounced(uint8_t pin, unsigned long& lowSinceMs) {
  if (digitalRead(pin) == LOW) {
    if (lowSinceMs == 0) {
      lowSinceMs = millis() | 1;
    }

    return (millis() - lowSinceMs) >= LIMIT_DEBOUNCE_MS;
  }

  lowSinceMs = 0;
  return false;
}

// For use while the motor is stopped (blocks for about 20 ms).
bool limitPressedStable(uint8_t pin) {
  for (int i = 0; i < 10; i++) {
    if (digitalRead(pin) != LOW) {
      return false;
    }

    delay(2);
  }

  return true;
}

// Driver enable

void driverEnable(bool on) {
  digitalWrite(PIN_ENABLE, on ? LOW : HIGH);

  Serial.print("A4988 DRIVER: ");

  Serial.println(on ? "ENABLED" : "DISABLED");
}

// Immediate stop (no deceleration). Used by STOP, MAX limit, occlusion and timeouts.

void stopMotionImmediately() {
  long stoppedPosition = stepper.currentPosition();

  stepper.setSpeed(0);

  stepper.setCurrentPosition(stoppedPosition);

  driverEnable(false);

  Serial.println();
  Serial.println("==========================================");

  Serial.println("MOTION STOPPED IMMEDIATELY");

  Serial.print("Stopped position: ");

  Serial.println(stoppedPosition);

  Serial.println("==========================================");
}

// Reset active run parameters

void resetRunParameters() {
  targetVolumeML = 0.0;

  flowRateMLPerHr = 0.0;

  totalStepsForRun = 0;

  stepsCompleted = 0;

  infusionTargetPosition = stepper.currentPosition();

  infusionStartPos = stepper.currentPosition();

  occlusionSinceMs = 0;

  graceStartMs = 0;

  pauseRequested = false;

  pauseStartMs = 0;
}

// Update infusion progress

void updateInfusionProgress() {
  long currentPosition = stepper.currentPosition();

  stepsCompleted = currentPosition - infusionStartPos;

  if (stepsCompleted < 0) {
    stepsCompleted = 0;
  }

  if (stepsCompleted > totalStepsForRun) {
    stepsCompleted = totalStepsForRun;
  }

  deliveredVolumeML = (float)stepsCompleted / SPM;
}

// Pause: decelerate, then disable the driver once the motor has stopped.

bool pauseMotion() {
  if (currentState != STATE_INFUSING) {
    publishEvent("pause rejected: pump not infusing");

    return false;
  }

  pauseRequested = true;

  pauseStartMs = millis();

  stepper.stop();

  Serial.println();
  Serial.println("==========================================");

  Serial.println("PAUSE REQUESTED");

  Serial.println("Controlled deceleration started");

  Serial.println("==========================================");

  return true;
}

// Resume motion

bool resumeMotion() {
  if (currentState != STATE_PAUSED) {
    publishEvent("resume rejected: pump not paused");

    return false;
  }

  if (REQUIRE_HOMING && !homed) {
    enterState(STATE_ALARM, "Cannot resume: pump is not homed");

    publishEvent("resume rejected: pump not homed");

    return false;
  }

  long currentPosition = stepper.currentPosition();

  if (currentPosition >= SYRINGE_EMPTY_POSITION_STEPS) {
    enterState(STATE_ALARM, "Cannot resume: syringe travel exhausted");

    publishEvent("resume rejected: syringe travel exhausted");

    return false;
  }

  updateInfusionProgress();

  long remainingSteps = totalStepsForRun - stepsCompleted;

  if (remainingSteps <= 0) {
    completeInfusion();

    return true;
  }

  float stepsPerSec = (flowRateMLPerHr * SPM) / 3600.0;

  if (stepsPerSec <= 0.0) {
    enterState(STATE_ALARM, "Cannot resume: invalid flow rate");

    publishEvent("resume rejected: invalid flow rate");

    return false;
  }

  if (stepsPerSec > MAX_STEPS_PER_SEC) {
    enterState(STATE_ALARM, "Cannot resume: step-rate cap exceeded");

    publishEvent("resume rejected: step-rate cap exceeded");

    return false;
  }

  driverEnable(true);

  delay(100);

  captureCurrentBaseline();

  stepper.setMaxSpeed(max(stepsPerSec, 1.0f));

  stepper.setAcceleration(INFUSION_ACCEL);

  stepper.moveTo(infusionTargetPosition);

  occlusionSinceMs = 0;

  graceStartMs = millis();

  occlusionLearnStartMs = 0;
  lastCurrentDebugMs = 0;
  occlusionBaselineLocked = false;
  homeLowSinceMs = 0;
  maxLowSinceMs = 0;

  pauseRequested = false;

  pauseStartMs = 0;

  enterState(STATE_INFUSING);

  publishEvent("resume accepted");

  Serial.println();
  Serial.println("==========================================");

  Serial.println("INFUSION RESUMED");

  Serial.print("Current position: ");

  Serial.println(currentPosition);

  Serial.print("Target position: ");

  Serial.println(infusionTargetPosition);

  Serial.print("Delivered volume: ");

  Serial.print(deliveredVolumeML, 3);

  Serial.println(" mL");

  Serial.print("Remaining steps: ");

  Serial.println(remainingSteps);

  Serial.println("==========================================");

  return true;
}

// Setup

void setup() {
  Serial.begin(115200);

  delay(500);

  Serial.println();

  Serial.println("==========================================");

  Serial.println(" SMART SYRINGE PUMP");

  Serial.println(" ESP32 + A4988 + NEMA 17");

  Serial.println(" FIRMWARE");

  Serial.println("==========================================");

  // Pins

  pinMode(PIN_STEP, OUTPUT);

  pinMode(PIN_DIR, OUTPUT);

  pinMode(PIN_ENABLE, OUTPUT);

  digitalWrite(PIN_STEP, LOW);

  digitalWrite(PIN_DIR, LOW);

  driverEnable(false);

  pinMode(PIN_LIMIT_HOME, INPUT_PULLUP);

  pinMode(PIN_LIMIT_MAX, INPUT_PULLUP);

  // Current sensor

  analogReadResolution(12);

  analogSetPinAttenuation(PIN_CURRENT_ADC, ADC_11db);

  pinMode(PIN_CURRENT_ADC, INPUT);

  currentFiltered = analogRead(PIN_CURRENT_ADC);

  // Accelstepper

  stepper.setMinPulseWidth(2);

  stepper.setAcceleration(INFUSION_ACCEL);

  // Calibration / persistent configuration

  loadPersistentConfig();

  SPM = calibratedStepsPerML;

  Serial.println();

  Serial.println("==========================================");

  Serial.println(" SYRINGE CALIBRATION");

  Serial.println("==========================================");

  Serial.print("Full position: ");

  Serial.print(SYRINGE_FULL_POSITION_STEPS);

  Serial.println(" steps");

  Serial.print("Empty position: ");

  Serial.print(SYRINGE_EMPTY_POSITION_STEPS);

  Serial.println(" steps");

  Serial.print("Maximum volume: ");

  Serial.print(SYRINGE_MAX_VOLUME_ML);

  Serial.println(" mL");

  Serial.print("CALIBRATED steps/mL: ");

  Serial.println(calibratedStepsPerML, 4);

  Serial.print("1 mL = ");

  Serial.print(volumeToSteps(1));

  Serial.println(" steps");

  Serial.print("5 mL = ");

  Serial.print(volumeToSteps(5));

  Serial.println(" steps");

  Serial.print("10 mL = ");

  Serial.print(volumeToSteps(10));

  Serial.println(" steps");

  Serial.print("30 mL = ");

  Serial.print(volumeToSteps(30));

  Serial.println(" steps");

  Serial.print("60 mL = ");

  Serial.print(volumeToSteps(60));

  Serial.println(" steps");

  Serial.print("Infusion acceleration: ");

  Serial.print(INFUSION_ACCEL);

  Serial.println(" steps/sec²");

  // Theoretical values

  Serial.println();

  Serial.println("THEORETICAL MECHANICAL VALUES:");

  Serial.print("Lead screw pitch: ");

  Serial.print(LEAD_SCREW_PITCH_MM);

  Serial.println(" mm/rev");

  Serial.print("Motor steps/rev: ");

  Serial.println(STEPS_PER_REV);

  Serial.print("Physical microsteps: ");

  Serial.println(MICROSTEPS);

  Serial.print("Theoretical steps/mm: ");

  Serial.println(theoreticalStepsPerMM());

  Serial.print("Theoretical steps/mL: ");

  Serial.println(theoreticalStepsPerML());

  Serial.println();

  // Hx711

  scale.begin(PIN_HX711_DOUT, PIN_HX711_SCK);

  if (scale.wait_ready_timeout(2000)) {
    scale.set_scale(hx711ScaleFactor);
    scale.tare();
    loadCellReady = true;

    Serial.println("HX711: OK");

  } else {
    Serial.println("WARNING: HX711 not responding");
  }

  // Mqtt / tls

  wifiClient.setCACert(HIVEMQ_ROOT_CA);

  wifiClient.setHandshakeTimeout(5);

  mqtt.setServer(MQTT_HOST, MQTT_PORT);

  mqtt.setSocketTimeout(5);

  mqtt.setKeepAlive(15);

  mqtt.setCallback(mqttCallback);

  // Wifi

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  lastWifiTryMs = millis();

  setupWebServer();

  Serial.println();

  Serial.println("Connecting to WiFi...");

  // Initial state

  enterState(STATE_IDLE);

  // Startup home detection

  if (limitPressedStable(PIN_LIMIT_HOME)) {
    stepper.setCurrentPosition(SYRINGE_FULL_POSITION_STEPS);

    homed = true;

    deliveredVolumeML = 0.0;

    targetVolumeML = 0.0;

    stepsCompleted = 0;

    totalStepsForRun = 0;

    infusionTargetPosition = 0;

    Serial.println();

    Serial.println("STARTUP HOME SWITCH: ACTIVE");

    Serial.println("Startup position established at 0 steps");

  } else {
    Serial.println();

    Serial.println("STARTUP HOME SWITCH: NOT ACTIVE");

    Serial.println("HOME COMMAND REQUIRED BEFORE START");
  }
}

// Main loop

void loop() {
  serviceWebServer();

  serviceConnections();

  sampleCurrent();
  sampleLoadCell();

  switch (currentState) {
      // Infusing

    case STATE_INFUSING: {
      stepper.run();

      updateInfusionProgress();

      // Position safety

      if (stepper.currentPosition() > SYRINGE_EMPTY_POSITION_STEPS) {
        stopMotionImmediately();

        enterState(STATE_ALARM, "Calibrated maximum syringe position exceeded");

        break;
      }

      // Max switch

      if (limitPressedDebounced(PIN_LIMIT_MAX, maxLowSinceMs)) {
        stopMotionImmediately();

        enterState(STATE_ALARM, "MAX limit switch hit during infusion");

        break;
      }

      // Pause deceleration

      if (pauseRequested) {
        if (stepper.distanceToGo() == 0 && stepper.speed() == 0) {
          updateInfusionProgress();

          driverEnable(false);

          pauseRequested = false;

          pauseStartMs = 0;

          occlusionSinceMs = 0;

          graceStartMs = 0;

          enterState(STATE_PAUSED);

          publishEvent("pause accepted");

          Serial.println();

          Serial.println("==========================================");

          Serial.println("INFUSION PAUSED");

          Serial.print("Paused position: ");

          Serial.println(stepper.currentPosition());

          Serial.print("Delivered volume: ");

          Serial.print(deliveredVolumeML, 3);

          Serial.println(" mL");

          Serial.print("Remaining volume: ");

          Serial.print(max(targetVolumeML - deliveredVolumeML, 0.0f), 3);

          Serial.println(" mL");

          Serial.println("==========================================");

          break;
        }

        if (millis() - pauseStartMs > PAUSE_TIMEOUT_MS) {
          stopMotionImmediately();

          pauseRequested = false;

          pauseStartMs = 0;

          updateInfusionProgress();

          enterState(STATE_PAUSED, "Pause deceleration timeout");

          publishEvent("pause completed by safety timeout");

          break;
        }
      }

      // Reached the target position.

      if (!pauseRequested && stepper.distanceToGo() == 0) {
        completeInfusion();

        break;
      }

      // Occlusion

      if (OCCLUSION_DETECTION_ENABLED && !pauseRequested && checkOcclusion()) {
        stopMotionImmediately();

        enterState(STATE_OCCLUDED,
                   String("Motor current above learned running-current threshold (current ") +
                       String(currentFiltered, 0) + ", baseline " + String(currentBaseline, 0) +
                       ", rise " + String(currentFiltered - currentBaseline, 0) +
                       " counts, limit " + String(OCCLUSION_DELTA_COUNTS) + ")");

        break;
      }

      // Offline

      if (!pauseRequested && OFFLINE_PAUSE_MS > 0 && offlineSinceMs != 0 &&
          millis() - offlineSinceMs >= OFFLINE_PAUSE_MS) {
        pauseMotion();
      }

      break;
    }

      // Homing

    case STATE_HOMING: {
      stepper.run();

      // Home switch

      if (limitPressedDebounced(PIN_LIMIT_HOME, homeLowSinceMs)) {
        stepper.stop();

        stepper.setCurrentPosition(SYRINGE_FULL_POSITION_STEPS);

        homed = true;

        driverEnable(false);

        deliveredVolumeML = 0.0;

        targetVolumeML = 0.0;

        stepsCompleted = 0;

        totalStepsForRun = 0;

        flowRateMLPerHr = 0.0;

        infusionTargetPosition = 0;

        publishEvent("homing complete: position 0");

        enterState(STATE_IDLE);

        Serial.println();

        Serial.println("==========================================");

        Serial.println("HOME POSITION = 0 STEPS");

        Serial.println("HOMING COMPLETE");

        Serial.println("==========================================");
      }

      // Homing travel exhausted

      else if (stepper.distanceToGo() == 0) {
        stopMotionImmediately();

        homed = false;

        enterState(STATE_ALARM, "Homing travel exhausted: HOME switch not reached");
      }

      // Homing timeout

      else if (millis() - homingStartMs > HOMING_TIMEOUT_MS) {
        stopMotionImmediately();

        homed = false;

        enterState(STATE_ALARM, "Homing timeout: HOME switch not reached");
      }

      break;
    }

    default:

      break;
  }

  // Telemetry

  unsigned long now = millis();

  if (now - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = now;

    publishTelemetry();
  }
}

// Dispensing complete

void completeInfusion() {
  Serial.println();

  Serial.println("==========================================");

  Serial.println("       DISPENSING COMPLETE");

  Serial.println("==========================================");

  stepper.setCurrentPosition(infusionTargetPosition);

  stepper.setSpeed(0);

  driverEnable(false);

  deliveredVolumeML = targetVolumeML;

  stepsCompleted = totalStepsForRun;

  finalizeGravimetricTest("complete");
  finalizeInfusionHistory("complete");

  Serial.print("Delivered volume: ");

  Serial.print(deliveredVolumeML, 3);

  Serial.println(" mL");

  Serial.print("Steps completed: ");

  Serial.println(stepsCompleted);

  Serial.print("Final position: ");

  Serial.println(stepper.currentPosition());

  enterState(STATE_COMPLETE);

  publishEvent("dispensing complete");

  delay(100);

  currentState = STATE_IDLE;

  alarmMessage = "";

  flowRateMLPerHr = 0.0;

  targetVolumeML = 0.0;

  totalStepsForRun = 0;

  stepsCompleted = 0;

  infusionStartPos = stepper.currentPosition();

  infusionTargetPosition = stepper.currentPosition();

  Serial.println("Pump state: IDLE");

  publishStatus();

  publishEvent("pump ready: idle");
}

// State transitions

void enterState(PumpState s, const String& reason) {
  currentState = s;

  alarmMessage = reason;

  if (s == STATE_ALARM || s == STATE_OCCLUDED) {
    driverEnable(false);

    String stateName = (s == STATE_OCCLUDED) ? "occluded" : "alarm";

    addAlarmHistory(stateName, reason.length() ? reason : "unspecified alarm");

    finalizeGravimetricTest((s == STATE_OCCLUDED) ? "occluded" : "alarm");

    finalizeInfusionHistory((s == STATE_OCCLUDED) ? "occluded" : "alarm");

    Serial.println();

    Serial.println("ALARM: " + reason);
  }

  publishStatus();
}

// Start infusion

bool startInfusion(float volumeML, float rateMLPerHr, String& err) {
  Serial.println();

  Serial.println("========== START REQUEST ==========");

  Serial.print("Volume: ");

  Serial.print(volumeML, 3);

  Serial.println(" mL");

  Serial.print("Rate: ");

  Serial.print(rateMLPerHr, 3);

  Serial.println(" mL/hr");

  Serial.print("Current state: ");

  Serial.println((int)currentState);

  Serial.print("Homed: ");

  Serial.println(homed ? "YES" : "NO");

  if (currentState != STATE_IDLE) {
    err = "pump not idle";

    Serial.println("START REJECTED: pump not idle");

    return false;
  }

  if (REQUIRE_HOMING && !homed) {
    err = "not homed";

    Serial.println("START REJECTED: not homed");

    return false;
  }

  if (volumeML <= 0 || volumeML > MAX_VOLUME_ML) {
    err = "volume out of range";

    Serial.println("START REJECTED: volume out of range");

    return false;
  }

  long requestedSteps = volumeToSteps(volumeML);

  long currentPosition = stepper.currentPosition();

  long targetPosition = currentPosition + requestedSteps;

  Serial.print("Current position: ");

  Serial.println(currentPosition);

  Serial.print("Required steps: ");

  Serial.println(requestedSteps);

  Serial.print("Target position: ");

  Serial.println(targetPosition);

  if (targetPosition > SYRINGE_EMPTY_POSITION_STEPS) {
    err = "requested volume exceeds remaining syringe capacity";

    Serial.println("START REJECTED: insufficient syringe travel");

    return false;
  }

  if (rateMLPerHr <= 0 || rateMLPerHr > MAX_RATE_ML_HR) {
    err = "rate out of range";

    Serial.println("START REJECTED: rate out of range");

    return false;
  }

  float stepsPerSec = (rateMLPerHr * SPM) / 3600.0;

  Serial.print("Steps/sec: ");

  Serial.println(stepsPerSec, 3);

  if (stepsPerSec > MAX_STEPS_PER_SEC) {
    err = "rate exceeds step-rate cap";

    Serial.println("START REJECTED: step-rate cap");

    return false;
  }

  // Save run parameters

  targetVolumeML = volumeML;

  flowRateMLPerHr = rateMLPerHr;

  deliveredVolumeML = 0.0;

  totalStepsForRun = requestedSteps;

  stepsCompleted = 0;

  infusionStartPos = currentPosition;

  infusionTargetPosition = targetPosition;

  activeInfusionHistory = addInfusionHistory(volumeML, rateMLPerHr, gravimetricActive);

  pauseRequested = false;

  pauseStartMs = 0;

  // Enable driver

  driverEnable(true);

  delay(150);

  // Current baseline

  captureCurrentBaseline();

  // True acceleration profile

  stepper.setMaxSpeed(max(stepsPerSec, 1.0f));

  stepper.setAcceleration(INFUSION_ACCEL);

  stepper.moveTo(infusionTargetPosition);

  // Occlusion timers

  occlusionSinceMs = 0;

  graceStartMs = millis();

  occlusionLearnStartMs = 0;
  lastCurrentDebugMs = 0;
  occlusionBaselineLocked = false;
  homeLowSinceMs = 0;
  maxLowSinceMs = 0;

  // State

  enterState(STATE_INFUSING);

  Serial.println();

  Serial.println("==========================================");

  Serial.println("DISPENSING STARTED");

  Serial.println("==========================================");

  Serial.print("Target volume: ");

  Serial.print(volumeML, 3);

  Serial.println(" mL");

  Serial.print("Required steps: ");

  Serial.println(requestedSteps);

  Serial.print("Target position: ");

  Serial.println(targetPosition);

  Serial.print("Maximum speed: ");

  Serial.print(stepsPerSec, 3);

  Serial.println(" steps/sec");

  Serial.print("Acceleration: ");

  Serial.print(INFUSION_ACCEL);

  Serial.println(" steps/sec²");

  publishEvent("dispensing started");

  return true;
}

// Homing

void homeAxis() {
  Serial.println();

  Serial.println("========== HOME REQUEST ==========");

  Serial.print("Current position: ");

  Serial.println(stepper.currentPosition());

  if (limitPressedStable(PIN_LIMIT_HOME)) {
    Serial.println("HOME SWITCH ALREADY ACTIVE");

    stepper.setCurrentPosition(SYRINGE_FULL_POSITION_STEPS);

    homed = true;

    driverEnable(false);

    deliveredVolumeML = 0.0;

    targetVolumeML = 0.0;

    stepsCompleted = 0;

    totalStepsForRun = 0;

    flowRateMLPerHr = 0.0;

    infusionTargetPosition = 0;

    enterState(STATE_IDLE);

    publishEvent("home already active: position set to 0");

    return;
  }

  stepper.setSpeed(0);

  stepper.setCurrentPosition(stepper.currentPosition());

  driverEnable(true);

  stepper.setMaxSpeed(HOMING_SPEED_STEPS_S);

  stepper.setAcceleration(HOMING_ACCEL);

  long homingDistance = lroundf(SYRINGE_EMPTY_POSITION_STEPS * HOMING_MAX_TRAVEL_FACTOR);

  stepper.move(HOME_DIRECTION * homingDistance);

  homingStartMs = millis();

  homeLowSinceMs = 0;
  maxLowSinceMs = 0;

  homed = false;

  enterState(STATE_HOMING);

  Serial.println("HOMING STARTED");

  Serial.print("Direction: ");

  Serial.println(HOME_DIRECTION < 0 ? "NEGATIVE" : "POSITIVE");

  Serial.print("HOME switch (GPIO 32) level: ");

  Serial.println(digitalRead(PIN_LIMIT_HOME) == LOW ? "LOW (pressed)" : "HIGH (released)");

  Serial.print("MAX switch (GPIO 33) level: ");

  Serial.println(digitalRead(PIN_LIMIT_MAX) == LOW ? "LOW (pressed)" : "HIGH (released)");

  Serial.print("Target travel: ");

  Serial.println(homingDistance);

  publishEvent(String("homing started: direction ") +
               (HOME_DIRECTION < 0 ? "negative" : "positive") + ", HOME switch " +
               (digitalRead(PIN_LIMIT_HOME) == LOW ? "pressed" : "released") + ", MAX switch " +
               (digitalRead(PIN_LIMIT_MAX) == LOW ? "pressed" : "released") + ", travel " +
               String(homingDistance) + " steps");
}

// Current

void sampleCurrent() {
  unsigned long now = millis();

  if (now - lastCurrentSampleMs < CURRENT_SAMPLE_MS) {
    return;
  }

  lastCurrentSampleMs = now;

  int raw = analogRead(PIN_CURRENT_ADC);

  currentRaw = raw;

  currentFiltered += CURRENT_EMA_ALPHA * (raw - currentFiltered);

  if (currentState == STATE_INFUSING && now - lastCurrentDebugMs >= CURRENT_DEBUG_INTERVAL_MS) {
    lastCurrentDebugMs = now;

    Serial.print("CURRENT RAW: ");
    Serial.print(raw);

    Serial.print(" | FILTERED: ");
    Serial.print(currentFiltered, 1);

    Serial.print(" | BASELINE: ");
    Serial.print(currentBaseline, 1);

    Serial.print(" | DELTA: ");
    Serial.print(currentFiltered - currentBaseline, 1);

    Serial.print(" | LEARNED: ");
    Serial.println(occlusionBaselineLocked ? "YES" : "NO");
  }
}

// Current baseline

void captureCurrentBaseline() {
  long sum = 0;

  const int N = 32;

  for (int i = 0; i < N; i++) {
    sum += analogRead(PIN_CURRENT_ADC);

    delay(2);
  }

  currentBaseline = (float)sum / N;

  currentFiltered = currentBaseline;

  Serial.print("CURRENT STATIONARY BASELINE: ");

  Serial.println(currentBaseline, 1);
}

// Occlusion

bool checkOcclusion() {
  unsigned long now = millis();

  if (now - graceStartMs < OCCLUSION_GRACE_MS) {
    occlusionSinceMs = 0;
    return false;
  }

  if (!occlusionBaselineLocked) {
    if (occlusionLearnStartMs == 0) {
      occlusionLearnStartMs = now;
    }

    currentBaseline += OCCLUSION_BASELINE_ALPHA * (currentFiltered - currentBaseline);

    if (now - occlusionLearnStartMs >= OCCLUSION_LEARN_MS) {
      occlusionBaselineLocked = true;

      Serial.print("RUNNING CURRENT BASELINE LOCKED: ");

      Serial.println(currentBaseline, 1);
    }

    occlusionSinceMs = 0;
    return false;
  }

  float delta = currentFiltered - currentBaseline;

  if (delta > OCCLUSION_DELTA_COUNTS) {
    if (occlusionSinceMs == 0) {
      occlusionSinceMs = now;
    }

    if (now - occlusionSinceMs >= OCCLUSION_HOLD_MS) {
      Serial.println("OCCLUSION CONFIRMED");

      Serial.print("Current filtered: ");

      Serial.println(currentFiltered, 1);

      Serial.print("Running baseline: ");

      Serial.println(currentBaseline, 1);

      Serial.print("Current delta: ");

      Serial.println(delta, 1);

      return true;
    }

  } else {
    occlusionSinceMs = 0;

    currentBaseline += OCCLUSION_BASELINE_ALPHA * (currentFiltered - currentBaseline);
  }

  return false;
}

// Wifi / mqtt connection

void serviceConnections() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (offlineSinceMs == 0) {
      offlineSinceMs = now;
    }

    if (now - lastWifiTryMs >= WIFI_RETRY_MS) {
      lastWifiTryMs = now;

      Serial.println("WiFi disconnected - reconnecting...");

      WiFi.disconnect();

      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    return;
  }

  // Start mDNS as soon as Wi-Fi is available so every device on the
  // local network can open the pump using: http://smartsyringe.local
  if (!mdnsStarted) {
    if (MDNS.begin(MDNS_HOSTNAME)) {
      MDNS.addService("http", "tcp", 80);
      mdnsStarted = true;
      Serial.println("mDNS: http://smartsyringe.local/");
    } else {
      Serial.println("mDNS: startup failed; will retry");
    }
  }

  if (!mqtt.connected()) {
    if (offlineSinceMs == 0) {
      offlineSinceMs = now;
    }

    bool motionActive = currentState == STATE_HOMING || currentState == STATE_INFUSING;

    if (!motionActive && now - lastMqttTryMs >= MQTT_RETRY_MS) {
      lastMqttTryMs = now;

      Serial.print("Connecting to MQTT...");

      if (mqtt.connect(DEVICE_ID, MQTT_USERNAME, MQTT_PASSWORD, topicStatus.c_str(), 1, true,
                       "{\"state\":\"offline\"}")) {
        Serial.println("connected");

        bool subscribed = mqtt.subscribe(topicCmd.c_str());

        Serial.print("MQTT subscribe: ");

        Serial.println(subscribed ? "SUCCESS" : "FAILED");

        Serial.print("Subscribed topic: ");

        Serial.println(topicCmd);

        offlineSinceMs = 0;

        publishStatus();

        publishEvent("MQTT connected and command topic subscribed");

      } else {
        Serial.print("failed, rc=");

        Serial.println(mqtt.state());
      }
    }

    return;
  }

  offlineSinceMs = 0;

  mqtt.loop();
}

// Mqtt callback

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println();

  Serial.println("==========================================");

  Serial.println("       MQTT MESSAGE RECEIVED");

  Serial.println("==========================================");

  Serial.print("Topic: ");

  Serial.println(t);

  Serial.print("Payload: ");

  Serial.println(message);

  Serial.print("Length: ");

  Serial.println(length);

  // Ping

  if (t.endsWith("/cmd/ping")) {
    Serial.println("PING COMMAND RECEIVED");

    publishEvent("pong");

    return;
  }

  // Start

  if (t.endsWith("/cmd/start")) {
    StaticJsonDocument<256> doc;

    DeserializationError jsonError = deserializeJson(doc, payload, length);

    if (jsonError) {
      Serial.print("JSON ERROR: ");

      Serial.println(jsonError.c_str());

      publishEvent("start rejected: bad JSON");

      return;
    }

    float vol = doc["volume_ml"] | 0.0;

    float rate = doc["rate_ml_hr"] | 0.0;

    Serial.println("START COMMAND RECEIVED");

    Serial.print("Parsed volume: ");

    Serial.print(vol, 3);

    Serial.println(" mL");

    Serial.print("Parsed rate: ");

    Serial.print(rate, 3);

    Serial.println(" mL/hr");

    String why;

    if (startInfusion(vol, rate, why)) {
      publishEvent("start accepted");

    } else {
      Serial.print("START REJECTED: ");

      Serial.println(why);

      publishEvent("start rejected: " + why);
    }

    return;
  }

  // Stop

  if (t.endsWith("/cmd/stop")) {
    Serial.println("STOP COMMAND RECEIVED");

    if (currentState == STATE_INFUSING || currentState == STATE_PAUSED ||
        currentState == STATE_HOMING || currentState == STATE_OCCLUDED) {
      long stoppedPosition = stepper.currentPosition();

      stopMotionImmediately();

      // Occlusion clear

      if (currentState == STATE_OCCLUDED) {
        homed = false;

        resetRunParameters();

        enterState(STATE_IDLE, "occlusion cleared: re-home required");

        publishEvent("stop accepted: occlusion cleared, re-home required");

        Serial.println();
        Serial.println("==========================================");
        Serial.println("OCCLUSION CLEARED");
        Serial.println("Re-home required before next START");
        Serial.println("==========================================");

        return;
      }

      // Homing abort

      if (currentState == STATE_HOMING) {
        homed = false;

        resetRunParameters();

        enterState(STATE_IDLE, "motion stopped: re-home required");

        publishEvent("stop accepted: homing aborted, re-home required");

        return;
      }

      // Infusion stop

      if (stoppedPosition >= infusionStartPos) {
        stepsCompleted = stoppedPosition - infusionStartPos;

        if (stepsCompleted > totalStepsForRun) {
          stepsCompleted = totalStepsForRun;
        }

        deliveredVolumeML = (float)stepsCompleted / SPM;

      } else {
        deliveredVolumeML = positionToVolume(stoppedPosition);
      }

      finalizeGravimetricTest("stopped");
      finalizeInfusionHistory("stopped");

      targetVolumeML = 0.0;

      flowRateMLPerHr = 0.0;

      totalStepsForRun = 0;

      stepsCompleted = 0;

      infusionStartPos = stoppedPosition;

      infusionTargetPosition = stoppedPosition;

      occlusionSinceMs = 0;

      graceStartMs = 0;

      pauseRequested = false;

      pauseStartMs = 0;

      enterState(STATE_IDLE);

      publishEvent("stop accepted");

      Serial.println();

      Serial.println("==========================================");

      Serial.println("INFUSION STOPPED");

      Serial.print("Stopped position: ");

      Serial.println(stoppedPosition);

      Serial.print("Delivered before STOP: ");

      Serial.print(deliveredVolumeML, 3);

      Serial.println(" mL");

      Serial.println("Pump state: IDLE");

      Serial.println("==========================================");

    } else {
      stepper.setSpeed(0);

      stepper.setCurrentPosition(stepper.currentPosition());

      driverEnable(false);

      publishEvent("stop accepted: pump already stopped");
    }

    return;
  }

  // Pause

  if (t.endsWith("/cmd/pause")) {
    Serial.println("PAUSE COMMAND RECEIVED");

    pauseMotion();

    return;
  }

  // Resume

  if (t.endsWith("/cmd/resume")) {
    Serial.println("RESUME COMMAND RECEIVED");

    resumeMotion();

    return;
  }

  // Home

  if (t.endsWith("/cmd/home")) {
    Serial.println("HOME COMMAND RECEIVED");

    if (currentState == STATE_INFUSING || currentState == STATE_HOMING ||
        currentState == STATE_PAUSED) {
      publishEvent("home rejected: pump motion/run active");

    } else {
      homeAxis();

      publishEvent("home command accepted");
    }

    return;
  }

  // Tare

  if (t.endsWith("/cmd/tare")) {
    Serial.println("TARE COMMAND RECEIVED");

    if (scale.is_ready()) {
      scale.tare();
      loadCellGrams = 0.0f;
      loadCellReady = true;

      publishEvent("load cell tared");

    } else {
      publishEvent("tare rejected: HX711 not ready");
    }

    return;
  }

  // Unknown command

  Serial.println("UNKNOWN MQTT COMMAND");

  publishEvent("unknown command: " + t);
}

// Local esp32 web dashboard

#include "dashboard_html.h"
#include "login_page.h"
// Graph downloads are handled client-side from live /api/status telemetry.

// Local login session
// The dashboard uses an explicit login page instead of the browser's
// HTTP Basic-Auth dialog. Authentication is still intended for trusted
// LAN/VPN use only because this firmware serves plain HTTP.
String webSessionToken = "";

String makeSessionToken() {
  uint32_t a = esp_random();
  uint32_t b = esp_random();
  char token[33];
  snprintf(token, sizeof(token), "%08lX%08lX%08lX%08lX", (unsigned long)a, (unsigned long)b,
           (unsigned long)esp_random(), (unsigned long)esp_random());
  return String(token);
}

bool requireWebAuth() {
  if (webSessionToken.length() == 0) {
    webSessionToken = makeSessionToken();
  }

  if (!webServer.hasHeader("Cookie")) {
    if (webServer.uri().startsWith("/api/")) {
      webServer.send(401, "application/json", "{\"ok\":false,\"message\":\"login required\"}");
    }
    return false;
  }

  String cookie = webServer.header("Cookie");
  String expected = "SP_SESSION=" + webSessionToken;
  if (cookie.indexOf(expected) >= 0) {
    return true;
  }

  if (webServer.uri().startsWith("/api/")) {
    webServer.send(401, "application/json", "{\"ok\":false,\"message\":\"login required\"}");
  }
  return false;
}

void handleLogin() {
  StaticJsonDocument<256> doc;
  DeserializationError e = deserializeJson(doc, webServer.arg("plain"));
  if (e) {
    webServer.send(400, "application/json", "{\"ok\":false,\"message\":\"invalid login request\"}");
    return;
  }

  const char* user = doc["username"] | "";
  const char* pass = doc["password"] | "";

  if (String(user) != WEB_AUTH_USER || String(pass) != WEB_AUTH_PASSWORD) {
    delay(150);
    webServer.send(401, "application/json",
                   "{\"ok\":false,\"message\":\"invalid username or password\"}");
    return;
  }

  webSessionToken = makeSessionToken();
  webServer.sendHeader("Set-Cookie",
                       "SP_SESSION=" + webSessionToken + "; Path=/; HttpOnly; SameSite=Strict");
  webServer.send(200, "application/json", "{\"ok\":true,\"message\":\"login successful\"}");
}

void handleLogout() {
  webSessionToken = makeSessionToken();
  webServer.sendHeader("Set-Cookie",
                       "SP_SESSION=deleted; Path=/; Max-Age=0; HttpOnly; SameSite=Strict");
  webServer.send(200, "application/json", "{\"ok\":true,\"message\":\"logged out\"}");
}

void setupWebServer() {
  const char* headerKeys[] = {"Cookie"};
  webServer.collectHeaders(headerKeys, 1);

  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/login", HTTP_GET, handleRoot);
  webServer.on("/api/login", HTTP_POST, handleLogin);
  webServer.on("/api/logout", HTTP_POST, handleLogout);
  webServer.on("/api/status", HTTP_GET, handleApiStatus);
  webServer.on("/api/setup", HTTP_GET, handleApiSetup);
  webServer.on("/api/diagnostics", HTTP_GET, handleApiDiagnostics);

  webServer.on("/api/start", HTTP_POST, handleApiCommand);
  webServer.on("/api/stop", HTTP_POST, handleApiCommand);
  webServer.on("/api/pause", HTTP_POST, handleApiCommand);
  webServer.on("/api/resume", HTTP_POST, handleApiCommand);
  webServer.on("/api/home", HTTP_POST, handleApiCommand);
  webServer.on("/api/tare", HTTP_POST, handleApiCommand);
  webServer.on("/api/ping", HTTP_POST, handleApiCommand);

  webServer.on("/api/config/save", HTTP_POST, handleApiSaveConfig);
  webServer.on("/api/gravimetric/start", HTTP_POST, handleApiGravimetricStart);
  webServer.on("/api/history/infusions", HTTP_GET, handleApiHistoryInfusions);
  webServer.on("/api/history/alarms", HTTP_GET, handleApiHistoryAlarms);
  webServer.on("/api/history/clear", HTTP_POST, handleApiClearHistory);

  webServer.onNotFound(handleNotFound);
  webServer.begin();
  webServerStarted = true;
  Serial.println("LOCAL WEB DASHBOARD: http://smartsyringe.local/");
  Serial.println("Fallback URL: http://<ESP32-IP>/");
}

void serviceWebServer() {
  if (webServerStarted) webServer.handleClient();
}

void handleRoot() {
  if (!requireWebAuth()) {
    webServer.send_P(200, "text/html; charset=utf-8", LOGIN_HTML);
    return;
  }
  webServer.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML);
}

void sendApiResponse(bool ok, const String& message) {
  StaticJsonDocument<256> doc;
  doc["ok"] = ok;
  doc["message"] = message;
  doc["state"] = (int)currentState;

  String out;
  serializeJson(doc, out);

  webServer.send(ok ? 200 : 400, "application/json", out);
}

void executeLocalCommand(const String& commandPath, const String& payload) {
  char topicBuf[128];
  String topic = topicBase + commandPath;
  topic.toCharArray(topicBuf, sizeof(topicBuf));

  mqttCallback(topicBuf, (byte*)payload.c_str(), payload.length());
}

String buildLocalStatusJson() {
  DynamicJsonDocument doc(4096);

  const char* stateText = "unknown";

  switch (currentState) {
    case STATE_IDLE:
      stateText = "idle";
      break;
    case STATE_HOMING:
      stateText = "homing";
      break;
    case STATE_PRIMING:
      stateText = "priming";
      break;
    case STATE_INFUSING:
      stateText = "infusing";
      break;
    case STATE_PAUSED:
      stateText = "paused";
      break;
    case STATE_OCCLUDED:
      stateText = "occluded";
      break;
    case STATE_COMPLETE:
      stateText = "complete";
      break;
    case STATE_ALARM:
      stateText = "alarm";
      break;
  }

  doc["device_id"] = DEVICE_ID;
  doc["state"] = stateText;
  doc["homed"] = homed;

  doc["target_volume_ml"] = targetVolumeML;
  doc["delivered_volume_ml"] = deliveredVolumeML;
  doc["flow_rate_ml_hr"] = (currentState == STATE_INFUSING) ? flowRateMLPerHr : 0.0;

  doc["position"] = stepper.currentPosition();
  doc["target_position"] = infusionTargetPosition;
  doc["distance_to_go"] = stepper.distanceToGo();
  doc["stepper_speed"] = stepper.speed();

  doc["steps_completed"] = stepsCompleted;
  doc["total_steps"] = totalStepsForRun;
  doc["max_steps"] = SYRINGE_EMPTY_POSITION_STEPS;

  doc["calibrated_steps_per_ml"] = calibratedStepsPerML;
  doc["acceleration"] = INFUSION_ACCEL;

  doc["current_raw"] = currentRaw;
  doc["current_adc"] = currentFiltered;
  doc["current_baseline"] = currentBaseline;
  doc["current_delta"] = currentFiltered - currentBaseline;
  doc["occlusion_baseline_locked"] = occlusionBaselineLocked;
  doc["occlusion_delta_counts"] = OCCLUSION_DELTA_COUNTS;
  doc["occlusion_hold_ms"] = OCCLUSION_HOLD_MS;
  doc["occlusion_grace_ms"] = OCCLUSION_GRACE_MS;

  doc["home_limit"] = (digitalRead(PIN_LIMIT_HOME) == LOW);
  doc["max_limit"] = (digitalRead(PIN_LIMIT_MAX) == LOW);

  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["mqtt_connected"] = mqtt.connected();
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;

  doc["load_cell_ready"] = loadCellReady;
  doc["load_cell_grams"] = loadCellGrams;
  doc["hx711_scale_factor"] = hx711ScaleFactor;

  doc["fluid_density_g_ml"] = fluidDensityGPerML;

  doc["gravimetric_active"] = gravimetricActive;
  doc["gravimetric_initial_g"] = gravimetricInitialGrams;
  doc["gravimetric_final_g"] = gravimetricFinalGrams;
  doc["gravimetric_measured_ml"] = gravimetricMeasuredML;
  doc["gravimetric_error_pct"] = gravimetricErrorPct;
  doc["gravimetric_density"] = gravimetricDensityGPerML;
  doc["gravimetric_target_ml"] = gravimetricTargetML;
  doc["gravimetric_pump_ml"] = deliveredVolumeML;
  doc["gravimetric_result"] = gravimetricResult;

  doc["free_heap"] = ESP.getFreeHeap();
  doc["min_free_heap"] = ESP.getMinFreeHeap();
  doc["cpu_mhz"] = ESP.getCpuFreqMHz();
  doc["uptime_s"] = millis() / 1000.0f;

  doc["last_event"] = lastWebEvent;
  doc["last_event_age_ms"] = (lastWebEventMs == 0) ? 0 : (millis() - lastWebEventMs);

  doc["infusion_history_count"] = infusionHistoryCount;
  doc["alarm_history_count"] = alarmHistoryCount;

  if (alarmMessage.length() > 0) {
    doc["message"] = alarmMessage;
  }

  String out;
  serializeJson(doc, out);
  return out;
}

String buildSetupJson() {
  DynamicJsonDocument doc(1024);

  doc["steps_per_ml"] = calibratedStepsPerML;
  doc["default_steps_per_ml"] = CALIBRATED_STEPS_PER_ML_DEFAULT;
  doc["hx711_scale"] = hx711ScaleFactor;
  doc["density_g_ml"] = fluidDensityGPerML;
  doc["acceleration"] = INFUSION_ACCEL;
  doc["max_volume_ml"] = MAX_VOLUME_ML;
  doc["max_rate_ml_hr"] = MAX_RATE_ML_HR;
  doc["full_position_steps"] = SYRINGE_FULL_POSITION_STEPS;
  doc["empty_position_steps"] = SYRINGE_EMPTY_POSITION_STEPS;
  doc["position"] = stepper.currentPosition();
  doc["homed"] = homed;
  doc["state"] = (int)currentState;

  String out;
  serializeJson(doc, out);
  return out;
}

String buildDiagnosticsJson() {
  DynamicJsonDocument doc(2048);

  doc["device_id"] = DEVICE_ID;
  doc["state"] = (int)currentState;
  doc["position"] = stepper.currentPosition();
  doc["target_position"] = infusionTargetPosition;
  doc["distance_to_go"] = stepper.distanceToGo();
  doc["stepper_speed"] = stepper.speed();
  doc["current_raw"] = currentRaw;
  doc["current_filtered"] = currentFiltered;
  doc["current_baseline"] = currentBaseline;
  doc["current_delta"] = currentFiltered - currentBaseline;
  doc["occlusion_locked"] = occlusionBaselineLocked;
  doc["home_limit"] = (digitalRead(PIN_LIMIT_HOME) == LOW);
  doc["max_limit"] = (digitalRead(PIN_LIMIT_MAX) == LOW);
  doc["hx711_ready"] = loadCellReady;
  doc["load_cell_grams"] = loadCellGrams;
  doc["hx711_scale"] = hx711ScaleFactor;
  doc["wifi"] = WiFi.status() == WL_CONNECTED;
  doc["mqtt"] = mqtt.connected();
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["min_free_heap"] = ESP.getMinFreeHeap();
  doc["uptime_ms"] = millis();
  doc["cpu_mhz"] = ESP.getCpuFreqMHz();

  String out;
  serializeJson(doc, out);
  return out;
}

void handleApiStatus() {
  if (!requireWebAuth()) return;
  webServer.send(200, "application/json", buildLocalStatusJson());
}

void handleApiSetup() {
  if (!requireWebAuth()) return;
  webServer.send(200, "application/json", buildSetupJson());
}

void handleApiDiagnostics() {
  if (!requireWebAuth()) return;
  webServer.send(200, "application/json", buildDiagnosticsJson());
}

void handleApiSaveConfig() {
  if (!requireWebAuth()) return;
  if (currentState != STATE_IDLE || !homed ||
      stepper.currentPosition() != SYRINGE_FULL_POSITION_STEPS) {
    sendApiResponse(false,
                    "configuration rejected: pump must be IDLE, homed, and at HOME position");
    return;
  }

  StaticJsonDocument<384> doc;
  DeserializationError e = deserializeJson(doc, webServer.arg("plain"));

  if (e) {
    sendApiResponse(false, "configuration rejected: bad JSON");
    return;
  }

  float newSPM = doc["steps_per_ml"] | calibratedStepsPerML;
  float newScale = doc["hx711_scale"] | hx711ScaleFactor;
  float newDensity = doc["density_g_ml"] | fluidDensityGPerML;

  if (!isfinite(newSPM) || newSPM < MIN_CALIBRATED_STEPS_PER_ML ||
      newSPM > MAX_CALIBRATED_STEPS_PER_ML) {
    sendApiResponse(false, "invalid steps/mL: allowed range is 50 to 1000");
    return;
  }

  if (!isfinite(newScale) || fabsf(newScale) < 0.000001f || fabsf(newScale) > 100000000.0f) {
    sendApiResponse(false, "invalid HX711 scale factor");
    return;
  }

  if (!isfinite(newDensity) || newDensity < 0.1f || newDensity > 2.0f) {
    sendApiResponse(false, "invalid density: allowed range is 0.1 to 2.0 g/mL");
    return;
  }

  calibratedStepsPerML = newSPM;
  SPM = calibratedStepsPerML;
  hx711ScaleFactor = newScale;
  fluidDensityGPerML = newDensity;

  scale.set_scale(hx711ScaleFactor);

  if (!saveConfig()) {
    sendApiResponse(false, "configuration changed in RAM but NVS save failed");
    return;
  }

  publishEvent("configuration saved");
  sendApiResponse(true, "configuration saved to ESP32 NVS");
}

void handleApiGravimetricStart() {
  if (!requireWebAuth()) return;
  if (currentState != STATE_IDLE || !homed) {
    sendApiResponse(false, "gravimetric test rejected: pump must be IDLE and homed");
    return;
  }

  if (!loadCellReady) {
    sendApiResponse(false, "gravimetric test rejected: HX711 not ready");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError e = deserializeJson(doc, webServer.arg("plain"));

  if (e) {
    sendApiResponse(false, "gravimetric test rejected: bad JSON");
    return;
  }

  float volume = doc["volume_ml"] | 0.0f;
  float rate = doc["rate_ml_hr"] | 0.0f;
  float density = doc["density_g_ml"] | fluidDensityGPerML;

  if (!isfinite(density) || density < 0.1f || density > 2.0f) {
    sendApiResponse(false, "invalid test density");
    return;
  }

  if (volume <= 0.0f || volume > MAX_VOLUME_ML) {
    sendApiResponse(false, "invalid test volume");
    return;
  }

  if (rate <= 0.0f || rate > MAX_RATE_ML_HR) {
    sendApiResponse(false, "invalid test rate");
    return;
  }

  gravimetricDensityGPerML = density;
  gravimetricTargetML = volume;
  gravimetricResult = "running";
  gravimetricInitialGrams = loadCellGrams;
  gravimetricFinalGrams = loadCellGrams;
  gravimetricMeasuredML = 0.0f;
  gravimetricErrorPct = 0.0f;
  gravimetricStartMs = millis();
  gravimetricActive = true;

  String err;

  if (!startInfusion(volume, rate, err)) {
    gravimetricActive = false;
    sendApiResponse(false, "gravimetric test rejected: " + err);
    return;
  }

  if (activeInfusionHistory >= 0) {
    infusionHistory[activeInfusionHistory].gravimetric = true;
    infusionHistory[activeInfusionHistory].densityGPerML = gravimetricDensityGPerML;
    infusionHistory[activeInfusionHistory].initialGrams = gravimetricInitialGrams;
  }

  publishEvent("gravimetric verification started");
  sendApiResponse(true, "gravimetric test started");
}

void handleApiHistoryInfusions() {
  if (!requireWebAuth()) return;
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.createNestedArray("records");

  for (int n = infusionHistoryCount - 1; n >= 0; --n) {
    JsonObject r = arr.createNestedObject();
    const InfusionRecord& rec = infusionHistory[n];

    r["id"] = rec.id;
    r["target_ml"] = rec.targetML;
    r["pump_delivered_ml"] = rec.pumpDeliveredML;
    r["rate_ml_hr"] = rec.rateMLHr;
    r["duration_s"] = rec.endMs >= rec.startMs ? (rec.endMs - rec.startMs) / 1000.0f : 0.0f;
    r["gravimetric"] = rec.gravimetric;
    r["measured_ml"] = rec.measuredML;
    r["error_pct"] = rec.errorPct;
    r["initial_g"] = rec.initialGrams;
    r["final_g"] = rec.finalGrams;
    r["density_g_ml"] = rec.densityGPerML;
    r["result"] = rec.result;
  }

  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

void handleApiHistoryAlarms() {
  if (!requireWebAuth()) return;
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.createNestedArray("records");

  for (int n = alarmHistoryCount - 1; n >= 0; --n) {
    JsonObject r = arr.createNestedObject();
    const AlarmRecord& rec = alarmHistory[n];

    r["uptime_s"] = rec.uptimeMs / 1000.0f;
    r["state"] = rec.state;
    r["message"] = rec.message;
  }

  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

void handleApiClearHistory() {
  if (!requireWebAuth()) return;
  if (activeInfusionHistory >= 0 || gravimetricActive) {
    sendApiResponse(false, "cannot clear history while a run is active");
    return;
  }

  infusionHistoryCount = 0;
  activeInfusionHistory = -1;
  alarmHistoryCount = 0;
  sendApiResponse(true, "infusion and alarm history cleared from RAM");
}

void handleApiCommand() {
  if (!requireWebAuth()) return;
  String uri = webServer.uri();

  if (uri == "/api/start") {
    StaticJsonDocument<256> doc;
    DeserializationError e = deserializeJson(doc, webServer.arg("plain"));

    if (e) {
      sendApiResponse(false, "start rejected: bad JSON");
      return;
    }

    float vol = doc["volume_ml"] | 0.0;
    float rate = doc["rate_ml_hr"] | 0.0;

    String payload =
        String("{\"volume_ml\":") + String(vol, 3) + ",\"rate_ml_hr\":" + String(rate, 3) + "}";

    executeLocalCommand("/cmd/start", payload);

    sendApiResponse(true, "start command processed");
    return;
  }

  if (uri == "/api/stop") {
    executeLocalCommand("/cmd/stop");
    sendApiResponse(true, "stop command processed");
    return;
  }

  if (uri == "/api/pause") {
    executeLocalCommand("/cmd/pause");
    sendApiResponse(true, "pause command processed");
    return;
  }

  if (uri == "/api/resume") {
    executeLocalCommand("/cmd/resume");
    sendApiResponse(true, "resume command processed");
    return;
  }

  if (uri == "/api/home") {
    executeLocalCommand("/cmd/home");
    sendApiResponse(true, "home command processed");
    return;
  }

  if (uri == "/api/tare") {
    executeLocalCommand("/cmd/tare");
    sendApiResponse(true, "tare command processed");
    return;
  }

  if (uri == "/api/ping") {
    executeLocalCommand("/cmd/ping");
    sendApiResponse(true, "ping command processed");
    return;
  }

  sendApiResponse(false, "unknown API command");
}

void handleNotFound() {
  if (!requireWebAuth()) return;
  if (webServer.uri().startsWith("/api/")) {
    sendApiResponse(false, "API endpoint not found");
  } else {
    webServer.send(404, "text/plain", "Smart Syringe Pump: page not found");
  }
}

// Persistent config / history implementation

void loadPersistentConfig() {
  pumpPrefs.begin("spump", false);

  calibratedStepsPerML = pumpPrefs.getFloat("spm", CALIBRATED_STEPS_PER_ML_DEFAULT);

  if (!isfinite(calibratedStepsPerML) || calibratedStepsPerML < MIN_CALIBRATED_STEPS_PER_ML ||
      calibratedStepsPerML > MAX_CALIBRATED_STEPS_PER_ML) {
    calibratedStepsPerML = CALIBRATED_STEPS_PER_ML_DEFAULT;
  }

  hx711ScaleFactor = pumpPrefs.getFloat("hxscale", 1.0f);

  if (!isfinite(hx711ScaleFactor) || fabsf(hx711ScaleFactor) < 0.000001f ||
      fabsf(hx711ScaleFactor) > 100000000.0f) {
    hx711ScaleFactor = 1.0f;
  }

  fluidDensityGPerML = pumpPrefs.getFloat("density", 1.0f);

  if (!isfinite(fluidDensityGPerML) || fluidDensityGPerML < 0.1f || fluidDensityGPerML > 2.0f) {
    fluidDensityGPerML = 1.0f;
  }
}

bool saveConfig() {
  size_t a = pumpPrefs.putFloat("spm", calibratedStepsPerML);

  size_t b = pumpPrefs.putFloat("hxscale", hx711ScaleFactor);

  size_t c = pumpPrefs.putFloat("density", fluidDensityGPerML);

  return a > 0 && b > 0 && c > 0;
}

void addAlarmHistory(const String& stateName, const String& message) {
  if (alarmHistoryCount < MAX_ALARM_HISTORY) {
    alarmHistoryCount++;
  } else {
    for (uint8_t i = 1; i < MAX_ALARM_HISTORY; i++) {
      alarmHistory[i - 1] = alarmHistory[i];
    }
  }

  uint8_t idx = alarmHistoryCount - 1;

  alarmHistory[idx].uptimeMs = millis();

  snprintf(alarmHistory[idx].state, sizeof(alarmHistory[idx].state), "%s", stateName.c_str());

  snprintf(alarmHistory[idx].message, sizeof(alarmHistory[idx].message), "%s", message.c_str());
}

int addInfusionHistory(float targetML, float rateMLHr, bool gravimetric) {
  if (infusionHistoryCount < MAX_INFUSION_HISTORY) {
    infusionHistoryCount++;
  } else {
    for (uint8_t i = 1; i < MAX_INFUSION_HISTORY; i++) {
      infusionHistory[i - 1] = infusionHistory[i];
    }
  }

  int idx = infusionHistoryCount - 1;

  InfusionRecord& rec = infusionHistory[idx];

  memset(&rec, 0, sizeof(InfusionRecord));

  rec.id = nextInfusionId++;
  rec.startMs = millis();
  rec.targetML = targetML;
  rec.rateMLHr = rateMLHr;
  rec.gravimetric = gravimetric;
  rec.densityGPerML = gravimetric ? gravimetricDensityGPerML : fluidDensityGPerML;

  snprintf(rec.result, sizeof(rec.result), "%s", "running");

  return idx;
}

void finalizeInfusionHistory(const char* result) {
  if (activeInfusionHistory < 0 || activeInfusionHistory >= infusionHistoryCount) {
    return;
  }

  InfusionRecord& rec = infusionHistory[activeInfusionHistory];

  rec.endMs = millis();
  rec.pumpDeliveredML = deliveredVolumeML;

  snprintf(rec.result, sizeof(rec.result), "%s", result);

  if (gravimetricActive) {
    rec.gravimetric = true;
    rec.densityGPerML = gravimetricDensityGPerML;
    rec.initialGrams = gravimetricInitialGrams;
    rec.finalGrams = gravimetricFinalGrams;
    rec.measuredML = gravimetricMeasuredML;
    rec.errorPct = gravimetricErrorPct;
  }

  activeInfusionHistory = -1;
}

void finalizeGravimetricTest(const char* result) {
  if (!gravimetricActive) {
    return;
  }

  gravimetricFinalGrams = loadCellGrams;

  float massDelivered = gravimetricFinalGrams - gravimetricInitialGrams;

  gravimetricMeasuredML = massDelivered / max(gravimetricDensityGPerML, 0.000001f);

  float target = targetVolumeML;

  if (activeInfusionHistory >= 0 && activeInfusionHistory < infusionHistoryCount) {
    target = infusionHistory[activeInfusionHistory].targetML;
  }

  if (target > 0.0f) {
    gravimetricErrorPct = ((gravimetricMeasuredML - target) / target) * 100.0f;
  } else {
    gravimetricErrorPct = 0.0f;
  }

  gravimetricActive = false;
  gravimetricResult = String(result);

  publishEvent(String("gravimetric test ") + result);
}

void sampleLoadCell() {
  unsigned long now = millis();

  if (now - lastLoadCellSampleMs < LOAD_CELL_SAMPLE_MS) {
    return;
  }

  lastLoadCellSampleMs = now;

  loadCellReady = scale.is_ready();

  if (loadCellReady) {
    loadCellGrams = scale.get_units(1);
  }
}

// Mqtt event

void publishEvent(const String& msg) {
  lastWebEvent = msg;
  lastWebEventMs = millis();

  Serial.print("EVENT: ");

  Serial.println(msg);

  if (mqtt.connected()) {
    mqtt.publish(topicEvent.c_str(), msg.c_str());
  }
}

// Telemetry

void publishTelemetry() {
  if (!mqtt.connected()) {
    return;
  }

  float rate = (currentState == STATE_INFUSING) ? flowRateMLPerHr : 0.0;

  mqtt.publish(topicTelFlow.c_str(), String(rate, 2).c_str());

  mqtt.publish(topicTelVol.c_str(), String(deliveredVolumeML, 3).c_str());

  mqtt.publish(topicTelCur.c_str(), String((int)currentFiltered).c_str());

  mqtt.publish(topicTelPos.c_str(), String(stepper.currentPosition()).c_str());

  if (loadCellReady) {
    StaticJsonDocument<64> doc;

    doc["grams"] = loadCellGrams;

    char buf[64];

    serializeJson(doc, buf);

    mqtt.publish(topicTelLoad.c_str(), buf);
  }
}

// Status

void publishStatus() {
  if (!mqtt.connected()) {
    return;
  }

  StaticJsonDocument<384> doc;

  switch (currentState) {
    case STATE_IDLE:

      doc["state"] = "idle";

      break;

    case STATE_HOMING:

      doc["state"] = "homing";

      break;

    case STATE_PRIMING:

      doc["state"] = "priming";

      break;

    case STATE_INFUSING:

      doc["state"] = "infusing";

      break;

    case STATE_PAUSED:

      doc["state"] = "paused";

      break;

    case STATE_OCCLUDED:

      doc["state"] = "occluded";

      break;

    case STATE_COMPLETE:

      doc["state"] = "complete";

      break;

    case STATE_ALARM:

      doc["state"] = "alarm";

      break;
  }

  doc["homed"] = homed;

  doc["target_volume_ml"] = targetVolumeML;

  doc["delivered_volume_ml"] = deliveredVolumeML;

  doc["position"] = stepper.currentPosition();

  doc["steps_completed"] = stepsCompleted;

  doc["total_steps"] = totalStepsForRun;

  doc["max_steps"] = SYRINGE_EMPTY_POSITION_STEPS;

  doc["calibrated_steps_per_ml"] = calibratedStepsPerML;

  doc["acceleration"] = INFUSION_ACCEL;

  if (alarmMessage.length() > 0) {
    doc["message"] = alarmMessage;
  }

  char buf[384];

  serializeJson(doc, buf);

  mqtt.publish(topicStatus.c_str(), buf, true);
}