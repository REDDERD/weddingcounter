# weddingcounter

Ein E-Paper-Display (ESPink), das seit dem Hochzeitstag die Zeit bis zum
naechsten Jahrestag bzw. seit der Hochzeit zaehlt: in den ersten 12 Monaten
in Tagen, danach in Jahren + Tagen seit dem letzten Jahrestag. Am Jahrestag
selbst wird eine Glueckwunsch-Meldung statt der Tage angezeigt.

## Hardware

- ESPink v3.5/3.6 (ESP32)
- E-Paper-Display GDEY042T81 (4.2", 400x300px)
- RTC-Modul DS3231 (I2C)

Pin-Belegung ist am Anfang von [main/main.ino](main/main.ino) definiert
(Display-Pins, `SDA_PIN`/`SCL_PIN` fuer den I2C-Bus, `POWER`-Pin zum
Ein-/Ausschalten der Peripherie, `VBAT_PIN` fuer die Akku-Messung).

## Sketches

### `main/`

Der eigentliche Zaehler. Liest bei jedem Boot die Uhrzeit von der RTC,
berechnet Jahre/Tage seit dem in `WED_YEAR`/`WED_MONTH`/`WED_DAY` hinterlegten
Hochzeitsdatum und zeichnet das Ergebnis auf das Display. Danach geht das
Board in den Deep Sleep bis Mitternacht (bzw. nach 25h als Fallback), um den
Akku zu schonen.

Falls die RTC beim Boot keine Zeit liefert (z. B. nach Batteriewechsel), wird
stattdessen ein Fehlerbildschirm angezeigt, der zum Stellen der Uhr auffordert.

**Vor dem Hochladen anpassen:**

- `WED_YEAR`, `WED_MONTH`, `WED_DAY` — das Hochzeitsdatum
- In `computeYearsDays()` gibt es eine auskommentierte Zeile, mit der sich ein
  festes "Jetzt"-Datum zum Testen erzwingen laesst. Fuer den Normalbetrieb
  muss diese Zeile entfernt/auskommentiert sein.

### `set_rtc_time/`

Einmal-Sketch, um die Uhrzeit auf dem RTC-Modul zu stellen. Er tut beim Boot
nichts weiter, als die im Code hartcodierte Zeit (`SET_YEAR`, `SET_MONTH`,
`SET_DAY`, `SET_HOUR`, `SET_MINUTE`, `SET_SECOND`) in die RTC zu schreiben.

**Ablauf:**

1. In [set_rtc_time/set_rtc_time.ino](set_rtc_time/set_rtc_time.ino) die
   `SET_*`-Konstanten auf die gewuenschte Zeit setzen (idealerweise ein paar
   Sekunden in der Zukunft, um Upload- und Boot-Dauer auszugleichen).
2. Sketch hochladen und einmal booten lassen (Board danach ausstecken oder
   neu starten reicht, es muss nicht weiterlaufen).
3. Zur Kontrolle gibt der Sketch die neu gesetzte RTC-Zeit ueber die
   seriellen Konsole (115200 Baud) aus.
4. Anschliessend wieder `main/main.ino` hochladen.

Die RTC (DS3231) hat eine eigene Pufferbatterie und behaelt die Zeit auch
ohne Hauptstromversorgung, das Stellen ist also nur einmalig bzw. nach einem
Batteriewechsel noetig.

## Schriften

Die Zaehlertexte werden mit der Skript-Schriftart "Great Vibes" in mehreren
Groessen gerendert (`main/GreatVibes*.h`, als Adafruit-GFX-Bitmap-Fonts).
Diese Fonts decken nur den ASCII-Bereich (0x20-0x7E) ab — Umlaute (ä/ö/ü/ß)
werden daher in allen angezeigten Texten vermieden (z. B. "Glueckwunsch"
statt "Glückwunsch").
