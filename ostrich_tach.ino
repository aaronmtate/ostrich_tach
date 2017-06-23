// diyAudio Arduino LCD Tachometer with Serial Output ("Ostrich")
//
// A turntable tachometer sketch, including integration with Phoenix Engineering's
// Falcon or Eagle power supplies. External communication is through the default
// serial port connection (D0, D1, though only D1 (TX) is actually used). PSU
// integration requires a RS232 shield or custom converter. Connection to PSU is
// with a 3.5mm (headphone jack) TRS connector, with TX port from DB9 connector
// attached to the Ring connector (red wire) on the TRS jack. Thanks to Pyramid
// (Phoenix Eng) for assist and communication specs.
// Base tachometer functionality based on design from diyAudio user tauro0221.

#include <LiquidCrystal.h>

#define SENSOR_PIN 3 // Interrupt input pin for sensor on Arduino UNO MUST be pin 3.

// Initialize the LCD library with the numbers of the interface pins.
LiquidCrystal lcd(12, 11, 6, 5, 4, 2);

// Global Variables
volatile unsigned long last_pulse_time = 1; // Timestamp of latest interrupt pulse from sensor.
volatile bool stale_display = false;

unsigned long prev_pulse_time = 0; // Timestamp of previous interrupt pulse from sensor.
byte cycle_count = 2;

void setup() {
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);  // Set cursor at first character, first line.
  lcd.print("diyAudio Ostrich");

  pinMode(SENSOR_PIN, INPUT_PULLUP); // Sensor setup
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), sensor_pulse, FALLING);

  Serial.begin(9600);  // Open the serial port at 9600 bps
}

void loop() {
  // Show waiting message after 5 seconds of no new pulses, but only if pulses were detected.
  if (stale_display) {
    calculate_rpm();
  }
  else if (micros() - last_pulse_time > 5000000 && cycle_count > 0) {
    lcd.setCursor(0, 1);  // Set cursor at first character, second line.
    lcd.print(" Awaiting Signal");
    cycle_count = 0;
  }
  // delay(100);
}

// Sensor pulse completion interrupt callback handler. Keep this as lightweight as possible.
void sensor_pulse() {
  last_pulse_time = micros();
  stale_display = true;
}

void calculate_rpm() {
  stale_display = false;

  if (cycle_count < 2) {
    // Build up a couple of cycles to limit poor reporting.
    cycle_count++;
    lcd.setCursor(0, 1);
    lcd.print("  RPM:          "); // Clear the Awaiting Signal message, prep for RPM update
  } else {
    // Avoid DIV0, just in case.
    float rpm = (last_pulse_time > prev_pulse_time) ? 60000000.0/(last_pulse_time - prev_pulse_time) : 0;
    lcd.setCursor(7, 1);
    lcd.print(rpm, 4);

    // Send update to Phoenix PSU via serial port (D0 and D1) in format XX.XXX[lf][cr]
    if ((10.0 <= rpm) && (rpm < 100.0)) {
      char outstr[8];
      dtostrf(rpm, 6, 3, outstr);
      outstr[6] = '\n';
      outstr[7] = '\r';

      Serial.print(outstr);
    }
  }
  prev_pulse_time = last_pulse_time;
}
