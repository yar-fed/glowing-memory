#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char eof = EOF;

void put_encoded_ch(unsigned char, unsigned char);

int main(void)
{
	unsigned char c = fgetc(stdin);

	while (!feof(stdin)) {
		unsigned char c2;
		unsigned char repeats = 1;

		while (c == (c2 = fgetc(stdin)) && !feof(stdin)) {
			if (++repeats == 255) {
				put_encoded_ch(c, 254);
				repeats = 1;
			}
		}
		if (repeats > 0)
			put_encoded_ch(c, repeats);
		if (feof(stdin))
			return 0;
		c = c2;
	}
	return 0;
}

void put_encoded_ch(unsigned char c, unsigned char n)
{
	fputc(c, stdout);
	fputc(n, stdout);
}

