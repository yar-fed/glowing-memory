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

struct fb_manip {
	static int fbfd;
	static __u16 *fbp;
};

static inline __u16 _get_2B_xbm_bit(char *bits, int x, int y)
{
	return ((u_int8_t)bits[(x >> 3) + (y << 1)] & (1 << (x % 8))) ? 0xffff : 0x0000;
}

static inline void putpixel(struct fb_manip *fb, int x, int y, __u16 c)
{
	fb->fbp[x + y * SCREEN_WIDTH] = c;
}

inline void drawrect(struct fb_manip *fb, int x, int y, int w, int h, __u16 c)
{
	for (int i = 0; i < w; ++i) {
		putpixel(fb, x + i, y, c);
	}
	for (int i = 1; i < h - 1; ++i) {
		putpixel(fb, x, y + i, c);
		putpixel(fb, x + w - 1, y + i, c);
	}
	for (int i = 0; i < w; ++i) {
		putpixel(fb, x + i, y + h - 1, c);
	}
}

inline void fillrect(struct fb_manip *fb, int x, int y, int w, int h, __u16 c)
{
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			putpixel(fb, x + j, y + i, c);
		}
	}
}

inline void put_2B_xbm_bg(struct fb_manip *fb, int point_size, int x, int y, __u16 fg,__u16 bg, char *bits)
{
	for (int i = 0; i < point_size; ++i) {
		for (int j = 0; j < point_size; ++j) {
			__u16 c = _get_2B_xbm_bit(bits, j, i) ? fg : bg;
			putpixel(x + j, y + i, c);
		}
	}
}

inline void put_2B_xbm(struct fb_manip *fb, int point_size, int x, int y, __u16 c, char *bits)
{
	for (int i = 0; i < point_size; ++i) {
		for (int j = 0; j < point_size; ++j) {
			putpixel(x + j, y + i,
				c & _get_2B_xbm_bit(bits, j, i));
		}
	}
}

inline void clear_screen(struct fb_manip *fb, __u16 c)
{
	memset(fb->fbp, c, SCREEN_BUFFER_SIZE);
}

#define put_10x10xbm_bg(fb, x, y, fg, bg, bits) put_2B_xbm_bg(fb, 10, x, y, fg, bg, bits)
#define put_10x10xbm(fb, x, y, c, bits) put_2B_xbm(fb, 10, x, y, c, bits)
#define put_12x12xbm_bg(fb, x, y, fg, bg, bits) put_2B_xbm_bg(fb, 12, x, y, fg, bg, bits)
#define put_12x12xbm(fb, x, y, c, bits) put_2B_xbm(fb, 12, x, y, c, bits)
#define put_14x14xbm_bg(fb, x, y, fg, bg, bits) put_2B_xbm_bg(fb, 14, x, y, fg, bg, bits)
#define put_14x14xbm(fb, x, y, c, bits) put_2B_xbm(fb, 14, x, y, c, bits)

inline void draw_keyboard(struct fb_manip *fb, int x, int y, __u16 fg, __u16 bg, __u16 layout)
{
	drawrect(fb, x, y, 4*14+2, 4*14+2, fg);
	int k = 0;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			put_14x14xbm_bg(fb, x + 1 + j * 14, y + 1 + i * 14,
					fg, bg, kb_layouts[layout].keys[k++]);
		}
	}
}

inline void draw_key(struct fb_manip *fb, int x, int y, __u16 fg, __u16 bg, __u16 key, __u16 layout, int down)
{
	if (down)
		SWAP(__u16, fg, bg);
	put_14x14xbm_bg(fb, x + 1 + (key & 3) * 14, y + 1 + (key >> 2) * 14, fg, bg, kb_layouts[layout].keys[key]);
}

inline void draw_symbol(struct fb_manip *fb, int x, int y, __u16 fg, __u16 bg, __u16 key, __u16 layout, int selected)
{
	if (selected)
		SWAP(__u16, fg, bg);
	put_12x12xbm_bg(fb, x + 1 + (key & 3) * 12, y + 1 + (key >> 2) * 12, fg, bg, kb_layouts[layout].keys[key]);
}

struct fb_manip *grab_fb(char *fb)
{
	if (NULL == fb)
		fb = "/dev/fb1";
	fbfd = open(fb, O_RDWR);
	if (fbfd == -1) {
		printf("Failed to open fb\n");
		return -1;
	}
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
	//draw_keyboard(20, 20, GREEN, BLACK, 0);
	for (int i = 0; i < 23; ++i) {
		draw_keyboard(20, 20, GREEN, BLACK, i);
		ioctl(fbfd, FBIO_WAITFORVSYNC, &dummy);
		getchar();
		draw_keyboard(20, 20, GREEN, BLACK, i);
	}
}

void release_fb(struct fb_manip *fb)
{
	munmap(fb->fbp, SCREEN_BUFFER_SIZE);
	close(fb->fbfd);
	return 0;
}
