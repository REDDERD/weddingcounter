// Einmal-Sketch zum Stellen der RTC (DS3231).
// Traegt die unten hartcodierte Zeit beim Boot in das RTC-Modul ein und tut
// sonst nichts. Vor dem Hochladen die Werte unter "Zeit zum Setzen" anpassen
// (z. B. auf die aktuelle Uhrzeit + ein paar Sekunden fuer die Upload-/Boot-Zeit).
//
// Nach dem Setzen kann dieser Sketch wieder durch main.ino ersetzt werden.

#include <Wire.h>
#include <RTClib.h>

// ---- Pins (ESPink v3.5/3.6) ----
#define POWER 47
#define SDA_PIN 42
#define SCL_PIN 2

// ---- Zeit zum Setzen (24h-Format, lokale Zeit) ----
#define SET_YEAR   2026
#define SET_MONTH  10
#define SET_DAY    1
#define SET_HOUR   18
#define SET_MINUTE 20
#define SET_SECOND 0

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!rtc.begin()) {
    Serial.println("RTC nicht gefunden!");
    return;
  }

  rtc.adjust(DateTime(SET_YEAR, SET_MONTH, SET_DAY, SET_HOUR, SET_MINUTE, SET_SECOND));

  DateTime now = rtc.now();
  Serial.print("RTC gesetzt auf: ");
  Serial.print(now.year());
  Serial.print("-");
  Serial.print(now.month());
  Serial.print("-");
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
}

void loop() {}
