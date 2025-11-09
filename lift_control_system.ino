#include <LiquidCrystal.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define FLOOR_1 1
#define FLOOR_2 2
#define FLOOR_3 3

#define BUTTON_FLOOR_1 2
#define BUTTON_FLOOR_2 3
#define BUTTON_FLOOR_3 4
#define BUTTON_INSIDE_1 5
#define BUTTON_INSIDE_2 6
#define BUTTON_INSIDE_3 7
#define EMERGENCY_STOP 8

#define RELAY_UP 9
#define RELAY_DOWN 10
#define RELAY_DOOR_OPEN 11
#define RELAY_DOOR_CLOSE 12

#define LED_FLOOR_1 13
#define LED_FLOOR_2 14
#define LED_FLOOR_3 15
#define LED_EMERGENCY 16
#define LED_MOVING 17

#define BUZZER 18

#define SENSOR_FLOOR_1 19
#define SENSOR_FLOOR_2 20
#define SENSOR_FLOOR_3 21
#define SENSOR_DOOR_OPEN 22
#define SENSOR_DOOR_CLOSE 23
#define SENSOR_OBSTRUCTION 24
#define SENSOR_WEIGHT A1

#define DOOR_OPEN_TIME 3000
#define DEBOUNCE_DELAY 50
#define EMERGENCY_TIMEOUT 5000

LiquidCrystal lcd(26, 27, 28, 29, 30, 31);

char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

int currentFloor = 1;
int targetFloor = 1;
bool isMoving = false;
bool emergencyActive = false;
bool doorOpen = false;
bool powerFailure = false;
bool overloadDetected = false;
bool obstructionDetected = false;
bool communicationFailure = false;

unsigned long lastButtonPress = 0;
unsigned long emergencyStartTime = 0;
unsigned long doorOpenTime = 0;

bool floor1Requested = false;
bool floor2Requested = false;
bool floor3Requested = false;

bool buttonStates[6] = {false, false, false, false, false, false};
bool lastButtonStates[6] = {false, false, false, false, false, false};
unsigned long lastDebounceTime[6] = {0};

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_FLOOR_1, INPUT_PULLUP);
  pinMode(BUTTON_FLOOR_2, INPUT_PULLUP);
  pinMode(BUTTON_FLOOR_3, INPUT_PULLUP);
  pinMode(BUTTON_INSIDE_1, INPUT_PULLUP);
  pinMode(BUTTON_INSIDE_2, INPUT_PULLUP);
  pinMode(BUTTON_INSIDE_3, INPUT_PULLUP);
  pinMode(EMERGENCY_STOP, INPUT_PULLUP);
  
  pinMode(RELAY_UP, OUTPUT);
  pinMode(RELAY_DOWN, OUTPUT);
  pinMode(RELAY_DOOR_OPEN, OUTPUT);
  pinMode(RELAY_DOOR_CLOSE, OUTPUT);
  
  pinMode(LED_FLOOR_1, OUTPUT);
  pinMode(LED_FLOOR_2, OUTPUT);
  pinMode(LED_FLOOR_3, OUTPUT);
  pinMode(LED_EMERGENCY, OUTPUT);
  pinMode(LED_MOVING, OUTPUT);
  
  pinMode(BUZZER, OUTPUT);
  
  pinMode(SENSOR_FLOOR_1, INPUT);
  pinMode(SENSOR_FLOOR_2, INPUT);
  pinMode(SENSOR_FLOOR_3, INPUT);
  pinMode(SENSOR_DOOR_OPEN, INPUT);
  pinMode(SENSOR_DOOR_CLOSE, INPUT);
  pinMode(SENSOR_OBSTRUCTION, INPUT);
  
  digitalWrite(RELAY_UP, LOW);
  digitalWrite(RELAY_DOWN, LOW);
  digitalWrite(RELAY_DOOR_OPEN, LOW);
  digitalWrite(RELAY_DOOR_CLOSE, LOW);
  
  lcd.begin(16, 2);
  lcd.print("Lift System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  
  WiFi.begin(ssid, pass);
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.begin(auth, ssid, pass);
    communicationFailure = false;
    lcd.clear();
    lcd.print("WiFi Connected");
    delay(1000);
  } else {
    communicationFailure = true;
    lcd.clear();
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Using Backup");
    delay(1000);
  }
  
  updateFloorDisplay();
  updateFloorLEDs();
  
  Serial.println("Lift System Initialized");
}

void loop() {
  if (!communicationFailure) {
    Blynk.run();
  }
  
  checkPowerStatus();
  checkEmergencyStop();
  checkOverload();
  checkObstruction();
  
  if (emergencyActive) {
    handleEmergency();
    return;
  }
  
  if (powerFailure) {
    handlePowerFailure();
    return;
  }
  
  if (overloadDetected) {
    handleOverload();
    return;
  }
  
  if (obstructionDetected) {
    handleObstruction();
    return;
  }
  
  readInputs();
  processRequests();
  updateFloorSensors();
  updateDoorStatus();
  
  if (!isMoving && targetFloor != currentFloor) {
    moveLift();
  }
  
  if (currentFloor == targetFloor && !doorOpen) {
    openDoor();
  }
  
  updateDisplay();
  sendStatusToBlynk();
  delay(10);
}

bool debounceButton(int buttonPin, int buttonIndex) {
  bool reading = digitalRead(buttonPin) == LOW;
  
  if (reading != lastButtonStates[buttonIndex]) {
    lastDebounceTime[buttonIndex] = millis();
  }
  
  if ((millis() - lastDebounceTime[buttonIndex]) > DEBOUNCE_DELAY) {
    if (reading != buttonStates[buttonIndex]) {
      buttonStates[buttonIndex] = reading;
      if (reading) {
        lastButtonStates[buttonIndex] = reading;
        return true;
      }
    }
  }
  
  lastButtonStates[buttonIndex] = reading;
  return false;
}

void readInputs() {
  if (debounceButton(BUTTON_FLOOR_1, 0)) {
    if (validateInput(FLOOR_1)) {
      floor1Requested = true;
      if (!isMoving && targetFloor == currentFloor) targetFloor = FLOOR_1;
    }
  }
  
  if (debounceButton(BUTTON_FLOOR_2, 1)) {
    if (validateInput(FLOOR_2)) {
      floor2Requested = true;
      if (!isMoving && targetFloor == currentFloor) targetFloor = FLOOR_2;
    }
  }
  
  if (debounceButton(BUTTON_FLOOR_3, 2)) {
    if (validateInput(FLOOR_3)) {
      floor3Requested = true;
      if (!isMoving && targetFloor == currentFloor) targetFloor = FLOOR_3;
    }
  }
  
  if (debounceButton(BUTTON_INSIDE_1, 3)) {
    if (validateInput(FLOOR_1)) {
      floor1Requested = true;
      if (!isMoving && targetFloor == currentFloor) targetFloor = FLOOR_1;
    }
  }
  
  if (debounceButton(BUTTON_INSIDE_2, 4)) {
    if (validateInput(FLOOR_2)) {
      floor2Requested = true;
      if (!isMoving && targetFloor == currentFloor) targetFloor = FLOOR_2;
    }
  }
  
  if (debounceButton(BUTTON_INSIDE_3, 5)) {
    if (validateInput(FLOOR_3)) {
      floor3Requested = true;
      if (!isMoving && targetFloor == currentFloor) targetFloor = FLOOR_3;
    }
  }
}

bool validateInput(int floor) {
  if (floor < FLOOR_1 || floor > FLOOR_3) {
    showError("Invalid Floor");
    return false;
  }
  
  if (floor == currentFloor && !isMoving) {
    return false;
  }
  
  if (isMoving) {
    if ((currentFloor < targetFloor && floor < currentFloor) ||
        (currentFloor > targetFloor && floor > currentFloor)) {
      return false;
    }
  }
  
  return true;
}

void processRequests() {
  if (isMoving) {
    return;
  }
  
  if (targetFloor == currentFloor) {
    if (floor1Requested && currentFloor != FLOOR_1) {
      targetFloor = FLOOR_1;
      floor1Requested = false;
    } else if (floor2Requested && currentFloor != FLOOR_2) {
      targetFloor = FLOOR_2;
      floor2Requested = false;
    } else if (floor3Requested && currentFloor != FLOOR_3) {
      targetFloor = FLOOR_3;
      floor3Requested = false;
    }
  }
  
  if (targetFloor != currentFloor) {
    int priorityFloor = getPriorityFloor();
    if (priorityFloor != 0) {
      targetFloor = priorityFloor;
    }
  }
}

int getPriorityFloor() {
  if (floor1Requested && currentFloor > FLOOR_1) {
    return FLOOR_1;
  }
  if (floor3Requested && currentFloor < FLOOR_3) {
    return FLOOR_3;
  }
  if (floor2Requested) {
    return FLOOR_2;
  }
  if (floor1Requested) {
    return FLOOR_1;
  }
  if (floor3Requested) {
    return FLOOR_3;
  }
  return 0;
}

void moveLift() {
  if (targetFloor > currentFloor) {
    isMoving = true;
    digitalWrite(LED_MOVING, HIGH);
    digitalWrite(RELAY_UP, HIGH);
    digitalWrite(RELAY_DOWN, LOW);
  } else if (targetFloor < currentFloor) {
    isMoving = true;
    digitalWrite(LED_MOVING, HIGH);
    digitalWrite(RELAY_UP, LOW);
    digitalWrite(RELAY_DOWN, HIGH);
  }
}

void updateFloorSensors() {
  if (digitalRead(SENSOR_FLOOR_1) == HIGH) {
    if (currentFloor != FLOOR_1) {
      currentFloor = FLOOR_1;
      if (targetFloor == FLOOR_1) {
        stopLift();
        floor1Requested = false;
      }
      updateFloorLEDs();
    }
  } else if (digitalRead(SENSOR_FLOOR_2) == HIGH) {
    if (currentFloor != FLOOR_2) {
      currentFloor = FLOOR_2;
      if (targetFloor == FLOOR_2) {
        stopLift();
        floor2Requested = false;
      }
      updateFloorLEDs();
    }
  } else if (digitalRead(SENSOR_FLOOR_3) == HIGH) {
    if (currentFloor != FLOOR_3) {
      currentFloor = FLOOR_3;
      if (targetFloor == FLOOR_3) {
        stopLift();
        floor3Requested = false;
      }
      updateFloorLEDs();
    }
  }
}

void stopLift() {
  isMoving = false;
  digitalWrite(LED_MOVING, LOW);
  digitalWrite(RELAY_UP, LOW);
  digitalWrite(RELAY_DOWN, LOW);
  targetFloor = currentFloor;
}

void openDoor() {
  doorOpen = true;
  digitalWrite(RELAY_DOOR_OPEN, HIGH);
  digitalWrite(RELAY_DOOR_CLOSE, LOW);
  doorOpenTime = millis();
  
  while (digitalRead(SENSOR_DOOR_OPEN) == LOW && (millis() - doorOpenTime) < DOOR_OPEN_TIME) {
    delay(10);
  }
  
  delay(DOOR_OPEN_TIME);
  
  closeDoor();
}

void closeDoor() {
  if (digitalRead(SENSOR_OBSTRUCTION) == HIGH) {
    obstructionDetected = true;
    return;
  }
  
  digitalWrite(RELAY_DOOR_OPEN, LOW);
  digitalWrite(RELAY_DOOR_CLOSE, HIGH);
  
  while (digitalRead(SENSOR_DOOR_CLOSE) == LOW) {
    if (digitalRead(SENSOR_OBSTRUCTION) == HIGH) {
      obstructionDetected = true;
      digitalWrite(RELAY_DOOR_CLOSE, LOW);
      return;
    }
    delay(10);
  }
  
  doorOpen = false;
  digitalWrite(RELAY_DOOR_CLOSE, LOW);
}

void updateDoorStatus() {
  if (digitalRead(SENSOR_DOOR_OPEN) == HIGH) {
    doorOpen = true;
  } else if (digitalRead(SENSOR_DOOR_CLOSE) == HIGH) {
    doorOpen = false;
  }
}

void checkEmergencyStop() {
  if (digitalRead(EMERGENCY_STOP) == LOW) {
    emergencyActive = true;
    emergencyStartTime = millis();
    stopLift();
    digitalWrite(LED_EMERGENCY, HIGH);
    tone(BUZZER, 1000);
  }
}

void checkPowerStatus() {
  if (analogRead(A0) < 100) {
    powerFailure = true;
  } else {
    powerFailure = false;
  }
}

void checkOverload() {
  int weightValue = analogRead(SENSOR_WEIGHT);
  if (weightValue > 800) {
    overloadDetected = true;
  } else {
    overloadDetected = false;
  }
}

void checkObstruction() {
  if (digitalRead(SENSOR_OBSTRUCTION) == HIGH && doorOpen) {
    obstructionDetected = true;
  }
}

void handleEmergency() {
  stopLift();
  digitalWrite(LED_EMERGENCY, HIGH);
  tone(BUZZER, 1000);
  
  lcd.clear();
  lcd.print("EMERGENCY STOP");
  lcd.setCursor(0, 1);
  lcd.print("Floor: ");
  lcd.print(currentFloor);
  
  if (!communicationFailure) {
    Blynk.virtualWrite(V1, "EMERGENCY");
    Blynk.virtualWrite(V2, currentFloor);
  }
  
  if (digitalRead(EMERGENCY_STOP) == HIGH && (millis() - emergencyStartTime) > EMERGENCY_TIMEOUT) {
    emergencyActive = false;
    digitalWrite(LED_EMERGENCY, LOW);
    noTone(BUZZER);
    lcd.clear();
    updateFloorDisplay();
  }
}

void handlePowerFailure() {
  stopLift();
  digitalWrite(LED_EMERGENCY, HIGH);
  tone(BUZZER, 500);
  
  lcd.clear();
  lcd.print("Power Failure");
  lcd.setCursor(0, 1);
  lcd.print("Emergency Mode");
  
  if (currentFloor != FLOOR_1) {
    digitalWrite(RELAY_DOWN, HIGH);
    while (digitalRead(SENSOR_FLOOR_1) == LOW) {
      delay(10);
    }
    stopLift();
    openDoor();
  }
  
  if (!communicationFailure) {
    Blynk.virtualWrite(V1, "POWER FAILURE");
  }
}

void handleOverload() {
  stopLift();
  digitalWrite(LED_EMERGENCY, HIGH);
  tone(BUZZER, 800);
  
  lcd.clear();
  lcd.print("OVERLOAD");
  lcd.setCursor(0, 1);
  lcd.print("Reduce Weight");
  
  if (!communicationFailure) {
    Blynk.virtualWrite(V1, "OVERLOAD");
  }
  
  if (!overloadDetected) {
    digitalWrite(LED_EMERGENCY, LOW);
    noTone(BUZZER);
    lcd.clear();
    updateFloorDisplay();
  }
}

void handleObstruction() {
  digitalWrite(RELAY_DOOR_CLOSE, LOW);
  digitalWrite(LED_EMERGENCY, HIGH);
  
  lcd.clear();
  lcd.print("Door Obstructed");
  lcd.setCursor(0, 1);
  lcd.print("Clear Path");
  
  if (!communicationFailure) {
    Blynk.virtualWrite(V1, "OBSTRUCTION");
  }
  
  if (digitalRead(SENSOR_OBSTRUCTION) == LOW) {
    obstructionDetected = false;
    digitalWrite(LED_EMERGENCY, LOW);
    lcd.clear();
    updateFloorDisplay();
    closeDoor();
  }
}

void updateFloorDisplay() {
  lcd.clear();
  lcd.print("Floor: ");
  lcd.print(currentFloor);
  lcd.setCursor(0, 1);
  if (isMoving) {
    if (targetFloor > currentFloor) {
      lcd.print("Moving Up");
    } else {
      lcd.print("Moving Down");
    }
  } else {
    lcd.print("Ready");
  }
}

void updateFloorLEDs() {
  digitalWrite(LED_FLOOR_1, (currentFloor == FLOOR_1) ? HIGH : LOW);
  digitalWrite(LED_FLOOR_2, (currentFloor == FLOOR_2) ? HIGH : LOW);
  digitalWrite(LED_FLOOR_3, (currentFloor == FLOOR_3) ? HIGH : LOW);
}

void updateDisplay() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 500) {
    updateFloorDisplay();
    lastUpdate = millis();
  }
}

void showError(String errorMsg) {
  lcd.clear();
  lcd.print(errorMsg);
  digitalWrite(LED_EMERGENCY, HIGH);
  delay(2000);
  digitalWrite(LED_EMERGENCY, LOW);
  updateFloorDisplay();
}

void sendStatusToBlynk() {
  if (communicationFailure) {
    return;
  }
  
  static unsigned long lastBlynkUpdate = 0;
  if (millis() - lastBlynkUpdate > 1000) {
    Blynk.virtualWrite(V0, currentFloor);
    Blynk.virtualWrite(V1, isMoving ? "MOVING" : "STOPPED");
    Blynk.virtualWrite(V2, targetFloor);
    Blynk.virtualWrite(V3, doorOpen ? "OPEN" : "CLOSED");
    lastBlynkUpdate = millis();
  }
}

BLYNK_WRITE(V10) {
  int floor = param.asInt();
  if (validateInput(floor)) {
    if (floor == FLOOR_1) floor1Requested = true;
    if (floor == FLOOR_2) floor2Requested = true;
    if (floor == FLOOR_3) floor3Requested = true;
    if (!isMoving && targetFloor == currentFloor) targetFloor = floor;
  }
}

BLYNK_WRITE(V11) {
  if (param.asInt() == 1) {
    emergencyActive = true;
  }
}

