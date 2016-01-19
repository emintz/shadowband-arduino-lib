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

#ifndef __OVERLAPPED_ANALOG_READ_H
#define __OVERLAPPED_ANALOG_READ_H

#include <avr/io.h>
#include "Arduino.h"
#include "wiring_private.h"
#include "pins_arduino.h"

class OverlappedAnalogRead {
  public:
    enum State {
      IDLE,
      CONVERTING,
      DONE,
    };

  private:
    State state;
    uint8_t analog_reference;

  public:
    OverlappedAnalogRead(uint8_t mode = DEFAULT);
    void startConvertingPin(uint8_t pin);
    void checkConversion();
    uint16_t readAnalogValue();

    State get_state() {
      return state;
   }
};

#endif // __OVERLAPPED_ANALOG_READ_H

