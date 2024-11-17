// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
// SPDX-License-Identifier: MIT
// Copyright 2023 

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  // TODO: Put your chip variables here
  pin_t pin_a0;
  pin_t pin_d0;
  pin_t pin_vcc;
  pin_t pin_gnd;
  uint32_t hr_attr;
  uint32_t threshold_attr;

} chip_state_t;

static void chip_timer_event(void *user_data);

void chip_timer_event(void *user_data){
  chip_state_t *chip = (chip_state_t*)user_data;
  float voltage = (attr_read_float(chip->hr_attr))*5/200;
  float threshold_v = (attr_read_float(chip->threshold_attr))*5/200;

  if(pin_read(chip->pin_vcc) && !pin_read(chip->pin_gnd))
  {
    pin_dac_write(chip->pin_a0, voltage);
    //if(voltage > threshold_v)
    //{
    //  pin_write(chip->pin_d0, HIGH);
    //}
    //else
    //{
    //  pin_write(chip->pin_d0, LOW);
    //}
  }
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->pin_a0 = pin_init("A0", ANALOG);
  chip->hr_attr = attr_init("heartRate", 10);
  chip->threshold_attr = attr_init("threshold", 50);
  chip->pin_d0 = pin_init("D0", OUTPUT_LOW);
  chip->pin_vcc = pin_init("VCC", INPUT_PULLDOWN);
  chip->pin_gnd = pin_init("GND", INPUT_PULLUP);

  const timer_config_t timer_config = {
    .callback = chip_timer_event,
    .user_data = chip,
  };

  timer_t timer_id = timer_init(&timer_config);
  timer_start(timer_id, 1000, true);
  

  // TODO: Initialize the chip, set up IO pins, create timers, etc.

  printf("Sensor de frecuencia cardiaca!\n");
}
