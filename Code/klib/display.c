#include "display.h"

unsigned int screen_width, screen_height, screen_pitch, isrgb;   /* dimensions and channel order */
unsigned char *lfb;                         /* raw frame buffer address */

/**
 * Set screen resolution to 1920x1080
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

/**
 * Display a string using fixed size PSF
 */
void lfb_print(int x, int y, char *s){
    // get our font
    psf_t *font = (psf_t*)&_binary____Code_fonts_font_psf_start;
    // draw next character if it's not zero
    while(*s) {
        // get the offset of the glyph. Need to adjust this to support unicode table
        unsigned char *glyph = (unsigned char*)&_binary____Code_fonts_font_psf_start +
         font->headersize + (*((unsigned char*)s)<font->numglyph?*s:0)*font->bytesperglyph;
        // calculate the offset on screen
        int offs = (y * screen_pitch) + (x * 4);
        // variables
        int i,j, line,mask, bytesperline=(font->width+7)/8;
        // handle carrige return
        if(*s == '\r') {
            x = 0;
        } else
        // new line
        if(*s == '\n') {
            x = 0; y += font->height;
        } else {
            // display a character
            for(j=0;j<font->height;j++){
                // display one row
                line=offs;
                mask=1<<(font->width-1);
                for(i=0;i<font->width;i++){
                    // if bit set, we use white color, otherwise black
                    *((unsigned int*)(lfb + line))=((int)*glyph) & mask?0xFFFFFF:0;
                    mask>>=1;
                    line+=4;
                }
                // adjust to next line
                glyph+=bytesperline;
                offs+=screen_pitch;
            }
            x += (font->width+1);
        }
        // next character
        s++;
    }
}

/**
 * Display a string using proportional SSFN
 */
void lfb_proprint(int x, int y, char *s){
    // get our font
    sfn_t *font = (sfn_t*)&_binary____Code_fonts_font_sfn_start;
    unsigned char *ptr, *chr, *frg;
    unsigned int c;
    unsigned long o, p;
    int i, j, k, l, m, n;

    while(*s) {
        // UTF-8 to UNICODE code point
        if((*s & 128) != 0) {
            if(!(*s & 32)) { c = ((*s & 0x1F)<<6)|(*(s+1) & 0x3F); s += 1; } else
            if(!(*s & 16)) { c = ((*s & 0xF)<<12)|((*(s+1) & 0x3F)<<6)|(*(s+2) & 0x3F); s += 2; } else
            if(!(*s & 8)) { c = ((*s & 0x7)<<18)|((*(s+1) & 0x3F)<<12)|((*(s+2) & 0x3F)<<6)|(*(s+3) & 0x3F); s += 3; }
            else c = 0;
        } else c = *s;
        s++;
        // handle carrige return
        if(c == '\r') {
            x = 0; continue;
        } else
        // new line
        if(c == '\n') {
            x = 0; y += font->height; continue;
        }
        // find glyph, look up "c" in Character Table
        for(ptr = (unsigned char*)font + font->characters_offs, chr = 0, i = 0; i < 0x110000; i++) {
            if(ptr[0] == 0xFF) { i += 65535; ptr++; }
            else if((ptr[0] & 0xC0) == 0xC0) { j = (((ptr[0] & 0x3F) << 8) | ptr[1]); i += j; ptr += 2; }
            else if((ptr[0] & 0xC0) == 0x80) { j = (ptr[0] & 0x3F); i += j; ptr++; }
            else { if((unsigned int)i == c) { chr = ptr; break; } ptr += 6 + ptr[1] * (ptr[0] & 0x40 ? 6 : 5); }
        }
        if(!chr) continue;
        // uncompress and display fragments
        ptr = chr + 6; o = (unsigned long)lfb + y * screen_pitch + x * 4;
        for(i = n = 0; i < chr[1]; i++, ptr += chr[0] & 0x40 ? 6 : 5) {
            if(ptr[0] == 255 && ptr[1] == 255) continue;
            frg = (unsigned char*)font + (chr[0] & 0x40 ? ((ptr[5] << 24) | (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]) :
                ((ptr[4] << 16) | (ptr[3] << 8) | ptr[2]));
            if((frg[0] & 0xE0) != 0x80) continue;
            o += (int)(ptr[1] - n) * screen_pitch; n = ptr[1];
            k = ((frg[0] & 0x1F) + 1) << 3; j = frg[1] + 1; frg += 2;
            for(m = 1; j; j--, n++, o += screen_pitch)
                for(p = o, l = 0; l < k; l++, p += 4, m <<= 1) {
                    if(m > 0x80) { frg++; m = 1; }
                    if(*frg & m) *((unsigned int*)p) = 0xFFFFFF;
                }
        }
        // add advances
        x += chr[4]+1; y += chr[5];
    }
}