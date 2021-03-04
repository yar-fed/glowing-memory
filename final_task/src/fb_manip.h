#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "layouts.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128
#define SCREEN_RESOLUTION SCREEN_WIDTH * SCREEN_HEIGHT
#define SCREEN_BPP 16
#define SCREEN_BYTESPP SCREEN_BPP >> 3
#define SCREEN_BUFFER_SIZE SCREEN_RESOLUTION * SCREEN_BYTESPP

#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define WHITE 0xffff
#define BLACK 0x0000
#define TRANSPARENT 0x0000

#define SWAP(t, x, y) {t _tmp = x; x = y; y = _tmp;}

static int fbfd;
static __u16 *fbp;
static struct fb_var_screeninfo vsinfo;

static inline void putpixel(int x, int y, __u16 c)
{
	fbp[x + y * SCREEN_WIDTH] = c;
}

static inline void drawrect(int x, int y, int w, int h, __u16 c)
{
	for (int i = 0; i < w; ++i) {
		putpixel(x + i, y, c);
	}
	for (int i = 1; i < h - 1; ++i) {
		putpixel(x, y + i, c);
		putpixel(x + w - 1, y + i, c);
	}
	for (int i = 0; i < w; ++i) {
		putpixel(x + i, y + h - 1, c);
	}
}

static inline void fillrect(int x, int y, int w, int h, __u16 c)
{
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			putpixel(x + j, y + i, c);
		}
	}
}

static inline __u16 _get_2B_xbm_bit(char *bits, int x, int y)
{
	return ((u_int8_t)bits[(x >> 3) + (y << 1)] & (1 << (x % 8))) ? 0xffff : 0x0000;
}

static inline void put_2B_xbm_bg(int point_size, int x, int y, __u16 fg,__u16 bg, char *bits)
{
	for (int i = 0; i < point_size; ++i) {
		for (int j = 0; j < point_size; ++j) {
			__u16 c = _get_2B_xbm_bit(bits, j, i) ? fg : bg;
			putpixel(x + j, y + i, c);
		}
	}
}

static inline void put_2B_xbm(int point_size, int x, int y, __u16 c, char *bits)
{
	for (int i = 0; i < point_size; ++i) {
		for (int j = 0; j < point_size; ++j) {
			putpixel(x + j, y + i,
				c & _get_2B_xbm_bit(bits, j, i));
		}
	}
}

static inline void clear_screen(__u16 c)
{
	memset(fbp, c, SCREEN_BUFFER_SIZE);
}

#define put_10x10xbm_bg(x, y, fg, bg, bits) put_2B_xbm_bg(10, x, y, fg, bg, bits)
#define put_10x10xbm(x, y, c, bits) put_2B_xbm(10, x, y, c, bits)
#define put_12x12xbm_bg(x, y, fg, bg, bits) put_2B_xbm_bg(12, x, y, fg, bg, bits)
#define put_12x12xbm(x, y, c, bits) put_2B_xbm(12, x, y, c, bits)
#define put_14x14xbm_bg(x, y, fg, bg, bits) put_2B_xbm_bg(14, x, y, fg, bg, bits)
#define put_14x14xbm(x, y, c, bits) put_2B_xbm(14, x, y, c, bits)

static inline void draw_keyboard(int x, int y, __u16 fg, __u16 bg, __u16 layout)
{
	drawrect(x, y, 4*14+2, 4*14+2, fg);
	put_14x14xbm_bg(x +  1, y +  1, fg, bg, kb_layouts[layout].keys[0]);
	put_14x14xbm_bg(x + 15, y +  1, fg, bg, kb_layouts[layout].keys[1]);
	put_14x14xbm_bg(x + 29, y +  1, fg, bg, kb_layouts[layout].keys[2]);
	put_14x14xbm_bg(x + 43, y +  1, fg, bg, kb_layouts[layout].keys[3]);
	put_14x14xbm_bg(x +  1, y + 15, fg, bg, kb_layouts[layout].keys[4]);
	put_14x14xbm_bg(x + 15, y + 15, fg, bg, kb_layouts[layout].keys[5]);
	put_14x14xbm_bg(x + 29, y + 15, fg, bg, kb_layouts[layout].keys[6]);
	put_14x14xbm_bg(x + 43, y + 15, fg, bg, kb_layouts[layout].keys[7]);
	put_14x14xbm_bg(x +  1, y + 29, fg, bg, kb_layouts[layout].keys[8]);
	put_14x14xbm_bg(x + 15, y + 29, fg, bg, kb_layouts[layout].keys[9]);
	put_14x14xbm_bg(x + 29, y + 29, fg, bg, kb_layouts[layout].keys[10]);
	put_14x14xbm_bg(x + 43, y + 29, fg, bg, kb_layouts[layout].keys[11]);
	put_14x14xbm_bg(x +  1, y + 43, fg, bg, kb_layouts[layout].keys[12]);
	put_14x14xbm_bg(x + 15, y + 43, fg, bg, kb_layouts[layout].keys[13]);
	put_14x14xbm_bg(x + 29, y + 43, fg, bg, kb_layouts[layout].keys[14]);
	put_14x14xbm_bg(x + 43, y + 43, fg, bg, kb_layouts[layout].keys[15]);
}

static inline void draw_key(int x, int y, __u16 fg, __u16 bg, __u16 key, __u16 layout, int down)
{
	if (down)
		SWAP(__u16, fg, bg);
	put_14x14xbm_bg(x + 1 + (key & 3) * 14, y + 1 + (key >> 2) * 14, fg, bg, kb_layouts[layout].keys[key]);
}

static inline void draw_symbol(int x, int y, __u16 fg, __u16 bg, __u16 key, __u16 layout, int selected)
{
	if (selected)
		SWAP(__u16, fg, bg);
	put_12x12xbm_bg(x + 1 + (key & 3) * 12, y + 1 + (key >> 2) * 12, fg, bg, kb_layouts[layout].keys[key]);
}

int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb1", O_RDWR);
	if (fbfd == -1)
		return -1;
	fbp = (__u16*)mmap(0,
		SCREEN_BUFFER_SIZE,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fbfd,
		0);
	if ((int)fbp == -1) {
		printf("Failed to mmap\n");
		return -2;
	}
	__u32 dummy = 0;
	clear_screen(BLACK);
	draw_keyboard(20, 20, GREEN, BLACK, 0);
	for (int i = 0; i < 16; ++i) {
		draw_key(20, 20, GREEN, BLACK, i, 0, 1);
		ioctl(fbfd, FBIO_WAITFORVSYNC, &dummy);
		sleep(1);
		draw_key(20, 20, GREEN, BLACK, i, 0, 0);
	}
	munmap(fbp, SCREEN_BUFFER_SIZE);
	close(fbfd);
	return 0;
}
