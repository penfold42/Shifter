#ifndef __G27_SHIFTER_H__
#define __G27_SHIFTER_H__

#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>

#include "Options.h"

#define BUTTON_SHIFT_REGISTER_MODE_PIN 0	// PortD.0 = Arduino 3
#define BUTTON_DATA_PIN 4			// PortD.4 = Arduino 4
#define BUTTON_CLOCK_PIN 1			// PortD.1 = Arduino 2
#define BUTTON_MODE_AND_CLOCK_WAIT 10
#define NUMBER_OF_SHIFT_REGISTER_BUTTONS 16

#define BUTTON_PORT PORTD
#define BUTTON_PIN PIND
#define BUTTON_IO DDRD

#define LED_PORT PORTE
#define LED_BIT 6				// PortE.6 = Arduino 7
#define LED_IO DDRE

#define STICK_X_ADC 7				// PortF.7 = Arduino A0
#define STICK_Y_ADC 6				// PortF.6 = Arduino A1
#define ADC_IO DDRF

/*
#define STICK_X_12 330
#define STICK_X_56R 550
#define STICK_Y_135 700		// 425
#define STICK_Y_246R 150

#define STICK_Y_SEQ_3 550		// 425
#define STICK_Y_SEQ_4 300
*/

#define neutral		0
#define first		1
#define second		2
#define third		3
#define fourth		4
#define fifth		5
#define sixth		6
#define reverse		7
#define shiftUp		8
#define shiftDown	9

typedef struct {
  uint16_t x;
  uint16_t y;
} g27coordinates;

void g27_initialize_io(void);

uint16_t read_buttons(void);

uint8_t read_selected_gear(bool isStickDown, bool isSequential);

#endif
