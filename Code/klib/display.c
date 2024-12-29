#include "display.h"

unsigned int screen_width, screen_height, screen_pitch, isrgb;   /* dimensions and channel order */
unsigned char *lfb;                         /* raw frame buffer address */

/**
 * Set screen resolution to 1024x768
 */
void lfb_init(){

    mbox[0] = 35*4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003;      //set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1920;         //FrameBufferInfo.width
    mbox[6] = 1080;         //FrameBufferInfo.height

    mbox[7] = 0x48004;      //set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1920;        //FrameBufferInfo.virtual_width
    mbox[11] = 1080;        //FrameBufferInfo.virtual_height

    mbox[12] = 0x48009;     //set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;           //FrameBufferInfo.x_offset
    mbox[16] = 0;           //FrameBufferInfo.y.offset

    mbox[17] = 0x48005;     //set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;          //FrameBufferInfo.depth

    mbox[21] = 0x48006;     //set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;           //RGB, not BGR preferably

    mbox[25] = 0x40001;     //get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;        //FrameBufferInfo.pointer
    mbox[29] = 0;           //FrameBufferInfo.size

    mbox[30] = 0x40008;     //get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;           //FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    //this might not return exactly what we asked for, could be
    //the closest supported resolution instead
    if(mbox_call(MBOX_CH_PROP) && mbox[20]==32 && mbox[28]!=0) {
        mbox[28]&=0x3FFFFFFF;   //convert GPU address to ARM address
        screen_width=mbox[5];   //get actual physical width
        screen_height=mbox[6];  //get actual physical height
        screen_pitch=mbox[33];  //get number of bytes per line
        isrgb=mbox[24];         //get the actual channel order
        lfb=(void*)((unsigned long)mbox[28]);
    } else {
        uart_puts("Unable to set screen resolution to 1920x1080x32\n");
    }
}

/**
 * Show a picture
 */
void lfb_showpicture(uint32_t* data, int image_width, int image_height, int print_x, int print_y){
    int x,y;
    uint8_t *ptr=lfb;

    ptr += print_y * screen_pitch;
    ptr += print_x * 4;



    int i = 0;

    for(y=0;y<image_height;y++) {
        for(x=0;x<image_width;x++) {
            // the image is in RGB. So if we have an RGB framebuffer, we can copy the pixels
            // directly, but for BGR we must swap R (pixel[0]) and B (pixel[2]) channels.

            //data[i] = 0xffffffff;

            if(isrgb){
                ptr[0] = ((uint8_t*)(&data[i]))[3];
                ptr[1] = ((uint8_t*)(&data[i]))[2];
                ptr[2] = ((uint8_t*)(&data[i]))[1];
                ptr[3] = ((uint8_t*)(&data[i]))[0];
            }else{
                ptr[0] = ((uint8_t*)(&data[i]))[1];
                ptr[1] = ((uint8_t*)(&data[i]))[2];
                ptr[2] = ((uint8_t*)(&data[i]))[3];
                ptr[3] = ((uint8_t*)(&data[i]))[0];
            }

            ptr+=4;
            i++;
        }
        ptr+=screen_pitch-image_width*4;
    }
}