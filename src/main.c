#include "FreeRTOS.h"
#include "task.h"

#include "console.h"
#include "i2c.h"
#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <stdio.h>

void vApplicationMallocFailedHook(void) {
    while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t task, char*) {
    while (1);
}

void setupLED(void) {
    DDRB |= _BV(PORTB5);
    PORTB &= ~_BV(PORTB5);
}

void setLEDOn(void) {
    PORTB |= _BV(PORTB5);
}

void setLEDOff(void) {
    PORTB &= ~_BV(PORTB5);
}

void TaskHeartbeat(void *pvParameters) {
    TickType_t wakeUpTime = xTaskGetTickCount();
    while (1) {
        setLEDOn();
        CONSOLE(printf_P(PSTR("LED On\r\n")));
        vTaskDelayUntil(&wakeUpTime, 500 / portTICK_PERIOD_MS);
        setLEDOff();
        CONSOLE(printf_P(PSTR("LED Off\r\n")));
        vTaskDelayUntil(&wakeUpTime, 500 / portTICK_PERIOD_MS);
    }
}

void TaskSonars(void *pvParameters) {
    // get software revision command
    uint8_t commandRegister = 0;
    uint8_t softwareRevision = 0;

    i2c_msg_t getSoftwareRevisionMsgs[] = {
        {
            .data = &commandRegister,
            .len = sizeof commandRegister - 1,
            .address = 0xE2,
        },
        {
            .data = &softwareRevision,
            .len = sizeof softwareRevision - 1,
            .address = 0xE3,
        },
    };

    i2c_transfer_t getSoftwareRevision = {
        .notify = xTaskGetCurrentTaskHandle(),
        .msgs = getSoftwareRevisionMsgs,
        .n = 1,
        .error = 0,
    };

    while (1) {
        getSoftwareRevision.error = 0;

        // submit get transfer
        queueI2CTransfer(&getSoftwareRevision);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // print out the software revision
        if (!getSoftwareRevision.error) {
            CONSOLE(printf_P(PSTR("sonar @ E2h software revision = %u\r\n"), softwareRevision));
            softwareRevision = 0;
        }
    }
}

void main(void) {
    setupLED();
    setupI2C();
    setupUART0();
    setupConsole();

    xTaskCreate(TaskHeartbeat, "Blink", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(TaskSonars, "Sonars", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    vTaskStartScheduler();
}
