/*!
\file
\brief Заголовочный файл библиотеки с описанием функций

Данный файл содержит в себе определения основных 
функций, используемых в библиотеке
*/

#ifndef LIB_LED_H
#define LIB_LED_H

#include <stm32f4xx.h>

// Функция инициализации переферии.
void LedInit(void);

// Функция установки и чтения заданного в формате RGB цвета на светодиоде.
void LedSetColor (uint32_t rgb);
uint32_t LedGetColor (void);

// Функции включения, выключения, переключения и возвращения текущего статуса светодиода.
void LedTurnOff (void);
void LedTurnOn (void);
void LedToggle (void);
uint8_t LedGetStatus (void);

//Функции управления яркостью светодиода
void LedBrightUp (void);
void LedBrightDown (void);

#endif
