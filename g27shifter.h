#ifndef __G27_SHIFTER_H__
#define __G27_SHIFTER_H__

#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>

#include "Options.h"

#define SPI_SRMODE_PIN 6		// PortB.6 = Arduino 10
#define SPI_MISO_PIN 3			// PortB.3 = Arduino 14
#define SPI_MOSI_PIN 2			// PortB.2 = Arduino 16
#define SPI_CLOCK_PIN 1			// PortB.1 = Arduino 15

#define SPI_PORT PORTB
#define SPI_PIN PINB
#define SPI_IO DDRB
#define G25_LED_PORT SPI_PORT
#define G25_LED_BIT SPI_MOSI_PIN

// button layout defines

#define SPARE1		(1<<0)	// spare pin on shift register
#define PRESSED		(1<<1)	// is the reverse switch pressed
#define SPARE2		(1<<2)	// spare pin on shift register
#define SEQUENTIAL	(1<<3)	// sequential or H-pattern

#define RED4_BITS	4	// bits 4..7
#define L3		(1<<3)
#define SELECT		(1<<1)
#define START		(1<<0)
#define R3		(1<<2)

#define TOP4_BITS	8	// bits 8..11
#define TRIANGLE	(1<<0)
#define CIRCLE		(1<<1)
#define SQUARE		(1<<2)
#define CROSS		(1<<3)

#define DPAD_BITS	12	// bits 12..15
#define NORTH		(1<<3)
#define WEST		(1<<1)
#define EAST		(1<<0)
#define SOUTH		(1<<2)

#define BUTTON_MODE_AND_CLOCK_WAIT 10

// blink every second when USB connected
#define TX_LED_PORT PORTD
#define TX_LED_BIT 5                           // PortD.5 = Arduino NO!
#define TX_LED_IO DDRD

// blink every 100 HID reports
#define RX_LED_PORT PORTB
#define RX_LED_BIT 0                           // PortB.0 = Arduino 17?
#define RX_LED_IO DDRB

#define STICK_X_ADC 7				// PortF.7 = Arduino 18/A0
#define STICK_Y_ADC 6				// PortF.6 = Arduino 19/A1
#define CLUTCH_ADC 5				// PortF.5 = Arduino 20/A2
#define BRAKE_ADC 4				// PortF.4 = Arduino 21/A3
#define ACCEL_ADC 8				// PortD.4 = Arduino 4/A6
// DDR to in.. pullups on pedals?
#define ADC_IO_SETUP  {		\
  DDRF  &= ~(0b11110000);	\
  DDRD  &= ~(0b00010000);	\
  PORTF |=  (0b00110000);	\
  PORTD |=  (0b00010000);	\
}

typedef enum SelectedGear {
	neutral,
	first,
	second,
	third,
	fourth,
	fifth,
	sixth,
	reverse,
	shiftUp,
	shiftDown
} SelectedGear_t;

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t clutch;
  uint16_t brake;
  uint16_t accel;
} g27coordinates;

void g27_initialize_io(void);

void update_adc_values(void);

uint16_t read_buttons(void);

uint8_t read_selected_gear(bool isStickDown, bool isSequential);

#endif
