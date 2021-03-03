#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define WIDTH 160


#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f

static int fbfd;
static __u16 *fbp;
static struct fb_var_screeninfo vsinfo;
static struct fb_var_screeninfo vsinfo_bkp;

#define HIR_A_bits12 HIRAGANA_LETTER_A_bits12
#define HIR_A_bits14 HIRAGANA_LETTER_A_bits14
#define HIR_W 10
#define HIR_H 10
static char HIRAGANA_LETTER_A_bits[] = {
  0x08, 0x00, 0x08, 0x00, 0xFF, 0x01, 0x08, 0x00, 0x7C, 0x00, 0xA6, 0x00, 
  0x15, 0x01, 0x15, 0x01, 0x8E, 0x00, 0x60, 0x00, };
static char HIRAGANA_LETTER_A_bits11[] = {
  0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0xFE, 0x03, 0x10, 0x00, 0xF8, 0x00, 
  0x4C, 0x01, 0x2A, 0x02, 0x2A, 0x02, 0x1C, 0x01, 0xC0, 0x00, 0x00, 0x00};
static char HIRAGANA_LETTER_A_bits14[] = {
  0xFF, 0x3F, 0x01, 0x20, 0x21, 0x20, 0x21, 0x20, 0xFD, 0x27, 0x21, 0x20, 
  0xF1, 0x21, 0x99, 0x22, 0x55, 0x24, 0x55, 0x24, 0x39, 0x22, 0x81, 0x21, 
  0x01, 0x20, 0xFF, 0x3F, };

static inline void putpixel(int x, int y, __u16 c)
{
	fbp[x + y * WIDTH] = c;
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

static inline clear_screen(__u16 c)
{
	memset(fbp, c, 1);
}

#define put_10x10xbm_bg(x, y, fg, bg, bits) put_2B_xbm_bg(10, x, y, fg, bg, bits)
#define put_10x10xbm(x, y, c, bits) put_2B_xbm(10, x, y, c, bits)
#define put_12x12xbm_bg(x, y, fg, bg, bits) put_2B_xbm_bg(12, x, y, fg, bg, bits)
#define put_12x12xbm(x, y, c, bits) put_2B_xbm(12, x, y, c, bits)
#define put_14x14xbm_bg(x, y, fg, bg, bits) put_2B_xbm_bg(14, x, y, fg, bg, bits)
#define put_14x14xbm(x, y, c, bits) put_2B_xbm(14, x, y, c, bits)

int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb1", O_RDWR);
	if (fbfd == -1)
		return -1;
	fbp = (__u16*)mmap(0,
		128*160,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fbfd,
		0);
	if ((int)fbp == -1) {
		printf("Failed to mmap\n");
		return -2;
	}
	//if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vsinfo)) {
	//	printf("Error reading variable information.\n");
	//}
	//memcpy(&vsinfo_bkp, &vsinfo, sizeof(struct fb_var_screeninfo));
	//vsinfo.rotate = 270;
	//if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vsinfo)) {
	//	printf("Error setting variable information.\n");
	//}
	//ioctl(fbfd, FBIO_WAITFORVSYNC, &dummy);
	__u32 dummy = 0;
	fillrect(20, 20, 48, 16, 0x0000);
	put_14x14xbm_bg(20, 20, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(34, 20, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(48, 20, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(20, 34, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(34, 34, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(48, 34, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(20, 48, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(34, 48, 0x07e0, 0x0000, HIR_A_bits14);
	put_14x14xbm_bg(48, 48, 0x0000, 0x07e0, HIR_A_bits14);
	ioctl(fbfd, FBIO_WAITFORVSYNC, &dummy);
	getchar();
	//if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vsinfo_bkp)) {
	//	printf("Error setting variable information.\n");
	//}
	munmap(fbp, 128*160);
	close(fbfd);
	return 0;
}
