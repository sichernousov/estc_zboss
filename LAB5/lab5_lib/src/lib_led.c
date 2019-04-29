/*!
\file
\brief Файл с имплементацией функций. <br>
Данный файл содержит в себе имплементацию основных, а также вспомогательных (статических)
функций, используемых в библиотеке.
*/

#include "../include/lib_led.h"

//!Период таймера.
#define T_PERIOD  (256 - 1)

//!Набор базовых цветов и соответствующих им номеров используемых каналов в ШИМ.
typedef enum {
	chRed   = 1,
	chGreen = 2,
	chBlue  = 3
} t_led_channel;

/*! 
\brief Функция цветокоррекции для светодиода. <br>
 Откорректированный диапазон значений ШИМ для каждого канала: <br>
	Red:   0x00 .. 0xBB <br>
	Green: 0x90 .. 0xFF <br>
    Blue:  0x00 .. 0xBB <br>
\param[in] ch Номер канала (1..3)
\param[in] val Значение для ШИМ (0..255)
\return Откорректированное значение для ШИМ 
*/
static uint8_t color_correct (t_led_channel ch, uint8_t val)
{
  uint8_t result = 0;
  switch(ch)
	{
	  case chRed:
	  {
		  result = (0xBB * val) >> 8;
		  break;
	  }
	  case chGreen:
	  {
		  result = 0x90 + ((0x6F * val) >> 8);
		  break;
	  }
	  case chBlue:
	  {
		  result = (0xBB * val) >> 8;
		  break;
	  }
	}
  return result;
}

/*!
\brief Функция установки заданного значения в нужный канал ШИМ.
\param[in] ch Номер канала (1..3)
\param[in] val Значение для установки в ШИМ (0..255)
*/
static void set_channel (t_led_channel ch, uint8_t val)
{
	switch(ch)
	{
	  case chRed:   TIM_SetCompare1 (TIM1, color_correct(chRed,   val)); break;
	  case chGreen: TIM_SetCompare2 (TIM1, color_correct(chGreen, val)); break;
	  case chBlue:  TIM_SetCompare3 (TIM1, color_correct(chBlue,  val)); break;
	}
}

/*!
\brief Функция инициализации переферии.
Необходимо вызывать перед использованием остальных функций библиотеки. <br>
Содержит в себе настройку <br>
GPIO (GPIOA, Pins: 8, 9, 10), <br>
Timer PWM (TIM1) <br>
*/
void LedInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	//GPIOA, advance led
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig (GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
	GPIO_PinAFConfig (GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);
	GPIO_PinAFConfig (GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);

	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Init Timers */
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);

	// TIM1
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_TIM1, ENABLE);
	TIM_InitStructure.TIM_Period = T_PERIOD;
	TIM_InitStructure.TIM_Prescaler = 168 - 1;
	TIM_InitStructure.TIM_ClockDivision = 0;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit (TIM1, &TIM_InitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_Pulse = 0;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_CtrlPWMOutputs (TIM1, ENABLE);
	TIM_Cmd (TIM1, ENABLE);
}

/*!
\brief Функция устанавливает заданный в формате RGB цвет на светодиод. <br>
Примеры использования:
\code
	LedSetColor (0xFF0000); //Red
    LedSetColor (0x00FFFF); //Aqua
    LedSetColor (0xC0C0C0); //Silver
\endcode
\param[in] rgb Код цвета в формате RGB (3 байта)
*/
void LedSetColor (uint32_t rgb)
{
	set_channel (chRed,   (uint8_t) ((rgb & 0xFF0000) >> 16));
	set_channel (chGreen, (uint8_t) ((rgb & 0xFF00)   >> 8) );
	set_channel (chBlue,  (uint8_t) ( rgb & 0xFF)           );
}

/*!
\brief Функция выключения светодиода. <br>
Аналогична вызову функции LedSetColor(0x00).
*/
void LedTurnOff (void)
{
	LedSetColor(0x00);
}
