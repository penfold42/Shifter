#include "g27shifter.h"

// Calibration settings
uint16_t STICK_X_12	= 380;
uint16_t STICK_X_56R	= 650;        // 550;
uint16_t STICK_Y_135	= 700;        // was 425
uint16_t STICK_Y_246R	= 150;

uint16_t STICK_Y_SEQ_3	= 550;
uint16_t STICK_Y_SEQ_4	= 300;

uint16_t STICK_X_MIN	= 0;
uint16_t STICK_X_MAX	= 1023;
uint16_t STICK_Y_MIN	= 0;
uint16_t STICK_Y_MAX	= 1023;

void g27_initialize_io() {
  BUTTON_IO = (1 << BUTTON_SHIFT_REGISTER_MODE_PIN) | (1 << BUTTON_CLOCK_PIN);
  BUTTON_PORT = BUTTON_PORT & ~(1 << BUTTON_DATA_PIN);

  LED_IO |= (1 << LED_BIT);
  LED_PORT |= (1 << LED_BIT);

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz
  ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
  ADC_IO = 0;
}

static inline void latch_shift_register() {
  BUTTON_PORT = BUTTON_PORT & ~(1 << BUTTON_SHIFT_REGISTER_MODE_PIN);
  _delay_us(BUTTON_MODE_AND_CLOCK_WAIT);
  BUTTON_PORT = BUTTON_PORT | (1 << BUTTON_SHIFT_REGISTER_MODE_PIN);
}

static inline uint8_t read_button() {
  BUTTON_PORT = BUTTON_PORT & ~(1 << BUTTON_CLOCK_PIN);
  _delay_us(BUTTON_MODE_AND_CLOCK_WAIT);

  uint8_t button = (BUTTON_PIN >> BUTTON_DATA_PIN) & 0x01;

  BUTTON_PORT = BUTTON_PORT | (1 << BUTTON_CLOCK_PIN);
  _delay_us(BUTTON_MODE_AND_CLOCK_WAIT);
  return button;
}

static inline void read_shift_register_buttons(uint8_t *buttons) {
  latch_shift_register();
  for (uint8_t i = 0; i < NUMBER_OF_SHIFT_REGISTER_BUTTONS; i++) {
    buttons[i] = read_button();
  }
}

uint16_t read_buttons() {
  uint16_t buttonResult = 0;
  uint8_t buttons[NUMBER_OF_SHIFT_REGISTER_BUTTONS];

  read_shift_register_buttons(buttons);
  for (uint8_t i = 0; i < NUMBER_OF_SHIFT_REGISTER_BUTTONS; i++) {
      buttonResult |= (buttons[i] << i);
  }

  return buttonResult;
}

static inline uint16_t read_adc(uint8_t mux) {
  ADMUX = ((ADMUX) & ~7) | (mux & 7);
  ADCSRA |= (1 << ADEN);  // Enable ADC
  ADCSRA |= (1 << ADSC);  // Start A2D Conversions
  while(ADCSRA & (1 << ADSC)) {}

  int result = ADCL | (ADCH << 8); //ADCL must be read first
  ADCSRA = ADCSRA & ~(1 << ADEN);  // Disable ADC, ADC must be disabled to change MUX
  return result;
}

extern g27coordinates c;

void update_adc_values() {
  c.x = read_adc(STICK_X_ADC);
  c.y = read_adc(STICK_Y_ADC);
  if (c.x >= STICK_X_MAX) STICK_X_MAX = c.x;
  if (c.x <= STICK_X_MIN) STICK_X_MIN = c.x;
  if (c.y >= STICK_Y_MAX) STICK_Y_MAX = c.y;
  if (c.y <= STICK_Y_MIN) STICK_Y_MIN = c.y;
}

static inline uint8_t decode_Sequential(g27coordinates c, bool isStickDown) {
      if(c.y > STICK_Y_SEQ_3){
        return shiftDown;
      }

      if(c.y < STICK_Y_SEQ_4){
        return shiftUp;
      }

      return neutral;
}

static inline uint8_t decode_Hpattern(g27coordinates c, bool isStickDown) {

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

uint8_t read_selected_gear(bool isStickDown, bool isSequential){
  update_adc_values();
  uint8_t selectedGear = 0;
  if (isSequential) {
    selectedGear = decode_Sequential(c, isStickDown);
  } else {
    selectedGear = decode_Hpattern(c, isStickDown);
  }
  return selectedGear;
}
