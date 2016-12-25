#include "AsyncWrite.h"

#define BAUD 230400

const char *pHello = "Hello, ";
const char *pWorld = "World!";
const char *pEndl = "\n";

const char *pBuffer[] = {pHello, pWorld, pEndl};
const uint8_t string_length[] = {7, 6, 1};

uint8_t index = 0;

AsyncWrite asyncWrite;

void setup() {
  asyncWrite.init(BAUD);
}

void loop() {
  if (asyncWrite.process()) {
    asyncWrite.send(string_length[index], pBuffer[index]);
    index = (++index) % 3;
  }
}
