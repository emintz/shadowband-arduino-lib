/*
    An Arduino library that supports interleaved serial writes
    Copyright (C) 2016  The Winer Observatory, www.winer.org.

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

#ifndef __ASYNCH_WRITE_H
#define __ASYNCH_WRITE_H

#include "Arduino.h"

class AsyncWrite {
  /*
   * States that the writer can assume.
   */
public:
  enum State {
    CREATED,    // Created but not initialized
    IDLE,       // Not busy and ready to write.
    SENDING,    // Sending a string.
    DONE,       // Done sending
  };

  private:
    State state;
    const char * pbuffer;
    uint8_t length;
    uint8_t sent;

  public:
    /*
     * Coustructs an AsyncWrite instance, The instance will be in the
     * CREATED state, and cannot be used until the desired baud rate is
     * set via a call to init().
     */
    AsyncWrite();

    /*
     * Sets the desired baud rate and prepares the instance to send
     * byte-oriented serial data.
     *
     *  Parameters
     *  ----------
     *  baud   the desired output speed in bits per second. There are
     *         standard speeds like 300, 1200, 9600, etc., but any
     *         value supported by the Arduino Serial library can be used.
     */
    void init(long baud);

    /*
     * Enqueues the specified byte string for output.
     *
     *  Parameters:
     *  ----------
     *
     *  length        the number of bytes to send, in the range 
     *                [0 .. 511].
     *  user_pbuffer  Pointer to the byte array to be sent.
     *
     *  Returns:
     *  -------
     *
     *  true  if and only if the instance accepted new output for
     *        transmission
     *  false otherwise
     */
    bool send(uint8_t length, const char *user_pbuffer);

    /*
     * Perform the next transmission step. Call this method at the
     * beginning of the loop() method.
     *
     *  Returns:
     *  -------
     *
     *  true  if the instance is ready to accept the next buffer
     *  false otherwise
     */
    bool process();

    State getState() {
      return state;
    }

};

#endif


