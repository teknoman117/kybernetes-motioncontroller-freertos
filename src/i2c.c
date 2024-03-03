#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static TaskHandle_t uI2CTaskHandle;
static QueueHandle_t uTransferQueue;

static i2c_msg_t *currentMsg;
static uint8_t currentByte;

ISR(TWI_vect) {
    /*BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(uI2CTask, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }*/
}

void I2CTask(void *pvParameters) {
    while (1) {
        // Get the next transfer from the queue
        i2c_transfer_t *transfer = NULL;
        while (!xQueueReceive(uTransferQueue, &transfer, portMAX_DELAY));

        uint8_t n = 0;
        do {
            i2c_msg_t *msg = &transfer->msgs[n];
            uint8_t dir = msg->address & 0x01;
            
            // send start or repeated start
            TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
            while (!(TWCR & _BV(TWINT)))
                portYIELD();

            // if status isn't start, abort
            if (TWSR & 0xF8 != 0x08 && TWSR & 0xF8 != 0x10) {
                transfer->error = 1;
                goto i2c_task_error;
            }
            
            // send address
            TWDR = msg->address;
            TWCR = _BV(TWINT) | _BV(TWEN);
            while (!(TWCR & _BV(TWINT)))
                portYIELD();

            if (TWSR & 0xF8 == 0x20 || TWSR & 0xF8 == 0x38 || TWSR & 0xF8 == 0x48) {
                transfer->error = 1;
                break;
            }

            // transfer data
            uint8_t i = 0;
            do {
                if (dir) {
                    // read
                    if (i != msg->len) {
                        // normal byte (send ack)
                        TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
                    } else {
                        // last byte (send nack)
                        TWCR = _BV(TWINT) | _BV(TWEN);
                    }
                } else {
                    TWDR = msg->data[i];
                    TWCR = _BV(TWINT) | _BV(TWEN);
                }

                while (!(TWCR & _BV(TWINT)))
                    portYIELD();

                if (dir) {
                    // read
                    msg->data[i] = TWDR;
                } else {
                    // write
                    // check ack/nack
                }
            } while (i++ != msg->len);
        } while (n++ != transfer->n);

        // send stop
        TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

        // notify calling task
i2c_task_error:
        if (transfer->notify) {
            xTaskNotifyGive(transfer->notify);
        }

        // wait for stop to complete
        while (TWCR & _BV(TWSTO))
            portYIELD();
    }
}

void setupI2C(void) {
    TWBR = (F_CPU / (F_I2C * 8)) - 2;
    TWCR = _BV(TWINT) | _BV(TWEN) /*| _BV(TWIE)*/;

    uTransferQueue = xQueueCreate(16, sizeof(i2c_transfer_t*));
    xTaskCreate(I2CTask, "I2C", configMINIMAL_STACK_SIZE, NULL, 2, &uI2CTaskHandle);
}

void queueI2CTransfer(i2c_transfer_t *transfer) {
    while (!xQueueSend(uTransferQueue, &transfer, 1));
}
