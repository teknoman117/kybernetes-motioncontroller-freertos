#include "FreeRTOS.h"
#include "task.h"

#include "console.h"
#include "uart.h"

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

void main(void) {
    setupLED();
    setupUART0();
    setupConsole();

    xTaskCreate(TaskHeartbeat, "Blink", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    vTaskStartScheduler();
}
