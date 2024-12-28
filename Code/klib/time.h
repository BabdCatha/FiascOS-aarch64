#include "gpio.h"

void wait_cycles(unsigned int n);
void wait_micro(unsigned int n);
void wait_micro_st(unsigned int n);

unsigned long get_system_timer();