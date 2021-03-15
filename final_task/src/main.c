#include "evdev_manip.h"
#include "fb_manip.h"
#include <signal.h>

// SCREEN_WIDTH / 20
#define TEXT_BUFFER_VISIBLE_WIDTH 8
//#define TEXT_BUFFER_VISIBLE_HEIGHT SCREEN_HEIGHT / 10
#define TEXT_BUFFER_VISIBLE_HEIGHT 5
#define TEXT_BUFFER_VISIBLE_LENGTH                                             \
	TEXT_BUFFER_VISIBLE_HEIGHT *TEXT_BUFFER_VISIBLE_WIDTH
#define KB_X 91
#define KB_Y 35

struct kp_event {
	struct input_event scan;
	struct input_event key;
	struct input_event sync;
};
#define KP_EV_SIZE sizeof(struct input_event) * 3
struct kb_state_t {
	int layout; // 0-23
	int base_layout; // 0-23
	int key_pressed; // 0-15 or -1 for none
	int cursor;
};
static struct kb_state_t state = { .layout = LAYOUT_HIRAGANA_DEFAULT,
				   .base_layout = LAYOUT_HIRAGANA_DEFAULT,
				   .key_pressed = -1,
				   .cursor = 0 };

struct dyn_buffer {
	size_t size;
	size_t max_size;
	size_t fvs; // First visible symbol
	__u16 *data;
} static text_buffer;

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
extern inline void draw_keyboard(struct fb_manip *fb, int x, int y, __u16 fg,
				 __u16 bg, int layout);
extern inline void draw_key(struct fb_manip *fb, int x, int y, __u16 fg,
			    __u16 bg, int key, int layout, int down);
extern inline void drawrect(struct fb_manip *fb, int x, int y, int w, int h,
			    __u16 c);
extern inline void wait_for_sync(struct fb_manip *fb);

#define draw_current_layout()                                                  \
	draw_keyboard(fb, KB_X, KB_Y, GREEN, BLACK, state.layout)

extern inline int read_event(int fd, struct input_event *e, size_t ev_size);

inline int CURSOR_X(int pos)
{
	return pos % TEXT_BUFFER_VISIBLE_WIDTH * 10;
}
inline int CURSOR_Y(int pos)
{
	return pos / TEXT_BUFFER_VISIBLE_WIDTH * 10;
}
extern inline int CURSOR_X(int pos);
extern inline int CURSOR_Y(int pos);

#define CURSOR_IN_BUFFER (state.cursor + text_buffer.fvs)

static void change_with_sound_mark(int shift)
{
	if (text_buffer.data[CURSOR_IN_BUFFER] > 0x3040 + shift) {
		text_buffer.data[CURSOR_IN_BUFFER] -= shift;
		if (text_buffer.data[CURSOR_IN_BUFFER] <
		    HIRAGANA_LETTER_SMALL_TU) {
			if (text_buffer.data[CURSOR_IN_BUFFER] & 1)
				text_buffer.data[CURSOR_IN_BUFFER] += 1;
			else
				text_buffer.data[CURSOR_IN_BUFFER] -= 1;
		} else if (text_buffer.data[CURSOR_IN_BUFFER] <
			   HIRAGANA_LETTER_TE) {
			if (text_buffer.data[CURSOR_IN_BUFFER] ==
			    HIRAGANA_LETTER_SMALL_TU)
				text_buffer.data[CURSOR_IN_BUFFER] =
					HIRAGANA_LETTER_DU;
			else
				text_buffer.data[CURSOR_IN_BUFFER] -= 1;
		} else if (text_buffer.data[CURSOR_IN_BUFFER] <
			   HIRAGANA_LETTER_NA) {
			if (text_buffer.data[CURSOR_IN_BUFFER] & 1)
				text_buffer.data[CURSOR_IN_BUFFER] -= 1;
			else
				text_buffer.data[CURSOR_IN_BUFFER] += 1;
		} else if (text_buffer.data[CURSOR_IN_BUFFER] <
				   HIRAGANA_LETTER_MA &&
			   text_buffer.data[CURSOR_IN_BUFFER] >
				   HIRAGANA_LETTER_NO) {
			if ((text_buffer.data[CURSOR_IN_BUFFER] -
			     HIRAGANA_LETTER_HA) %
				    3 ==
			    2) {
				text_buffer.data[CURSOR_IN_BUFFER] -= 2;
			} else {
				text_buffer.data[CURSOR_IN_BUFFER] += 1;
			}
		} else if (text_buffer.data[CURSOR_IN_BUFFER] <
				   HIRAGANA_LETTER_RA &&
			   text_buffer.data[CURSOR_IN_BUFFER] >
				   HIRAGANA_LETTER_MO) {
			if (text_buffer.data[CURSOR_IN_BUFFER] & 1)
				text_buffer.data[CURSOR_IN_BUFFER] += 1;
			else
				text_buffer.data[CURSOR_IN_BUFFER] -= 1;
		}
		text_buffer.data[CURSOR_IN_BUFFER] += shift;
	}
}

static int put_symbol_after_cursor(int code)
{
	state.cursor++;
	text_buffer.size++;
	if (text_buffer.size > text_buffer.max_size) {
		text_buffer.max_size += TEXT_BUFFER_VISIBLE_LENGTH;
		text_buffer.data = realloc(text_buffer.data,
					   text_buffer.max_size *
						   sizeof(*text_buffer.data));
	}
	for (int i = text_buffer.size - 1; i > CURSOR_IN_BUFFER; --i) {
		text_buffer.data[i] = text_buffer.data[i - 1];
	}
	if (text_buffer.size == 1)
		state.cursor = 0;
	text_buffer.data[CURSOR_IN_BUFFER] = code;

	if (state.cursor == TEXT_BUFFER_VISIBLE_LENGTH) {
		text_buffer.fvs++;
		state.cursor--;
		return 1;
	}
	return 0;
}

static int delete_symbol_under_cursor(void)
{
	if (text_buffer.size <= 0)
		return 1;
	text_buffer.size--;
	for (int i = CURSOR_IN_BUFFER; i < text_buffer.size; ++i) {
		text_buffer.data[i] = text_buffer.data[i + 1];
	}
	text_buffer.data[text_buffer.size] = 0;
	if (CURSOR_IN_BUFFER == text_buffer.size)
		state.cursor--;
	if (state.cursor < 0) {
		state.cursor = 0;
		if (text_buffer.fvs != 0) {
			text_buffer.fvs--;
			return 0;
		}
		return 1;
	}
	return 0;
}

static int move_cursor_left(void)
{
	if (CURSOR_IN_BUFFER != 0) {
		if (state.cursor != 0) {
			state.cursor--;
		} else if (text_buffer.fvs != 0) {
			text_buffer.fvs--;
			return 1;
		}
	}
	return 0;
}

static int move_cursor_right(void)
{
	if (CURSOR_IN_BUFFER != text_buffer.size - 1) {
		if (state.cursor != TEXT_BUFFER_VISIBLE_LENGTH - 1) {
			state.cursor++;
		} else if (text_buffer.fvs != text_buffer.size - 1) {
			text_buffer.fvs++;
			return 1;
		}
	}
	return 0;
}

static void redraw_full_buffer(void)
{
	for (int i = 0; i < TEXT_BUFFER_VISIBLE_LENGTH; ++i) {
		draw_symbol_from_char_code(
			fb, CURSOR_X(i), CURSOR_Y(i), GREEN, BLACK,
			text_buffer.data[text_buffer.fvs + i], IS_NOT_SELECTED);
	}
	draw_symbol_from_char_code(
		fb, CURSOR_X(state.cursor), CURSOR_Y(state.cursor), GREEN,
		BLACK, text_buffer.data[text_buffer.fvs + state.cursor],
		IS_SELECTED);
}

static void redraw_buffer_from_cursor(void)
{
	draw_symbol_from_char_code(fb, CURSOR_X(state.cursor),
				   CURSOR_Y(state.cursor), GREEN, BLACK,
				   text_buffer.data[CURSOR_IN_BUFFER],
				   IS_SELECTED);
	for (int i = state.cursor + 1; i < TEXT_BUFFER_VISIBLE_LENGTH; ++i) {
		if (text_buffer.data[text_buffer.fvs + i] != 0) {
			draw_symbol_from_char_code(
				fb, CURSOR_X(i), CURSOR_Y(i), GREEN, BLACK,
				text_buffer.data[text_buffer.fvs + i],
				IS_NOT_SELECTED);
		} else {
			draw_symbol_from_char_code(fb, CURSOR_X(i), CURSOR_Y(i),
						   BLACK, BLACK,
						   IDEOGRAPHIC_SPACE,
						   IS_NOT_SELECTED);
			return;
		}
	}
}

static int on_key_down(int key)
{
	printf("%s %d\n", __FUNCTION__, key);
	if (state.key_pressed == -1)
		state.key_pressed = key;
	draw_key(fb, KB_X, KB_Y, GREEN, BLACK, key, state.layout, IS_DOWN);
	return 0;
}

static int on_key_up(int key)
{
	printf("%s %d\n", __FUNCTION__, key);
	if (key != state.key_pressed)
		return 0;
	switch (key) {
	case KATAKANA_HIRAGANA_KEY:
		state.base_layout = LAYOUT_KATAKANA_DEFAULT ^ state.base_layout;
		state.layout = state.layout % LAYOUT_KATAKANA_DEFAULT +
			       state.base_layout;
		goto finilize;
	case BACKSPACE_KEY:
		if (state.layout != state.base_layout) {
			state.layout = state.base_layout;
			goto finilize;
		}
		if (delete_symbol_under_cursor()) {
			draw_symbol_from_char_code(fb, CURSOR_X(0), CURSOR_Y(0),
						   BLACK, BLACK,
						   IDEOGRAPHIC_SPACE,
						   IS_NOT_SELECTED);
		} else {
			redraw_buffer_from_cursor();
		}
		goto finilize;
	case LEFT_KEY:
		if (state.layout != state.base_layout) {
			state.layout = state.base_layout;
			goto finilize;
		}
		if (move_cursor_left()) {
			redraw_buffer_from_cursor();
		} else {
			if (text_buffer.size > 1)
				draw_symbol_from_char_code(
					fb, CURSOR_X(state.cursor + 1),
					CURSOR_Y(state.cursor + 1), GREEN,
					BLACK,
					text_buffer.data[CURSOR_IN_BUFFER + 1],
					IS_NOT_SELECTED);
			draw_symbol_from_char_code(
				fb, CURSOR_X(state.cursor),
				CURSOR_Y(state.cursor), GREEN, BLACK,
				text_buffer.data[CURSOR_IN_BUFFER],
				IS_SELECTED);
		}
		goto finilize;
	case RIGHT_KEY:
		if (state.layout != state.base_layout) {
			state.layout = state.base_layout;
			goto finilize;
		}
		if (move_cursor_right()) {
			redraw_full_buffer();
		} else {
			if (text_buffer.size > 1)
				draw_symbol_from_char_code(
					fb, CURSOR_X(state.cursor - 1),
					CURSOR_Y(state.cursor - 1), GREEN,
					BLACK,
					text_buffer.data[CURSOR_IN_BUFFER - 1],
					IS_NOT_SELECTED);
			draw_symbol_from_char_code(
				fb, CURSOR_X(state.cursor),
				CURSOR_Y(state.cursor), GREEN, BLACK,
				text_buffer.data[CURSOR_IN_BUFFER],
				IS_SELECTED);
		}
		goto finilize;
	default:
		break;
	}
	switch (state.layout) {
	case LAYOUT_HIRAGANA_DEFAULT:
		state.base_layout = state.layout;
		state.layout = layout_switch[state.key_pressed];
		if (state.key_pressed == SOUND_MARKS_KEY) {
			change_with_sound_mark(0);
			draw_symbol_from_char_code(
				fb, CURSOR_X(state.cursor),
				CURSOR_Y(state.cursor), GREEN, BLACK,
				text_buffer.data[CURSOR_IN_BUFFER],
				IS_SELECTED);
		}
		break;
	case LAYOUT_KATAKANA_DEFAULT:
		state.base_layout = state.layout;
		state.layout =
			layout_to_katakana(layout_switch[state.key_pressed]);
		if (state.key_pressed == SOUND_MARKS_KEY) {
			change_with_sound_mark(0x30A0 - 0x3040);
			draw_symbol_from_char_code(
				fb, CURSOR_X(state.cursor),
				CURSOR_Y(state.cursor), GREEN, BLACK,
				text_buffer.data[CURSOR_IN_BUFFER],
				IS_SELECTED);
		}
		break;
	default:
		if (kb_layouts[state.layout].codes[state.key_pressed] == 0)
			break;
		if (put_symbol_after_cursor(
			    kb_layouts[state.layout].codes[key])) {
			redraw_full_buffer();
		} else {
			if (text_buffer.size > 1) {
				draw_symbol_from_char_code(
					fb, CURSOR_X(state.cursor - 1),
					CURSOR_Y(state.cursor - 1), GREEN,
					BLACK,
					text_buffer.data[CURSOR_IN_BUFFER - 1],
					IS_NOT_SELECTED);
				redraw_buffer_from_cursor();
			} else {
				draw_symbol_from_char_code(
					fb, CURSOR_X(state.cursor),
					CURSOR_Y(state.cursor), GREEN, BLACK,
					text_buffer.data[CURSOR_IN_BUFFER],
					IS_SELECTED);
			}
		}
		if (state.layout != state.base_layout) {
			state.layout = state.base_layout;
			draw_keyboard(fb, KB_X, KB_Y, GREEN, BLACK,
				      state.layout);
		}
	}

finilize:
	draw_current_layout();
	state.key_pressed = -1;
	return 0;
}

static void init_buffer(void)
{
	text_buffer.max_size = TEXT_BUFFER_VISIBLE_LENGTH;
	text_buffer.data =
		malloc(TEXT_BUFFER_VISIBLE_LENGTH * sizeof(*text_buffer.data));
}

static void redefine_missing_glyphs()
{
	memset(IDEOGRAPHIC_SPACE_xbm10, 0, sizeof(IDEOGRAPHIC_SPACE_xbm10));
	memset(WAVE_DASH_xbm10, 0, sizeof(WAVE_DASH_xbm10));
	((__u16 *)WAVE_DASH_xbm10)[4] = 0x010C;
	((__u16 *)WAVE_DASH_xbm10)[5] = 0x0092;
	((__u16 *)WAVE_DASH_xbm10)[6] = 0x0061;
	for (int i = 0; i < sizeof(WAVE_DASH_xbm10) >> 1; ++i) {
		((__u16 *)WAVE_DASH_xbm14)[i + 2] &=
			0x2001 ^ (((__u16 *)WAVE_DASH_xbm10)[i] << 2);
	}
}

static void print_hello(void)
{
	put_symbol_after_cursor(HIRAGANA_LETTER_MI);
	put_symbol_after_cursor(HIRAGANA_LETTER_N);
	put_symbol_after_cursor(HIRAGANA_LETTER_NA);
	put_symbol_after_cursor(HIRAGANA_LETTER_SA);
	put_symbol_after_cursor(HIRAGANA_LETTER_N);
	put_symbol_after_cursor(IDEOGRAPHIC_COMMA);
	put_symbol_after_cursor(HIRAGANA_LETTER_KO);
	put_symbol_after_cursor(HIRAGANA_LETTER_N);
	put_symbol_after_cursor(HIRAGANA_LETTER_NI);
	put_symbol_after_cursor(HIRAGANA_LETTER_TI);
	put_symbol_after_cursor(HIRAGANA_LETTER_HA);
	put_symbol_after_cursor(IDEOGRAPHIC_FULL_STOP);
}

typedef int (*ev_handler_t)(int);
ev_handler_t handlers[2] = { on_key_up, on_key_down };

int main(int argc, char *argv[])
{
	struct kp_event e;
	fb = grab_fb("/dev/fb1");
	if (fb == NULL)
		return -1;
	kp = grab_keypad("/dev/input/event0");
	if (kp == -1) {
		release_fb(fb);
		return -1;
	}
	init_buffer();
	redefine_missing_glyphs();
	clear_screen(fb, BLACK);
	draw_keyboard(fb, KB_X, KB_Y, GREEN, BLACK, state.layout);
	print_hello();
	state.cursor = 0;
	redraw_buffer_from_cursor();
	while (1) {
		if (read_event(kp, (struct input_event *)&e, KP_EV_SIZE))
			break;
		handlers[e.key.value](e.scan.value);
		wait_for_sync(fb);
	}
	release_fb(fb);
	release_keypad(kp);
	free(text_buffer.data);
	return 0;
}
