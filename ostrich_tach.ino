#include <LiquidCrystal.h>

#define SENSOR_PIN 3 // Interrupt input pin for sensor on Arduino UNO MUST be pin 3.

// Initialize the LCD library with the numbers of the interface pins.
LiquidCrystal lcd(12, 11, 6, 5, 4, 2);

// Global Variables
volatile unsigned long prev_pulse_time = 0; // Timestamp of previous interrupt pulse from sensor.
volatile unsigned long last_pulse_time = 1; // Timestamp of latest interrupt pulse from sensor.
volatile byte cycle_count = 2;

void setup() {
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);  // Set cursor at first character, first line.
  lcd.print("diyAudio Ostrich");

  pinMode(SENSOR_PIN, INPUT_PULLUP); // Sensor setup
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), calculate_rpm, FALLING);

  Serial.begin(9600);  // Open the serial port at 9600 bps
}

void loop() {
  // Show waiting message after 5 seconds of no new pulses, but only if pulses were detected.
  if (micros() - last_pulse_time > 5000000 && cycle_count > 0) {
    lcd.setCursor(0, 1);  // Set cursor at first character, second line.
    lcd.print(" Awaiting Signal");
    cycle_count = 0;
  }
  delay(1000);
}

// Sensor pulse completion interrupt callback handler
void calculate_rpm() {
  prev_pulse_time = last_pulse_time;
  last_pulse_time = micros();

  lcd.setCursor(0, 1);

  if (cycle_count < 2) {
    // Build up a couple of cycles to limit poor reporting.
    cycle_count++;
  } else {
    // Avoid DIV0, just in case.
    float rpm = (last_pulse_time > prev_pulse_time) ? 60000000.0/(last_pulse_time - prev_pulse_time) : 0;
    lcd.print("  RPM: ");
    lcd.print(rpm, 4);
    lcd.print("  ");

    // Send update to Phoenix PSU via serial port (D0 and D1) in format XX.XXX[lf][cr]
    if ((10.0 <= rpm) && (rpm < 100)) {
      char outstr[6];
      dtostrf(rpm, 6, 3, outstr);
      Serial.print(outstr);
      Serial.print("\n\r");
    }
  }
}
