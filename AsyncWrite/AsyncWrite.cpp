/*
    Arduino library that supports overlapped serial output
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

#include "AsyncWrite.h"

AsyncWrite::AsyncWrite() {
  state = CREATED;
  pbuffer = 0;
  length = 0;
  sent = 0;
}

void AsyncWrite::init(long baud) {
  Serial.begin(baud);
  while(!Serial) {}
  state = IDLE;
}


bool AsyncWrite::send(uint8_t user_length, const char *user_pbuffer) {
  if (state == DONE || state == IDLE) {
     pbuffer = user_pbuffer;
     length = user_length;
     sent = 0;
     state = SENDING;
      return true; 
    }
  return false;
}

bool AsyncWrite::process() {
  switch (state) {
    case CREATED:
      return false;

    case IDLE:
      return true;

    case SENDING:
      if (sent == length) {
        pbuffer = 0;
        length = 0;
        sent = 0;
        state = DONE;
      } else {
        if (0 < Serial.availableForWrite()) {
          Serial.write(pbuffer[sent]);
          ++sent;
        }
      }
      return false;

    case DONE:
      return true;
   }
}
