#ifndef UTEIS_H
#define UTEIS_H

#include <stdint.h>
#include "timer.h"


void set_microstep_speed(GPT_t* gptp, int8_t velocidade);

uint8_t get_nbr_overflows(void);


#endif /* UTEIS_H */