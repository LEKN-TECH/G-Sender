#ifndef ENCODER_H
#define ENCODER_H

#include "gsender.h"

extern volatile int16_t encoder_value;
extern volatile int8_t encoder_push;
extern volatile int8_t encoder_changed;

void encoder_init(void);

#endif
