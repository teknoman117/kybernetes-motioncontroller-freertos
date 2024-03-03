#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include "console.h"

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
        //CONSOLE(printf_P(PSTR("[i2c] starting transfer. %u phases.\r\n"), transfer->n + 1));

        uint8_t n = 0;
        transfer->error = 0;
        do {
            i2c_msg_t *msg = &transfer->msgs[n];
            //CONSOLE(printf_P(PSTR("[i2c] starting phase %u.\r\n"), n));
            
            // send start or repeated start
            TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
            while (!(TWCR & _BV(TWINT)))
                portYIELD();

            // state machine
            uint8_t i = 0;
            uint8_t complete = 0;
            do {
                //CONSOLE(printf_P(PSTR("[i2c] twsr: %02x.\r\n"), TWSR & 0xF8));
                uint8_t control = _BV(TWINT) | _BV(TWEN);
                switch (TWSR & 0xF8) {
                    case 0x08:
                        // START
                        /* FALLTHROUGH */
                    case 0x10:
                        // REPEATED-START
                        // load slave address
                        TWDR = msg->address;
                        break;

                    case 0x18:
                        // SLA+W transmitted, ACK received
                        /* FALLTHROUGH */
                    case 0x28:
                        // Data transmitted, ACK received
                        TWDR = msg->data[i];
                        if (i++ == msg->len)
                            complete = 1;
                        break;

                    case 0x20:
                        // SLA+W transmitted, NACK received
                        /* FALLTHROUGH */
                    case 0x30:
                        // Data transmitted, NACK received
                        transfer->error = 1;
                        complete = 1;
                        control = 0;
                        break;

                    case 0x38:
                        // Arbitration loss
                        transfer->error = 1;
                        complete = 1;
                        control = 0;
                        break;

                    case 0x40:
                        // SLA+R transmitted, ACK received
                        if (i != msg->len) {
                            control |= _BV(TWEA);
                        } 
                        break;
                    case 0x48:
                        // SLA+R transmitted, NACK received
                        transfer->error = 1;
                        complete = 1;
                        control = 0;
                        break;
                    case 0x50:
                        // Data received, ACK transmitted
                        msg->data[i++] = TWDR;
                        if (i != msg->len) {
                            control |= _BV(TWEA);
                        }
                        break;

                    case 0x58:
                        // Data received, NACK transmitted
                        msg->data[i++] = TWDR;
                        complete = 1;
                        control = 0;
                        break;
                    default:
                        transfer->error = 1;
                        complete = 1;
                        control = 0;
                        break;
                }

                // write the control register if requested
                if (control) {
                    TWCR = control;
                    while (!(TWCR & _BV(TWINT)))
                        portYIELD();
                }
            } while (!complete);
        } while ((n++ != transfer->n) && !transfer->error);

        // send stop
        TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

        // notify calling task
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
