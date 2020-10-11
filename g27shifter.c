#include "g27shifter.h"

// Calibration settings
uint16_t STICK_X_12	= 380;
uint16_t STICK_X_56R	= 650;        // 550;
uint16_t STICK_Y_135	= 700;        // was 425
uint16_t STICK_Y_246R	= 150;

uint16_t STICK_Y_SEQ_3	= 550;
uint16_t STICK_Y_SEQ_4	= 300;

uint16_t STICK_X_MIN = 0; uint16_t STICK_X_MAX = 1023;
uint16_t STICK_Y_MIN = 0; uint16_t STICK_Y_MAX = 1023;
uint16_t  CLUTCH_MIN = 0; uint16_t  CLUTCH_MAX = 1023;
uint16_t   BRAKE_MIN = 0; uint16_t   BRAKE_MAX = 1023;
uint16_t   ACCEL_MIN = 0; uint16_t   ACCEL_MAX = 1023;

void g27_initialize_io() {
  BUTTON_IO |= ( (1 << BUTTON_SRMODE_PIN) | (1 << BUTTON_CLOCK_PIN) );
  BUTTON_PORT |= (1 << BUTTON_DATA_PIN);	// pullup on data pin

  G25_LED_IO |= (1 << G25_LED_BIT);
  G25_LED_PORT |= (1 << G25_LED_BIT);
  RX_LED_IO |= (1 << RX_LED_BIT);
  RX_LED_PORT |= (1 << RX_LED_BIT);
  TX_LED_IO |= (1 << TX_LED_BIT);
  TX_LED_PORT |= (1 << TX_LED_BIT);

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz
  ADMUX = (1 << REFS0); // Set ADC reference to AVCC

  ADC_IO_SETUP		// macro defined with hardware pins

/* Setup SPI
   8 M/s was too fast - had corruption
   4 M/s was too fast - had corruption
   2 M/s was ok
   1 M/s was ok

*/
  SPSR  = (1<<SPI2X);			// double speed

  SPCR  = (0<<SPIE) | (1<<SPE)		// no interrupts
	| (1<<DORD) | (1<<MSTR)		// MSB first
	| (0<<CPOL) | (0<<CPHA)		// idle 0, sample trailing
	| (1<<SPR1) | (0<<SPR0)		// SPI2X + clk/64 = 500kHz
	;

}

uint16_t read_buttons() {
  uint16_t buttonResult = 0;

  // blink the LED every SPI access
  static int countMS;
  if (++countMS > 100) {
    countMS = 0;
    RX_LED_PORT &= ~(1<<RX_LED_BIT);
  }

  // latch shift registers
  BUTTON_PORT &= ~(1 << BUTTON_SRMODE_PIN);
  _delay_us(BUTTON_MODE_AND_CLOCK_WAIT);
  BUTTON_PORT |= (1 << BUTTON_SRMODE_PIN);

  SPDR = 0;			// do a transfer
  while(! (SPSR & (1<<SPIF)) );	// wait for it to finish
  buttonResult = SPDR;		// read the byte from 1st SR
  SPDR = 0;			// do a transfer
  while(! (SPSR & (1<<SPIF)) );	// wait for it to finish
  buttonResult |= (SPDR<<8);	// read the byte from 2nd SR

  RX_LED_PORT |= (1<<RX_LED_BIT);
  return buttonResult;
}

static inline uint16_t read_adc(uint8_t mux) {
  if (mux <= 7) {
    ADMUX = ((ADMUX) & 0b1110000) | (mux & 7);
    ADCSRB &= ~(1 << MUX5);	// for mux <= 7
  } else if (mux <= 15) {
    ADMUX = ((ADMUX) & 0b1110000) | (mux & 7);
    ADCSRB |= (1 << MUX5);	// for mux >= 8
  }
  ADCSRA |= (1 << ADEN);  // Enable ADC
  ADCSRA |= (1 << ADSC);  // Start A2D Conversions
  while(ADCSRA & (1 << ADSC)) {}

  int result = ADCL | (ADCH << 8); //ADCL must be read first
  ADCSRA = ADCSRA & ~(1 << ADEN);  // Disable ADC, ADC must be disabled to change MUX
  return result;
}

/* yay for global variables....
 * use this as a global as ADC reads are expensive
 */
g27coordinates AdcValues;

void update_adc_values(void) {
  AdcValues.x      = read_adc(STICK_X_ADC);
  AdcValues.y      = read_adc(STICK_Y_ADC);
  if (AdcValues.x >= STICK_X_MAX) STICK_X_MAX = AdcValues.x;
  if (AdcValues.x <= STICK_X_MIN) STICK_X_MIN = AdcValues.x;
  if (AdcValues.y >= STICK_Y_MAX) STICK_Y_MAX = AdcValues.y;
  if (AdcValues.y <= STICK_Y_MIN) STICK_Y_MIN = AdcValues.y;

#if (USE_PEDALS == 1)
  AdcValues.clutch = read_adc(CLUTCH_ADC);
  AdcValues.brake  = read_adc(BRAKE_ADC);
  AdcValues.accel  = read_adc(ACCEL_ADC);
  if (AdcValues.clutch >= CLUTCH_MAX) CLUTCH_MAX = AdcValues.clutch;
  if (AdcValues.clutch <= CLUTCH_MIN) CLUTCH_MIN = AdcValues.clutch;
  if (AdcValues.brake  >= BRAKE_MAX)  BRAKE_MAX  = AdcValues.brake;
  if (AdcValues.brake  <= BRAKE_MIN)  BRAKE_MIN  = AdcValues.brake;
  if (AdcValues.accel  >= ACCEL_MAX)  ACCEL_MAX  = AdcValues.accel;
  if (AdcValues.accel  <= ACCEL_MIN)  ACCEL_MIN  = AdcValues.accel;
#endif
}

static inline SelectedGear_t decode_Sequential(g27coordinates c, bool isStickDown) {
      if(c.y > STICK_Y_SEQ_3){
        return shiftDown;
      }

      if(c.y < STICK_Y_SEQ_4){
        return shiftUp;
      }

      return neutral;
}

static inline SelectedGear_t decode_Hpattern(g27coordinates c, bool isStickDown) {

      if(c.x < STICK_X_12) {
        if(c.y > STICK_Y_135){
          return first;
        }
        else if(c.y < STICK_Y_246R){
          return second;
        }
      }

      if(c.x > STICK_X_56R){
        if(c.y > STICK_Y_135){
          return fifth;
        }
        else if(c.y < STICK_Y_246R){
          if(isStickDown){
            return reverse;
          }
          return sixth;
        }
      }

      if(c.y > STICK_Y_135){
        return third;
      }

      if(c.y < STICK_Y_246R){
        return fourth;
      }

      return neutral;
}

SelectedGear_t read_selected_gear(bool isStickDown, bool isSequential){
  update_adc_values();
  SelectedGear_t selectedGear = neutral;
  if (isSequential) {
    selectedGear = decode_Sequential(AdcValues, isStickDown);
  } else {
    selectedGear = decode_Hpattern(AdcValues, isStickDown);
  }
  return selectedGear;
}
