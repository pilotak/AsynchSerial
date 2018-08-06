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

#include "mbed.h"
#include "AsynchSerial.h"

AsynchSerial::AsynchSerial(PinName tx, PinName rx, uint32_t baud):
    _serial(tx, rx, baud),
    _baud(baud),
    _bits(8),
    _parity(SerialBase::None),
    _stop_bits(1),
    _read_flag(false),
    _extra_timeout(0) {
    _timeout = get_timeout(baud);
    _serial.set_blocking(false);
}

void AsynchSerial::baud(uint32_t baud) {
    _baud = baud;
    _timeout = get_timeout(baud);
    _serial.set_baud(baud);
}

void AsynchSerial::format(uint8_t bits, SerialBase::Parity parity, uint8_t stop_bits) {
    _bits = bits;
    _parity = parity;
    _stop_bits = stop_bits;
}

#if DEVICE_SERIAL_FC
void AsynchSerial::set_flow_control(SerialBase::Flow type, PinName flow1, PinName flow2) {
    _serial.set_flow_control(type, flow1, flow2);
}
#endif

void AsynchSerial::init(uint8_t extra_timeout) {
    _extra_timeout = extra_timeout;
    _serial.sigio(callback(this, &AsynchSerial::rxCb));
}

void AsynchSerial::attach(Callback<void()> cb, Callback_type type) {
    _cb[type] = cb;
}

uint32_t AsynchSerial::get_timeout(uint32_t baud) {
    uint32_t timeout = (((1 + _bits + _parity + _stop_bits) * 1000000) / baud);  // get us
    timeout = (timeout + 1000 - (timeout % 1000)) / 1000;  // round up and convert to ms

    if (timeout < 1) {  // timeout needs to be at least 1ms
        timeout = 1;
    }

    timeout += _extra_timeout;

    return timeout;
}

void AsynchSerial::rxCb() {
    if ((_serial.poll(POLLIN) & POLLIN) && !_read_flag) {
        if (_cb[RX]) {
            _read_flag = true;
            _cb[RX].call();
        }
    }
}

int16_t AsynchSerial::getc() {
    pollfh fhs;
    fhs.fh = &_serial;
    fhs.events = POLLIN;
    int32_t count = poll(&fhs, 1, _timeout);

    if (count > 0 && (fhs.revents & POLLIN)) {
        unsigned char ch;
        return _serial.read(&ch, 1) == 1 ? ch : -1;

    } else {
        return -1;
    }
}

int16_t AsynchSerial::putc(unsigned char c) {
    pollfh fhs;
    fhs.fh = &_serial;
    fhs.events = POLLOUT;

    int32_t count = poll(&fhs, 1, _timeout);

    if (count > 0 && (fhs.revents & POLLOUT)) {
        return _serial.write(&c, 1) == 1 ? 0 : -1;

    } else {
        return -1;
    }
}

int16_t AsynchSerial::read(unsigned char *data, int16_t size) {
    int16_t i = 0;

    if (data) {
        for (; i < size; i++) {
            int16_t c = getc();

            if (c < 0) {  // end of reading
                if (i > 0) {  // got data
                    break;

                } else {  // no data read
                    i = -1;
                }

            } else {
                data[i] = c;
            }
        }

        if (i == size) {  // in case buffer is shorter than received data
            flush();
        }

    } else {
        i = -2;
    }

    _read_flag = false;

    return i;
}

int16_t AsynchSerial::write(const unsigned char *data, int16_t size) {
    int16_t i = 0;

    if (data) {
        for (; i < size; i++) {
            if (putc(data[i]) < 0) {
                i = -1;
                break;
            }
        }

    } else {
        i = -2;
    }

    if (_cb[TX]) {
        _cb[TX].call();
    }

    return i;
}

void AsynchSerial::flush() {
    int16_t i = getc();

    while (i > 0) {
        i = getc();
    }
}
