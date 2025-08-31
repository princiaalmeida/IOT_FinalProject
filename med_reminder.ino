#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// --- WiFi Credentials ---
const char* ssid = "Techno";
const char* password = "meGhana12";

// --- Hardware Pin Definitions ---
const int BUZZER_PIN = 18;
const int LED_PINS[] = {19, 23, 25, 26};
const int BTN_PINS[] = {13, 14, 27, 33};

// --- OLED Display Definitions ---
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Hard-coded Reminder Times ---
struct Reminder {
  int hour;
  int minute;
};
Reminder reminders[4] = {
  {8, 0},   // Reminder 1 at 8:00 AM
  {13, 47},  // Reminder 2 at 1:47 PM
  {19, 0},  // Reminder 3 at 7:00 PM
  {22, 0}   // Reminder 4 at 10:00 PM
};

bool isAlarmActive[4] = {false, false, false, false};
bool isAlarmAcknowledged[4] = {false, false, false, false};

// --- Variables for Repeating Beep ---
unsigned long lastBeepTime = 0;
const long beepInterval = 5000; // Beep every 5 seconds (adjust as needed)

// --- Update the OLED display ---
void updateDisplay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  char timeString[20];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  display.println("Current Time:");
  display.setTextSize(2);
  display.println(timeString);
  display.setTextSize(1);

  display.setCursor(0, 32);
  display.println("Reminders:");
  for (int i = 0; i < 4; i++) {
    char rem_str[6];
    sprintf(rem_str, "%02d:%02d", reminders[i].hour, reminders[i].minute);
    display.print("Med " + String(i+1) + ": " + String(rem_str));
    if (isAlarmActive[i]) {
      display.println(" -> ALARM!");
    } else if (isAlarmAcknowledged[i]) {
      display.println(" -> Taken");
    } else {
      display.println();
    }
  }

  display.display();
}

// --- Check for alarms and activate if needed ---
void checkAlarms() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  for (int i = 0; i < 4; i++) {
    if (timeinfo.tm_hour == reminders[i].hour && timeinfo.tm_min == reminders[i].minute && !isAlarmAcknowledged[i]) {
      isAlarmActive[i] = true;
      digitalWrite(LED_PINS[i], HIGH);
    }
  }
}

// --- Setup Function ---
void setup() {
  Serial.begin(115200);

  // Initialize hardware pins
  pinMode(BUZZER_PIN, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    digitalWrite(LED_PINS[i], LOW);
  }

  // Initialize OLED display
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  
  // Configure time from NTP server (IST is UTC+5:30)
  configTime(19800, 0, "pool.ntp.org"); 
}

// --- Main Loop ---
void loop() {
  checkAlarms();
  
  // Check physical buttons to acknowledge alarms
  for (int i = 0; i < 4; i++) {
    if (digitalRead(BTN_PINS[i]) == LOW && isAlarmActive[i]) {
      isAlarmActive[i] = false;
      isAlarmAcknowledged[i] = true;
      digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
      digitalWrite(LED_PINS[i], LOW);
    }
  }
  
  // Repeating beep logic for active alarms
  for (int i = 0; i < 4; i++) {
    if (isAlarmActive[i] && (millis() - lastBeepTime >= beepInterval)) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200); // Beep for 200 ms
      digitalWrite(BUZZER_PIN, LOW);
      lastBeepTime = millis();
    }
  }

  // Update display
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 1000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
}