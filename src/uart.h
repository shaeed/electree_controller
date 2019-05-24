/*

UART_MQTT HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef UART_H
#define UART_H

#include <Arduino.h>
#include <string.h>
#include "debug.h"

#ifndef UART_USE_SOFT
#define UART_USE_SOFT          0           // Use SoftwareSerial
#endif

#ifndef UART_HW_PORT
#define UART_HW_PORT           Serial      // Hardware serial port (if UART_MQTT_USE_SOFT == 0)
#endif

#ifndef UART_RX_PIN
#define UART_RX_PIN            4           // RX PIN (if UART_MQTT_USE_SOFT == 1)
#endif

#ifndef UART_TX_PIN
#define UART_TX_PIN            5           // TX PIN (if UART_MQTT_USE_SOFT == 1)
#endif

#ifndef UART_BAUDRATE
#define UART_BAUDRATE          115200      // Serial speed
#endif

#ifndef UART_TERMINATION
#define UART_TERMINATION      '\n'         // Termination character
#endif

#define UART_BUFFER_SIZE       200         // UART buffer size

void _receiveUART();
void _sendOnMqtt(const char * data);
void _sendOnUart(const char * message);
void _uartmqttLoop();
void uartmqttSetup();
//char * _toCharArray(String str);
void _sendMqttStatusToBluePill();
void _sendMqttStatusToBluePill(bool status);
void _settingsGet(char * data);
//void _settingsSet(char * data);

#endif