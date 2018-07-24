# AsynchSerial
Wrapper around mbed UARTSerial with added timeouts

```cpp
#include "mbed.h"
#include "AsynchSerial.h"

AsynchSerial serial(PD_8, PD_9, 9600);
EventQueue queue;

void getData() {
    char buffer[20];
    int16_t len = serial.read(buffer, sizeof(buffer));

    if (len > 0) {
        printf("len:%i\n", len);

        for (int16_t i = 0; i < len; ++i) {
            printf("%02X ", buffer[i]);
        }

        printf("\n");

        // Echo back what we received
        serial.write(buffer, len);
    }
}

void rxCb() {
    queue.call(getData);  // can't read data in callback
}

void txCb() {
    printf("sent\n");
}

int main() {
    Thread eventThread;
    eventThread.start(callback(&queue, &EventQueue::dispatch_forever));

    serial.attach(callback(rxCb), AsynchSerial::RX);
    serial.attach(callback(txCb), AsynchSerial::TX);
    serial.init();

    while (1) {
    }
}

```
