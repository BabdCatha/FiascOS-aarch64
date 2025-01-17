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

    lfb_init_tty();
    tty_printf("FiascOS starting\n");
    //lfb_showpicture(image_data, IMAGE_WIDTH, IMAGE_HEIGHT, 1920-IMAGE_WIDTH, 1080-IMAGE_HEIGHT);

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
        tty_printf("Serial number: 0x%x%x\n", mbox[6], mbox[5]);
    } else {
        tty_printf("Unable to query serial!\n");
    }

    int y = 0;
    char test[] = "abc";

    while(1){
        wait_micro(500000);
        tty_printf("test: %d + %s\n", y, &test);
        y++;
    }

}