/*
MIT License

Copyright (c) 2018 Pavel Slama

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ASYNCHSERIAL_H
#define ASYNCHSERIAL_H

#include "mbed.h"

class AsynchSerial {
 public:
  typedef enum {
    RX = 0,
    TX
  } Callback_type;

  AsynchSerial(PinName tx, PinName rx, uint32_t baud = MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE);
  void init();
  void attach(Callback<void()> cb, Callback_type type = RX);
  int16_t getc();
  int16_t putc(char c);
  void baud(uint32_t baud);
  void format(uint8_t bits = 8, SerialBase::Parity parity = SerialBase::None, uint8_t stop_bits = 1);
  int16_t read(char *data, int16_t size);
  int16_t write(const char *data, int16_t size);

#if DEVICE_SERIAL_FC
  void set_flow_control(SerialBase::Flow type, PinName flow1 = NC, PinName flow2 = NC);
#endif

 protected:
  uint32_t get_timeout(uint32_t baud);

 private:
  UARTSerial _serial;
  Callback<void()> _cb[2];
  void rxCb();

  uint32_t _baud;
  uint8_t _bits;
  SerialBase::Parity _parity;
  uint8_t _stop_bits;
  bool _read_rs;
  uint32_t _timeout;
};

#endif
