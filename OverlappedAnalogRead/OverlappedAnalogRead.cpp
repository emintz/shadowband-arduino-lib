/*
    Arduino library that supports overlapped analog to digital conversion
    Copyright (C) 2016 The Winer Observatory, http://www.winer.org

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "OverlappedAnalogRead.h"

OverlappedAnalogRead::OverlappedAnalogRead(uint8_t mode) {
  analog_reference = mode;
  state = IDLE;
}

void OverlappedAnalogRead::startConvertingPin(uint8_t pin) {

#if defined(analogPinToChannel)
#if defined(__AVR_ATmega32U4__)
  if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#endif
  pin = analogPinToChannel(pin);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if (pin >= 54) pin -= 54; // allow for channel or pin numbers
#elif defined(__AVR_ATmega32U4__)
  if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
  if (pin >= 24) pin -= 24; // allow for channel or pin numbers
#else
  if (pin >= 14) pin -= 14; // allow for channel or pin numbers
#endif

#if defined(ADCSRB) && defined(MUX5)
  // the MUX5 bit of ADCSRB selects whether we're reading from channels
  // 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
  ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#endif
  
  // set the analog reference (high two bits of ADMUX) and select the
  // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
  // to 0 (the default).
  ADMUX = (analog_reference << 6) | (pin & 0x07);

  // start the conversion
  sbi(ADCSRA, ADSC);
  state = CONVERTING;
}

void OverlappedAnalogRead::checkConversion() {
  // ADSC is cleared when the conversion finishes
  if (state == CONVERTING && bit_is_set(ADCSRA, ADSC)) {
    state = DONE;
  }
}

uint16_t  OverlappedAnalogRead::readAnalogValue() {
  // we have to read ADCL first; doing so locks both ADCL
  // and ADCH until ADCH is read.  reading ADCL second would
  // cause the results of each conversion to be discarded,
  // as ADCL and ADCH would be locked when it completed.
  uint8_t low  = ADCL;
  uint8_t high = ADCH;

  state = IDLE;
  // combine the two bytes
  return (high << 8) | low;
}
