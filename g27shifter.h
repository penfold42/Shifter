#ifndef __G27_SHIFTER_H__
#define __G27_SHIFTER_H__

#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>

#include "Options.h"

#if (HWVARIANT == 1)

#define BUTTON_SRMODE_PIN 0			// PortD.0 = Arduino 3
#define BUTTON_DATA_PIN 4			// PortD.4 = Arduino 4
#define BUTTON_CLOCK_PIN 1			// PortD.1 = Arduino 2

#define BUTTON_PORT PORTD
#define BUTTON_PIN PIND
#define BUTTON_IO DDRD

#elif (HWVARIANT == 2)

#define BUTTON_SRMODE_PIN 6			// PortB.6 = Arduino 10
#define BUTTON_DATA_PIN 3			// PortB.3 = Arduino 14
#define BUTTON_CLOCK_PIN 1			// PortB.1 = Arduino 15

#define BUTTON_PORT PORTB
#define BUTTON_PIN PINB
#define BUTTON_IO DDRB

#else
#error invalid HWVARIANT
#endif

#define BUTTON_MODE_AND_CLOCK_WAIT 10
#define NUMBER_OF_SHIFT_REGISTER_BUTTONS 16

#define G25_LED_PORT PORTE
#define G25_LED_BIT 6				// PortE.6 = Arduino 7
#define G25_LED_IO DDRE

#define TX_LED_PORT PORTD
#define TX_LED_BIT 5                           // PortD.5 = Arduino NO!
#define TX_LED_IO DDRD

#define RX_LED_PORT PORTB
#define RX_LED_BIT 0                           // PortB.0 = Arduino 17?
#define RX_LED_IO DDRB

#define STICK_X_ADC 7				// PortF.7 = Arduino 18/A0
#define STICK_Y_ADC 6				// PortF.6 = Arduino 19/A1
#define CLUTCH_ADC 5				// PortF.5 = Arduino 20/A2
#define BRAKE_ADC 4				// PortF.4 = Arduino 21/A3
#define ACCEL_ADC 8				// PortD.4 = Arduino 4/A6
#define ADC_TO_IN DDRF &= ~(0b11110000); DDRD &= ~(0b00010000);


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
