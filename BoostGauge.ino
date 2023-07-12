#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define DISPLAY_ADDRESS 0x3C
#define SENSOR_ADDRESS  0x28

#define MIN_SENSOR_PRESSURE 0.0 // minimum pressure the sensor can measure (psi)
#define MAX_SENSOR_PRESSURE 145.03773773 // maximum pressure the sensor can measure (psi)
#define MAX_GAUGE_PRESSURE 20 // max pressure to be measured by the gauge

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

// keep track of highest pressure measured
float topPressurePsi = 0;

void setup() {
  Serial.begin(9600);

  Wire.begin();

  delay(250);
  
  display.begin(DISPLAY_ADDRESS, true);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();

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

    updateMaxPressure(pressurePsi);
    drawGauge(pressurePsi);
  } else {
    display.setCursor(0, 15);
    display.println("SENSOR ERROR");
  }

  display.display();
  delay(33);
}

void startupDisplay() {
  float fakePressure = 0.0; // fake pressure to send to drawGauge function
  float step = 4.0f * float(MAX_GAUGE_PRESSURE) / 124; // calculate how much to increment the fake pressure to fill one more pixel in gauge
  while (fakePressure <= MAX_GAUGE_PRESSURE){ // fill gauge
    display.clearDisplay();
    updateMaxPressure(fakePressure);
    drawGauge(fakePressure);
    display.display();
    fakePressure += step;
  }
  while (fakePressure > 0){ // empty gauge
    display.clearDisplay();
    drawGauge(fakePressure);
    display.display();
    fakePressure -= step;
  }
  topPressurePsi = 0; // reset topPressurePsi
  display.clearDisplay();
}

void updateMaxPressure(float pressure) {
  if(pressure > topPressurePsi) {
    topPressurePsi = pressure;
  }
}

void drawGauge (float pressure) {
  int gaugeWidth = (pressure / MAX_GAUGE_PRESSURE) * 124; // calculate width of gauge 'progress' bar
  int topPressureGaugeX = 2 + (topPressurePsi / MAX_GAUGE_PRESSURE)  * 124; // calculate x for the max boost line

  float currentLineX = 2; // X position of cursor for the current marker
  int currentMarker = 0; // 0psi, 1psi, 2psi, ...
  int lineXint = int(currentLineX); // X position of cursor as int
  float spacing = (124 - 1) / float(MAX_GAUGE_PRESSURE); // spacing between markers

  // make sure gauge and top pressure line dont go past bar
  if( gaugeWidth > 125 ) {
    gaugeWidth = 125;
  }
  if( topPressureGaugeX > 125 ) {
    topPressureGaugeX = 125;
  }

  display.setTextSize(3);
  int textWidth = (6 * 3) * String(pressure, 1).length(); // calculate width of text (width of text * text size)
  display.setCursor(display.width() - textWidth, 0); // set cursor with calculated width so text is right-aligned
  display.println(pressure, 1); // display pressure
  display.setTextSize(1);
  display.setCursor(110, 23);
  display.println("psi"); // display unit of measurement

  display.drawRect(0, 39, 128, 25, SH110X_WHITE); // outer rectangle of gauge
  display.fillRect(2, 41, gaugeWidth, 21, SH110X_WHITE); // gauge 'progress' bar
  display.drawFastVLine(topPressureGaugeX , 41, 21, SH110X_WHITE); // max boost line

  while(lineXint <= 125){ // 125 - screen is 128 pixels wide, x starts at 0, gauge is 2 pixels from right edge
    int markerHeight = 2; // markers are 2px tall with a taller marker every 5psi
    if(currentMarker%5 == 0){ // check if marker is multiple of 5
      markerHeight = 5;
    }

    display.drawFastVLine(lineXint, 38-markerHeight, markerHeight, SH110X_WHITE); // tick marker
    currentLineX += spacing; // set x for next marker (float)
    lineXint = int(currentLineX); // convert x to int
    currentMarker++; // increment marker counter
  }
}