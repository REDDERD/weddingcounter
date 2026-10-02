#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include "bitmap.h"
#include "GreatVibes9pt7b.h"
#include "GreatVibes12pt7b.h"
#include "GreatVibes18pt7b.h"
#include "GreatVibes24pt7b.h"
#include "GreatVibes28pt7b.h"
#include <Wire.h>
#include <RTClib.h>

// ---- Pins (ESPink v3.5/3.6, Display GDEY042T81) ----
#define DC 48
#define RST 45
#define BUSY 38
#define POWER 47
#define SDA_PIN 42
#define SCL_PIN 2
#define VBAT_PIN 9

// ---- set wedding date here ----
#define WED_YEAR 2025
#define WED_MONTH 8
#define WED_DAY 22

#define MAX_SLEEP_SEC (25UL * 3600UL)

GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(SS, DC, RST, BUSY));
RTC_DS3231 rtc;

void printCenteredAt(const char* txt, int cx, int y, const GFXfont* font) {
  display.setFont(font);
  int16_t bx, by;
  uint16_t bw, bh;
  display.getTextBounds(txt, 0, 0, &bx, &by, &bw, &bh);
  display.setCursor(cx - bw / 2 - bx, y);
  display.print(txt);
}

void computeYearsDays(DateTime now, int& years, long& days) {
  years = now.year() - WED_YEAR;
  if (now.month() < WED_MONTH || (now.month() == WED_MONTH && now.day() < WED_DAY)) {
    years--;
  }
  if (years < 0) years = 0;
  DateTime anniv(WED_YEAR + years, WED_MONTH, WED_DAY, 0, 0, 0);
  days = (now.unixtime() - anniv.unixtime()) / 86400L;
}

long daysUntilNextAnniversary(DateTime now, int years) {
  DateTime nextAnniv(WED_YEAR + years + 1, WED_MONTH, WED_DAY, 0, 0, 0);
  return (nextAnniv.unixtime() - now.unixtime()) / 86400L;
}

// ---- battery charge ----
float readVbatt() {
  analogReadResolution(12);
  analogSetPinAttenuation(VBAT_PIN, ADC_11db);
  uint32_t mv = 0;
  const int N = 16;
  for (int i = 0; i < N; i++) {
    mv += analogReadMilliVolts(VBAT_PIN);
    delay(3);
  }
  return (mv / (float)N) / 1000.0f * 2.0f;
}
int battPercent(float v) {
  const float vt[] = { 3.30, 3.45, 3.55, 3.62, 3.68, 3.73, 3.80, 3.87, 3.93, 4.00, 4.10, 4.20 };
  const int pt[] = { 0, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
  const int n = 12;
  if (v <= vt[0]) return 0;
  if (v >= vt[n - 1]) return 100;
  for (int i = 0; i < n - 1; i++)
    if (v < vt[i + 1]) {
      float f = (v - vt[i]) / (vt[i + 1] - vt[i]);
      return (int)(pt[i] + f * (pt[i + 1] - pt[i]) + 0.5f);
    }
  return 100;
}

void printBatterySmall() {
  char b[8];
  snprintf(b, sizeof(b), "%d%%", battPercent(readVbatt()));
  display.setFont(NULL);
  display.setTextSize(1);
  int16_t bx, by;
  uint16_t bw, bh;
  display.getTextBounds(b, 0, 0, &bx, &by, &bw, &bh);
  display.setCursor(display.width() - bw - 4, display.height() - bh - 3);
  display.print(b);
}

void showCounter() {
  DateTime now = rtc.now();
  // ---- TEST: for testing set the date & time that should be simulated ----
  // now = DateTime(2028, 10, 28, 0, 0, 0);	// use this line for testing
  // -----------------------------------------------------------------
  int years;
  long days;
  computeYearsDays(now, years, days);

  char lineJahre[16], lineTage[16];
  if (years == 1) snprintf(lineJahre, sizeof(lineJahre), "1 Jahr");
  else snprintf(lineJahre, sizeof(lineJahre), "%d Jahre", years);
  if (days == 1) snprintf(lineTage, sizeof(lineTage), "1 Tag");
  else snprintf(lineTage, sizeof(lineTage), "%ld Tage", days);

  const int RX = 300;

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

      display.drawBitmap(0, 0, WED_BMP, WED_BMP_W, WED_BMP_H, GxEPD_BLACK);

    if (years <= 0) {
      printCenteredAt("Verheiratet seit:", RX, 120, &GreatVibes18pt7b);
      printCenteredAt(lineTage, RX, 190, &GreatVibes28pt7b);
    } else if (days == 0) {
      // Hochzeitstag (Jahrestag): statt der Tage eine Glueckwunsch-Zeile anzeigen
      printCenteredAt("Verheiratet seit:", RX, 65, &GreatVibes18pt7b);
      printCenteredAt(lineJahre, RX, 126, &GreatVibes28pt7b);
      printCenteredAt("Alles Gute zum", RX, 190, &GreatVibes18pt7b);
      printCenteredAt("Hochzeitstag!", RX, 230, &GreatVibes18pt7b);
    } else {
      printCenteredAt("Verheiratet seit:", RX, 65, &GreatVibes18pt7b);
      printCenteredAt(lineJahre, RX, 126, &GreatVibes28pt7b);
      printCenteredAt(lineTage, RX, 200, &GreatVibes28pt7b);
      }

    printBatterySmall();
  } while (display.nextPage());
  display.hibernate();
}

void showError() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    printCenteredAt("Uhr-Zeit verloren", 200, 90, &GreatVibes12pt7b);
    printCenteredAt("Datum stimmt nicht mehr", 200, 140, &GreatVibes9pt7b);
    printCenteredAt("Bitte Uhr per USB", 200, 185, &GreatVibes9pt7b);
    printCenteredAt("neu stellen", 200, 215, &GreatVibes9pt7b);
  } while (display.nextPage());
  display.hibernate();
}

uint64_t secondsUntilMidnight() {
  DateTime now = rtc.now();
  long secsIntoDay = (long)now.hour() * 3600L + (long)now.minute() * 60L + now.second();
  long remaining = 86400L - secsIntoDay + 2;
  if (remaining < 60) remaining += 86400L;
  if (remaining > (long)MAX_SLEEP_SEC) remaining = MAX_SLEEP_SEC;
  return (uint64_t)remaining;
}
void sleepUntilMidnight() {
  digitalWrite(POWER, LOW);
  esp_sleep_enable_timer_wakeup(secondsUntilMidnight() * 1000000ULL);
  delay(50);
  esp_deep_sleep_start();
}
void sleepFallback() {
  digitalWrite(POWER, LOW);
  esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL);
  delay(50);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);
  delay(200);
  display.init();
  display.setRotation(4);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!rtc.begin()) {
    showError();
    sleepFallback();
  }
  if (rtc.lostPower()) {
    showError();
    sleepFallback();
  }

  showCounter();
  sleepUntilMidnight();
}
void loop() {}
