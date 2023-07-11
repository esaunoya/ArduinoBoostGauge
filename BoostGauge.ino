#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define DISPLAY_ADDRESS 0x3C
#define SENSOR_ADDRESS  0x28

#define MIN_SENSOR_PRESSURE 0.0 // minimum pressure the sensor can measure (psi)
#define MAX_SENSOR_PRESSURE 145.03773773 // maximum pressure the sensor can measure (psi)
#define MAX_GAUGE_PRESSURE 15 // max pressure to be measured by the gauge

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

// keep track of highest pressure measured
float topPressurePsi = 0;

void setup() {
  Serial.begin(9600);

  delay(250);
  display.begin(DISPLAY_ADDRESS, true);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();

  Wire.begin();

  startupDisplay();
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);

  // request 2 bytes of data from the pressure sensor
  Wire.requestFrom(SENSOR_ADDRESS, 2);
  
  if (Wire.available() >= 2) {
    // read the two bytes from the sensor
    uint16_t pressureRaw = (Wire.read() << 8) | Wire.read();
    // convert raw pressure value to its actual value using sensor's scaling
    float pressurePsi = (pressureRaw - 1638.0) / 13107.0 * (MAX_SENSOR_PRESSURE - MIN_SENSOR_PRESSURE) + MIN_SENSOR_PRESSURE;

    if(pressurePsi > topPressurePsi) {
      topPressurePsi = pressurePsi;
    }

    // draw pressure text
    display.setTextSize(3);
    int textWidth = (6 * 3) * String(pressurePsi, 1).length();
    display.setCursor(display.width() - textWidth, 0);
    display.println(pressurePsi, 1);
    display.setTextSize(1);
    display.setCursor(110, 23);
    display.println("psi");

    // draw gauge
    int gaugeWidth = (pressurePsi / MAX_GAUGE_PRESSURE) * 124;
    int topPressureGaugeX = 2 + (topPressurePsi / MAX_GAUGE_PRESSURE)  * 124;
    drawMarkers();
    display.drawRect(0, 39, 128, 25, SH110X_WHITE);
    display.fillRect(2, 41, gaugeWidth, 21, SH110X_WHITE);
    display.drawFastVLine(topPressureGaugeX , 41, 21, SH110X_WHITE);
  } else {
    display.setCursor(0, 15);
    display.println("SENSOR ERROR");
  }

  display.display();
  delay(33);
}

void startupDisplay() {
  display.setCursor(0, 0);
  display.println("Starting.");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Starting..");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void drawMarkers() {
  float currentLineX = 2; // X position of cursor for the current marker
  int currentMarker = 0; // 0psi, 1psi, 2psi, ...
  int lineXint = int(currentLineX); // X position of cursor as float
  float spacing = (124 - 1) / float(MAX_GAUGE_PRESSURE); // spacing between markers

  while(lineXint <= 125){
    // markers should be 2px tall with a taller marker every 5psi
    int markerHeight = 2;
    if(currentMarker%5 == 0){
      markerHeight = 5;
    }

    display.drawFastVLine(lineXint, 38-markerHeight, markerHeight, SH110X_WHITE);
    currentLineX += spacing;
    lineXint = int(currentLineX);
    currentMarker++;
  }
}
