#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "fonts.h"



__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};



static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

uint8_t charsize;
uint8_t charwidth;
uint8_t cursorXpos;
uint8_t cursorYpos;
struct limine_framebuffer *framebuffer;
volatile uint32_t *fb_ptr;


void init_fb() {
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    framebuffer = framebuffer_request.response->framebuffers[0];

	/* Code to gather information used for the printing of characters */
	charsize = 8;
	charwidth = (framebuffer->width / charsize);
	cursorXpos = 0;
	cursorYpos = 0;
	fb_ptr = framebuffer->address;

}


/* Small helper function for the later kputchar() 
	that plots a pixel on to the screen
	TODO: add color support (foreground, background) */
	inline __attribute__((always_inline)) void plot(int x, int y, uint32_t color) {
		fb_ptr[(x + (y * framebuffer->pitch / sizeof(uint32_t)))] = color;
	}

/* When called, places a character into the framebuffer
DONE: allow placement of multiple chars without overlapping
TODO: maybe add support for a 16x16 font for readability?
TODO: multi-line support
TODO: probably move all of this code to a seperate file
TODO: color */ 
void kputchar(char x) {
	char *fontchar = font8x8_basic[x];
	for (int i = 0; i < 8; i++) {
		char currentline = fontchar[i];
		if (x == '\n') {
				cursorXpos = 0;
				++cursorYpos;
				return;
			}
		for (int j = 0; j < 8; j++) {
			if ((currentline >> (7 - j)) & 1) {
				plot((7 - j + cursorXpos * charsize), (i + (charsize * cursorYpos)), 0x0000FF);
			}
			else {
				plot((7 - j + cursorXpos * charsize), (i + (charsize * cursorYpos)), 0xFFFFFF);
			}
		}
	}
	++cursorXpos;
}

/* a non-formatted print function */
void kprint(char *string) {
	int i = 0;
	while (*(string + i) != 0) {
		kputchar(string[i]);
		i++;
	}
}
