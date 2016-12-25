/*
   Direct connection to 64-bit counter
*/

#define BAUD 115200

#define OUT_COUNT_CLOCK 12

#define OUT_GAL_NOT 11
#define OUT_GAU_NOT 10
#define OUT_GBL_NOT  9
#define OUT_GBU_NOT  8

#define OUT_RCLK 7

#define OUT_CCLR_NOT 6

#define IN_Y4 2
#define IN_Y5 3
#define IN_Y6 4
#define IN_Y7 5

#define IN_Y3 13
#define IN_Y2 A0
#define IN_Y1 A1
#define IN_Y0 A2

#define OUT_TRIGGER A3

static const PROGMEM uint8_t INPUT_PINS[] = {
  IN_Y0,
  IN_Y1,
  IN_Y2,
  IN_Y3,
  IN_Y4,
  IN_Y5,
  IN_Y6,
  IN_Y7,
};

static const PROGMEM uint8_t CONTROL_PINS[] = {
  OUT_GBU_NOT,
  OUT_GBL_NOT,
  OUT_GAU_NOT,
  OUT_GAL_NOT,
};

static const PROGMEM uint32_t COUNTS[] = {
  0,
  0b00000001,
  0b00000010,
  0b00000100,
  0b00001000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b10000000,
  0x0004,
  0x0030,
  0x0200,
  0x1000,
  0x1234,
};

#define COUNTS_SIZE 14;
uint8_t count_index = 0;

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {};

  pinMode(OUT_COUNT_CLOCK, OUTPUT);
  pinMode(OUT_GAL_NOT, OUTPUT);
  pinMode(OUT_GAU_NOT, OUTPUT);
  pinMode(OUT_GBL_NOT, OUTPUT);
  pinMode(OUT_GBU_NOT, OUTPUT);
  pinMode(OUT_RCLK, OUTPUT);
  pinMode(OUT_CCLR_NOT, OUTPUT);
  pinMode(IN_Y7, INPUT);
  pinMode(IN_Y6, INPUT);
  pinMode(IN_Y5, INPUT);
  pinMode(IN_Y4, INPUT);
  pinMode(IN_Y3, INPUT);
  pinMode(IN_Y2, INPUT);
  pinMode(IN_Y1, INPUT);
  pinMode(IN_Y0, INPUT);
  pinMode(OUT_TRIGGER, OUTPUT);

  digitalWrite(OUT_COUNT_CLOCK, LOW);
  digitalWrite(OUT_CCLR_NOT, HIGH);
  digitalWrite(OUT_GAL_NOT, HIGH);
  digitalWrite(OUT_GAU_NOT, HIGH);
  digitalWrite(OUT_GBL_NOT, HIGH);
  digitalWrite(OUT_GBU_NOT, HIGH);
  digitalWrite(OUT_TRIGGER, LOW);
  Serial.println("Ready");
}

/*
   Put a descending pulse on the specified pin.

   Preconditiions:
   --------------
   The specified pin must be configured for output and must be HIGH.

   Name                      Type       Contents
   ------------------------- ---------- -----------------------------------------------
   pin                       uint8_t    Pin number. See microcontroller documentation for
                                        details.
*/
void falling_pulse(uint8_t pin) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
}

void long_falling_pulse(uint8_t pin) {
  digitalWrite(pin, LOW);
  delayMicroseconds(100);
  digitalWrite(pin, HIGH);
}

/*
   Put a ascending pulse on the specified pin.

   Preconditiions:
   --------------
   The specified pin must be configured for output and must be LOW.

   Name                      Type       Contents
   ------------------------- ---------- -----------------------------------------------
   pin                       uint8_t    Pin number. See microcontroller documentation for
                                        details.
*/
void rising_pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

void long_rising_pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(100);
  digitalWrite(pin, LOW);
}

void oscillate_pin(uint8_t pin, uint8_t count) {
  for (uint8_t i = 0; i < count; ++i) {
    delayMicroseconds(50);
    digitalWrite(pin, HIGH);
    delayMicroseconds(50);
    digitalWrite(pin, LOW);
  }
}

void fast_pulse_train(uint8_t pin, uint32_t count) {
  while (0 < count) {
    digitalWrite(pin, HIGH);
    digitalWrite(pin, LOW);
    --count;
  }
}

uint8_t read_counter_byte_value() {
  uint8_t value = 0;

  for (int pin_index = 0; pin_index < 8; ++pin_index) {
    uint8_t pin = pgm_read_byte(INPUT_PINS + pin_index);
    if (digitalRead(pin) == HIGH) {
      bitSet(value, pin_index);
    }
  }

  return value;
}

uint8_t extract_and_read_counter_byte(uint8_t control_pin) {
  digitalWrite(control_pin, LOW);
  uint8_t byte_value = read_counter_byte_value();
  digitalWrite(control_pin, HIGH);
  return byte_value;
}

uint32_t read_counter_value() {
  uint32_t value = 0;
  for (uint8_t i = 0; i < 4; ++i) {
    uint8_t byte_enable_pin = pgm_read_byte(CONTROL_PINS + i);
    value <<= 8;
    value |= extract_and_read_counter_byte(byte_enable_pin);
  }
  return value;
}

void loop() {

  // Get current test count
  uint32_t current_count = pgm_read_dword(COUNTS + count_index);

  // Clear the counter
  falling_pulse(OUT_CCLR_NOT);

  // Load the counter
  fast_pulse_train(OUT_COUNT_CLOCK, current_count);

  // Signal read start
  long_rising_pulse(OUT_TRIGGER);
  delayMicroseconds(1);

  // Load the count into the output register
  rising_pulse(OUT_RCLK);

  // Read the value from the counter
  uint32_t counter_value = read_counter_value();

  Serial.print("Wrote ");
  Serial.print(current_count, HEX);
  Serial.print(", read ");
  Serial.println(counter_value, HEX);

  count_index = (++count_index) % COUNTS_SIZE;
}
