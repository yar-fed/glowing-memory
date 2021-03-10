#include "evdev_manip.h"
#include "fb_manip.h"
#include <signal.h>

// SCREEN_WIDTH / 20
#define TEXT_BUFFER_WIDTH 8 
#define TEXT_BUFFER_HEIGHT SCREEN_HEIGHT / 10
#define TEXT_BUFFER_LENGTH TEXT_BUFFER_HEIGHT * TEXT_BUFFER_WIDTH
#define KB_X 91
#define KB_Y 35

struct kp_event{
	struct input_event scan;
	struct input_event key;
	struct input_event sync;
};
#define KP_EV_SIZE sizeof(struct input_event) * 3
struct kb_state_t {
	int layout; // 0-22
	int previous_layout; // 0-22
	int key_pressed; // 0-15 or -1 for none
	int cursor;
};
static struct kb_state_t state = {
	.layout = LAYOUT_HIRAGANA_DEFAULT,
	.previous_layout = LAYOUT_HIRAGANA_DEFAULT,
	.key_pressed = -1,
	.cursor = 0
};

static __u16 text_buffer[TEXT_BUFFER_LENGTH];

static struct fb_manip *fb;
static int kp;

extern inline void clear_screen(struct fb_manip *fb, __u16 c);
extern inline __u16 _get_2B_xbm_bit(char *bits, int x, int y);
extern inline void putpixel(struct fb_manip *fb, int x, int y, __u16 c);
extern inline void put_2B_xbm_bg(struct fb_manip *fb, int point_size, int x,
				 int y, __u16 fg, __u16 bg, char *bits);
extern inline void draw_symbol_from_char_code(struct fb_manip *fb, int x, int y,
					      __u16 fg, __u16 bg, int char_code,
					      int selected);
extern inline void draw_symbol_from_key(struct fb_manip *fb, int x, int y,
					__u16 fg, __u16 bg, int key, int layout,
					int selected);
extern inline void draw_keyboard(struct fb_manip *fb, int x, int y, __u16 fg, __u16 bg, int layout);
extern inline void draw_key(struct fb_manip *fb, int x, int y, __u16 fg, __u16 bg, int key, int layout, int down);
extern inline void drawrect(struct fb_manip *fb, int x, int y, int w, int h, __u16 c);
extern inline void wait_for_sync(struct fb_manip *fb);

extern inline int read_event(int fd, struct input_event *e, size_t ev_size);

inline int CURSOR_X(void)
{
	return state.cursor % TEXT_BUFFER_WIDTH * 10;
}
inline int CURSOR_Y(void)
{
	return state.cursor / TEXT_BUFFER_WIDTH * 10;
}
extern inline int CURSOR_X(void);
extern inline int CURSOR_Y(void);


static void change_with_sound_mark(int shift)
{
	if (text_buffer[state.cursor] > 0x3040 + shift) {
		text_buffer[state.cursor] -= shift;
		if (text_buffer[state.cursor] < HIRAGANA_LETTER_SMALL_TU) {
			if (text_buffer[state.cursor] & 1)
				text_buffer[state.cursor] += 1;
			else
				text_buffer[state.cursor] -= 1;
		} else if (text_buffer[state.cursor] < HIRAGANA_LETTER_TE) {
			if (text_buffer[state.cursor] ==
			    HIRAGANA_LETTER_SMALL_TU)
				text_buffer[state.cursor] = HIRAGANA_LETTER_DU;
			else
				text_buffer[state.cursor] -= 1;
		} else if (text_buffer[state.cursor] < HIRAGANA_LETTER_NA) {
			if (text_buffer[state.cursor] & 1)
				text_buffer[state.cursor] += 1;
			else
				text_buffer[state.cursor] -= 1;
		} else if (text_buffer[state.cursor] < HIRAGANA_LETTER_MA &&
			   text_buffer[state.cursor] > HIRAGANA_LETTER_NO) {
			if ((text_buffer[state.cursor] - HIRAGANA_LETTER_HA) %
				    3 ==
			    2) {
				text_buffer[state.cursor] -= 2;
			} else {
				text_buffer[state.cursor] += 1;
			}
		} else if (text_buffer[state.cursor] < HIRAGANA_LETTER_RA &&
			   text_buffer[state.cursor] > HIRAGANA_LETTER_MO) {
			if (text_buffer[state.cursor] & 1)
				text_buffer[state.cursor] += 1;
			else
				text_buffer[state.cursor] -= 1;
		}
		text_buffer[state.cursor] += shift;
	}
}

static int on_key_down(int key)
{
	printf("%s %d\n", __FUNCTION__, key);
	state.key_pressed = key;
	draw_key(fb, KB_X, KB_Y, GREEN, BLACK,
			key, state.layout, IS_DOWN);
	return 0;
}

static int on_key_up(int key)
{
	printf("%s %d\n", __FUNCTION__, key);
	switch (state.layout) {
	case LAYOUT_HIRAGANA_DEFAULT:
		state.previous_layout = state.layout;
		state.layout = layout_switch[state.key_pressed];
		printf("%s layout %d -> %d\n", __FUNCTION__, state.previous_layout, state.layout);
		if (state.key_pressed == SOUND_MARKS_KEY) {
			change_with_sound_mark(0);
			draw_symbol_from_char_code(fb, CURSOR_X(), CURSOR_Y(),
						   GREEN, BLACK,
						   text_buffer[state.cursor],
						   IS_SELECTED);
		}
		draw_keyboard(fb, KB_X, KB_Y, GREEN, BLACK, state.layout);
		break;
	case LAYOUT_KATAKANA_DEFAULT:
		state.previous_layout = state.layout;
		state.layout =
			layout_to_katakana(layout_switch[state.key_pressed]);
		printf("%s layout %d -> %d\n", __FUNCTION__, state.previous_layout, state.layout);
		if (state.key_pressed == SOUND_MARKS_KEY) {
			change_with_sound_mark(0x30A0 - 0x3040);
			draw_symbol_from_char_code(fb, CURSOR_X(), CURSOR_Y(),
						   GREEN, BLACK,
						   text_buffer[state.cursor],
						   IS_SELECTED);
		}
		draw_keyboard(fb, KB_X, KB_Y, GREEN, BLACK, state.layout);
		break;
	default:
		printf("[%d] %d,%d\n", state.cursor, CURSOR_X(), CURSOR_Y());
		if (text_buffer[state.cursor] != 0)
			draw_symbol_from_char_code(fb, CURSOR_X(), CURSOR_Y(),
						   GREEN, BLACK,
						   text_buffer[state.cursor++],
						   IS_NOT_SELECTED);
		printf("[%d] %d,%d\n", state.cursor, CURSOR_X(), CURSOR_Y());
		draw_symbol_from_key(fb, CURSOR_X(), CURSOR_Y(), GREEN, BLACK,
				     state.key_pressed, state.layout,
				     IS_SELECTED);
		text_buffer[state.cursor] =
			kb_layouts[state.layout].codes[state.key_pressed];
		printf("[%d] %d,%d %%/ %d\n", state.cursor, CURSOR_X(), CURSOR_Y(), state.cursor % 8);
		if (state.layout != state.previous_layout) {
			state.layout = state.previous_layout;
			draw_keyboard(fb, KB_X, KB_Y, GREEN, BLACK, state.layout);
		}
	}
	state.key_pressed = -1;
	return 0;
}

typedef int (*ev_handler_t)(int);
ev_handler_t handlers[2] = {
	on_key_up,
	on_key_down
};
static void sigint_handler(int signum)
{
	release_fb(fb);
	release_keypad(kp);
}

int main(int argc, char *argv[])
{
	struct kp_event e;
	signal(SIGINT, sigint_handler);
	fb = grab_fb("/dev/fb1");
	if (fb == NULL)
		return -1;
	kp = grab_keypad("/dev/input/event0");
	if (kp == -1) {
		release_fb(fb);
		return -1;
	}
	clear_screen(fb, BLACK);
	draw_keyboard(fb, KB_X, KB_Y, GREEN, BLACK, state.layout);
	while (1) {
		if (read_event(kp, (struct input_event *)&e, KP_EV_SIZE))
			break;
		handlers[e.key.value](e.scan.value);
		wait_for_sync(fb);
	}
	release_fb(fb);
	release_keypad(kp);
	return 0;
		//draw_symbol_from_char_code(fb, 1, 1, GREEN, BLACK, HIRAGANA_LETTER_KO,
		//			   IS_NOT_SELECTED);
		//draw_symbol_from_char_code(fb, 13, 1, GREEN, BLACK, HIRAGANA_LETTER_N,
		//			   IS_NOT_SELECTED);
		//draw_symbol_from_char_code(fb, 25, 1, GREEN, BLACK, HIRAGANA_LETTER_NI,
		//			   IS_NOT_SELECTED);
		//draw_symbol_from_char_code(fb, 37, 1, GREEN, BLACK, HIRAGANA_LETTER_TI,
		//			   IS_NOT_SELECTED);
		//draw_symbol_from_char_code(fb, 49, 1, GREEN, BLACK, HIRAGANA_LETTER_HA,
		//			   IS_NOT_SELECTED);
		//draw_symbol_from_char_code(fb, 61, 1, GREEN, BLACK,
		//			   IDEOGRAPHIC_FULL_STOP, IS_NOT_SELECTED);
}
