/*
 * Driver for a pair of sensor counter subsystems proof of concept build. Counts
 * are uploaded to a host computer via a USB serial connection at BAUD (see below)
 * bits per second. You can use the Arduino SDK's Serial Monitor to retrieve the
 * output, or redirect the output to a file for later processing.
 */

#include <SPI.h>
#define SPI_RATE 3500000

#define BAUD 115200
 
#define OUT_GAL_NOT 2  // Show lower byte of counter A (least significant)
#define OUT_GAU_NOT 3  // Show upper byte of counter A
#define OUT_GBL_NOT 4  // Show lower byte of counter B (most significant)
#define OUT_GBU_NOT 5  // Show upper byte of counter B
#define OUT_PAR_IN_SER_OUT_SHIFT_HI_LOAD_LO 6
#define OUT_RCLK_A 7
#define UNUSED_8 8
#define OUT_START_SYNC 9
#define NOT_USED_SLAVE_SELECT 10
#define NOT_USED_SPI_MOSI 11
#define IN_SPI_MISO 12
#define OUT_SPI_CLK 13
#define OUT_ENABLE_A_NOT A0
#define OUT_ENABLE_B_NOT A1
#define OUT_COUNTER_ENABLE_NOT A2
#define UNUSED_A3 A3
#define OUT_CCLR_NOT A4
#define UNUSED_A5 A5

#define SHORT_PULSE_WIDTH_MICROSECONDS 5   // Nominal short pulse with in microseconds
#define LONG_PULSE_WIDTH_MICROSECONDS 100  // Nominal long pulse width in microseconds
#define EXPOSURE_TIME_MICROSECONDS 2000    // Nominal light exposure time in microseconds

const uint8_t SHIFT_IN_ENABLE[] PROGMEM = {
  OUT_GBU_NOT,
  OUT_GBL_NOT,
  OUT_GAU_NOT,
  OUT_GAL_NOT,
};

const uint32_t TEST_VALUES[] PROGMEM = {
  0x0,
  0x1,
  0x7,
  0x1F,
  0xF00F,
  0x1234,
  0xFFFF,
  0x8000,
  0x01234567
};

#define TEST_VALUE_COUNT 9

uint8_t value_index = 0;

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {};

  pinMode(OUT_GAL_NOT, OUTPUT);
  pinMode(OUT_GAU_NOT, OUTPUT);
  pinMode(OUT_GBL_NOT, OUTPUT);
  pinMode(OUT_GBU_NOT, OUTPUT);
  pinMode(OUT_PAR_IN_SER_OUT_SHIFT_HI_LOAD_LO, OUTPUT);
  pinMode(OUT_RCLK_A, OUTPUT);
  pinMode(OUT_START_SYNC, OUTPUT);
  pinMode(NOT_USED_SLAVE_SELECT, OUTPUT);
  pinMode(NOT_USED_SPI_MOSI, OUTPUT);
  pinMode(IN_SPI_MISO, INPUT);
  pinMode(OUT_SPI_CLK, OUTPUT);
  pinMode(OUT_ENABLE_A_NOT, OUTPUT);
  pinMode(OUT_ENABLE_B_NOT, OUTPUT);
  pinMode(OUT_COUNTER_ENABLE_NOT, OUTPUT);

  pinMode(OUT_CCLR_NOT, OUTPUT);

  digitalWrite(OUT_GAL_NOT, HIGH);
  digitalWrite(OUT_GAU_NOT, HIGH);
  digitalWrite(OUT_GBL_NOT, HIGH);
  digitalWrite(OUT_GBU_NOT, HIGH);

  digitalWrite(OUT_PAR_IN_SER_OUT_SHIFT_HI_LOAD_LO, HIGH);
  digitalWrite(OUT_RCLK_A, LOW);
  digitalWrite(OUT_START_SYNC, LOW);

  digitalWrite(NOT_USED_SLAVE_SELECT, LOW);
  digitalWrite(NOT_USED_SPI_MOSI, LOW);
  digitalWrite(OUT_SPI_CLK, LOW);

  digitalWrite(OUT_ENABLE_A_NOT, HIGH);
  digitalWrite(OUT_ENABLE_B_NOT, HIGH);

  digitalWrite(OUT_COUNTER_ENABLE_NOT, HIGH);

  digitalWrite(OUT_CCLR_NOT, HIGH);

  clear_counters();
  
  SPI.begin();

  Serial.println("Ready");
}

/*
 * Emit the shortest possible descending pulse on the specified pin.
 *
 * Preconditiions:
 * --------------
 * The specified pin must be configured for output and must be HIGH.
 *
 * Name                      Type       Contents
 * ------------------------- ---------- -----------------------------------------------
 * pin                       uint8_t    Pin number. See microcontroller documentation for
 *                                      details.
 */
void descending_pulse(uint8_t pin) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
}

/*
 * Emit a short descending pulse on the specified pin. The nominal pulse width
 * is set by SHORT_PULSE_WIDTH_MICROSECONDS
 *
 * Preconditiions:
 * --------------
 * The specified pin must be configured for output and must be HIGH.
 *
 * Name                      Type       Contents
 * ------------------------- ---------- -----------------------------------------------
 * pin                       uint8_t    Pin number. See microcontroller documentation for
 *                                      details.
 */
void short_descending_pulse(uint8_t pin) {
  digitalWrite(pin, LOW);
  delayMicroseconds(SHORT_PULSE_WIDTH_MICROSECONDS);
  digitalWrite(pin, HIGH);
}

/*
 * Emit a long descending pulse on the specified pin. The nominal pulse width
 * is set by LONG_PULSE_WIDTH_MICROSECONDS
 *
 * Preconditiions:
 * --------------
 * The specified pin must be configured for output and must be HIGH.
 *
 * Name                      Type       Contents
 * ------------------------- ---------- -----------------------------------------------
 * pin                       uint8_t    Pin number. See microcontroller documentation for
 *                                      details.
 */
void long_descending_pulse(uint8_t pin) {
  digitalWrite(pin, LOW);
  delayMicroseconds(LONG_PULSE_WIDTH_MICROSECONDS);
  digitalWrite(pin, HIGH);
}

/*
 * Emit the shortest possible ascending pulse on the specified pin.
 *
 * Preconditiions:
 * --------------
 * The specified pin must be configured for output and must be LOW.
 *
 * Name                      Type       Contents
 * ------------------------- ---------- -----------------------------------------------
 * pin                       uint8_t    Pin number. See microcontroller documentation for
 *                                      details.
 */
void rising_pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

/*
 * Emit a short ascending pulse on the specified pin. The nominal pulse width
 * is set by SHORT_PULSE_WIDTH_MICROSECONDS
 *
 * Preconditiions:
 * --------------
 * The specified pin must be configured for output and must be LOW.
 *
 * Name                      Type       Contents
 * ------------------------- ---------- -----------------------------------------------
 * pin                       uint8_t    Pin number. See microcontroller documentation for
 *                                      details.
 */
void short_rising_pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(SHORT_PULSE_WIDTH_MICROSECONDS);
  digitalWrite(pin, LOW);
}

/*
 * Emit a long ascending pulse on the specified pin. The nominal pulse width
 * is set by LONG_PULSE_WIDTH_MICROSECONDS
 *
 * Preconditiions:
 * --------------
 * The specified pin must be configured for output and must be LOW.
 *
 * Name                      Type       Contents
 * ------------------------- ---------- -----------------------------------------------
 * pin                       uint8_t    Pin number. See microcontroller documentation for
 *                                      details.
 */
void long_rising_pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(LONG_PULSE_WIDTH_MICROSECONDS);
  digitalWrite(pin, LOW);
}

/*
 * Read the specified byte from the enabled counter.
 * 
 * Preconditiions:
 * --------------
 * The specified counter must be enabled; i.e. the enable pin on its associated 74xx125 must
 * be LOW..
 *
 * Name                      Type       Contents
 * ------------------------- ---------- -----------------------------------------------
 * pin                       uint8_t    The counter pin whose lowering will put the byte
 *                                      value on the output bus.
 */
uint8_t read_byte(uint8_t enable_pin) {
  digitalWrite(enable_pin, LOW);  // Count onto the data bus
  short_descending_pulse(OUT_PAR_IN_SER_OUT_SHIFT_HI_LOAD_LO);  // Data bus --> par in/ser out
  SPI.beginTransaction(SPISettings(SPI_RATE, MSBFIRST, SPI_MODE0));
  uint8_t shifted_in_byte = SPI.transfer((uint8_t)0);
  SPI.endTransaction();
  digitalWrite(enable_pin, HIGH); // Counter to float on data bus
  return shifted_in_byte;
}

uint32_t read_counter(uint8_t counter_enable_pin) {
  uint32_t value = 0;
  digitalWrite(counter_enable_pin, LOW);
 
  for (uint8_t i = 0; i < 4; ++i) {
     value <<= 8;
     value &= 0xFFFFFF00;
     uint8_t shift_in_enable_index = pgm_read_byte(SHIFT_IN_ENABLE + i);
     value |= (uint8_t) read_byte(shift_in_enable_index); 
  }

  digitalWrite(counter_enable_pin, HIGH);
  return value;
}

void clear_counters() {
  long_descending_pulse(OUT_CCLR_NOT);
  rising_pulse(OUT_RCLK_A);
}

void loop() {
  uint8_t next_index = (value_index + 1) % TEST_VALUE_COUNT;
  clear_counters();

  // Enable the sensors for the exposure time.
  digitalWrite(OUT_COUNTER_ENABLE_NOT, LOW);
  delayMicroseconds(EXPOSURE_TIME_MICROSECONDS);
  digitalWrite(OUT_COUNTER_ENABLE_NOT, HIGH);

  rising_pulse(OUT_RCLK_A);

  Serial.print("Read A: ");
  short_rising_pulse(OUT_START_SYNC);
  uint32_t count_a = read_counter(OUT_ENABLE_A_NOT);
  uint32_t count_b = read_counter(OUT_ENABLE_B_NOT);
  Serial.print(count_a, HEX);
  Serial.print(", B: ");
  Serial.println(count_b, HEX);
  
  value_index = next_index;
}
