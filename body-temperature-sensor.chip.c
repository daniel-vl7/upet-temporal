#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t wokwi_timer_t; // Custom type alias to avoid conflict

typedef struct {
  pin_t pin_a0;            // Analog output pin
  pin_t pin_d0;            // Digital output pin
  pin_t pin_vcc;           // Power supply pin
  pin_t pin_gnd;           // Ground pin
  uint32_t hr_attr;        // Body temperature attribute
} chip_state_t;

//upet
static void chip_timer_event(void *user_data);

void chip_timer_event(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;

  // Read body temperature attribute and convert to voltage
  float bodyTemperature = attr_read_float(chip->hr_attr);
  float voltage = (bodyTemperature - 20.0) * (5.0 / (45.0 - 20.0));

  // Output voltage to analog pin A0
  pin_dac_write(chip->pin_a0, voltage);

  // Example logic based on voltage
  //if (voltage > 2.5) {
  //  pin_write(chip->pin_d0, HIGH); // Set D0 HIGH based on voltage condition
  //} else {
  //  pin_write(chip->pin_d0, LOW);
  //}
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  // Initialize pins
  chip->pin_a0 = pin_init("A0", ANALOG);
  chip->pin_d0 = pin_init("D0", OUTPUT_LOW);
  chip->pin_vcc = pin_init("VCC", INPUT_PULLDOWN);
  chip->pin_gnd = pin_init("GND", INPUT_PULLUP);

  // Initialize body temperature attribute with default value
  chip->hr_attr = attr_init("bodyTemperature", 20.0); // Initial temperature set to 20.0°C

  const timer_config_t timer_config = {
    .callback = chip_timer_event,
    .user_data = chip,
  };

  wokwi_timer_t timer_id = timer_init(&timer_config);
  timer_start(timer_id, 1000, true); // Start timer with 1000ms interval

  // Initialize the chip, set up IO pins, create timers, etc.
  printf("Sensor de temperatura corporal!\n");
}
