/****************************************************************************
 * Copyright (C) 2021 by Lukas Brüggemann, Markus Reinhold, Sebastian Pötter
 ****************************************************************************/

/**
 * @file led.h
 * @author Markus Reinhold
 * @date 1 Jan 2021
 * @brief Header file for led.c
 *
 */

#ifndef LED_H_ 
#define LED_H_

#include <stdbool.h>

void setLED(bool redValue, bool greenValue, bool blueValue);
void setup_LED(void);

#endif // LED_H_
