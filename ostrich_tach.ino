// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 6, 5, 4, 2);

// Warning you must connect the Sda to pin A4 and the Clock to pin A5

// Global Variables
unsigned long abort_time = millis();
float rpm;
volatile byte intp_flag = 0;
byte intrp_pin = 3; // Interrupt pin for Arduino UNO MUST be pin 3
volatile unsigned long start_time = 0; // start platen index pulse time value
volatile unsigned long end_time   = 0; // end platen index time value
// byte pwr_swt = 4;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);  // set cursor character position=0  line=0
  lcd.print("Starting Display");
  lcd.setCursor(0, 1);  // set cursor character position=0  line=1
  lcd.print(" diyAudio Tach  ");
  delay(5000);
  lcd.clear();

  pinMode(intrp_pin, INPUT_PULLUP); // Sensor
  // pinMode(pwr_swt, INPUT_PULLUP); // Power Switch

  // Serial.begin(9600);  // open the serial port at 9600 bps
}

void loop() {
  rpm_calc();
  disp_msg();
  delay(2500);
}

// rpm calculation
void rpm_calc() {
  float calc_time  = 0;
  interrupt_handle();  // read the start index pulse time
  calc_time = (end_time - start_time) / 1000000.0;
  rpm = (1.0 / calc_time) * 60.0;
}

// interrupt handler function
void interrupt_handle() {
  intp_flag = 0;
  abort_time = millis();
  attachInterrupt(digitalPinToInterrupt(intrp_pin), index_pulse_time, FALLING);
  do {
    // Wait for the interrupt and branch to read the index_pulse_time()
    if (millis() - abort_time > 5000) {
      pwr_off_msg();
    }
  } while (intp_flag <= 2);
  detachInterrupt(digitalPinToInterrupt(intrp_pin));
}

// display rpm function
void disp_msg() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" diyAudio Tach  ");
  lcd.setCursor(0, 1);
  lcd.print("  RPM: "); lcd.print(rpm, 4);
}

// no switch Power OFFF message
void pwr_off_msg() {
  // if (digitalRead(pwr_swt)==1){return;} // read power switch = 1 return
  lcd.clear();
  do {
    lcd.setCursor(0, 0);
    lcd.print("Platter Stopped ");
    lcd.setCursor(0, 1);
    lcd.print(" POWER OFF NOW! " );
    delay(5000);
  } while (digitalRead(intrp_pin) != 1);
}

// read time function
void index_pulse_time() {
  switch (intp_flag) {
    case 0:
      break;
    case 1:
      start_time = micros();
      break;
    case 2:
      end_time = micros();
  }
  intp_flag = intp_flag + 1;
}
