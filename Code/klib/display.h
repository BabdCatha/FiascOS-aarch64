#include "uart.h"
#include "mbox.h"
#include "time.h"
#include "../stdlib/stdmem.h"

#include <stdarg.h>

#define FONT_HEIGHT 16

/* PC Screen Font as used by Linux Console */
typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int numglyph;
    unsigned int bytesperglyph;
    unsigned int height;
    unsigned int width;
    unsigned char glyphs;
} __attribute__((packed)) psf_t;
extern volatile unsigned char _binary____Code_fonts_font_psf_start;

/* Scalable Screen Font (https://gitlab.com/bztsrc/scalable-font2) */
typedef struct {
    unsigned char  magic[4];
    unsigned int   size;
    unsigned char  type;
    unsigned char  features;
    unsigned char  width;
    unsigned char  height;
    unsigned char  baseline;
    unsigned char  underline;
    unsigned short fragments_offs;
    unsigned int   characters_offs;
    unsigned int   ligature_offs;
    unsigned int   kerning_offs;
    unsigned int   cmap_offs;
} __attribute__((packed)) sfn_t;
extern volatile unsigned char _binary____Code_fonts_font_sfn_start;

void lfb_init_tty();
void lfb_showpicture(uint32_t* data, int image_width, int image_height, int print_x, int print_y);
void lfb_print(int x, int y, char s);
void lfb_proprint_s(int x, int y, char* s);
void lfb_scroll(int delta_y);
void tty_putc(char c);
void tty_printf(const char* fmt, ...);
void tty_printf_number(va_list* args, int length, bool sign, int radix);
void tty_clearline(int line_y);