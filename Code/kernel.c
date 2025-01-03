#include "uart.h"
#include "mbox.h"
#include "klib/time.h"
#include "klib/rand.h"
#include "klib/power.h"
#include "klib/display.h"

#include "klib/koupen.h"

void main()
{
    // set up serial console
    uart_init();
    
    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My serial number is: 0x");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }

    lfb_init();
    lfb_showpicture(image_data, IMAGE_WIDTH, IMAGE_HEIGHT, 1920-IMAGE_WIDTH, 1080-IMAGE_HEIGHT);

    lfb_print(80, 80, "Hello World!");

    lfb_proprint(80, 120, "Hello 多种语言 Многоязычный többnyelvű World!");

    int i = 0;

    wait_micro_st(10000000);

    for(i = 0; i < 5; i++){
        uart_hex(rand(0,4294967295));
        uart_puts("\n");
    }

    while(1){

    }

}