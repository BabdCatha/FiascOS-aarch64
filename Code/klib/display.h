#include "uart.h"
#include "mbox.h"
#include "time.h"

void lfb_init();
void lfb_showpicture(uint32_t* data, int image_width, int image_height, int print_x, int print_y);