#include "uart.h"
#include "mbox.h"
#include "klib/time.h"
#include "klib/rand.h"
#include "klib/power.h"
#include "klib/display.h"
#include "klib/sd.h"
#include "klib/fat.h"

#include "klib/koupen.h"

void main()
{
    // set up serial console
    uart_init();

    lfb_init_tty();
    tty_printf("FiascOS starting\n");
    //lfb_showpicture(image_data, IMAGE_WIDTH, IMAGE_HEIGHT, 1920-IMAGE_WIDTH, 1080-IMAGE_HEIGHT);

    wait_micro_st(2500000);

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
        tty_printf("[KERNEL] Serial number: 0x%x%x\n", mbox[6], mbox[5]);
    } else {
        tty_printf("[KERNEL] Unable to query serial number\n");
    }

    // initialize EMMC and detect SD card type
    if(sd_init()==SD_OK) {
        // read the master boot record and find our partition
        if(fat_loadpartition() != FAT_LOAD_SUCCESS) {
            tty_printf("[KERNEL] FAT partition could not be loaded\n");
        } else {
            
        }
    }

    tty_printf("[KERNEL] Entering infinite loop");
    while(1){

    }

}