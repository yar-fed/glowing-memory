#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char eof = EOF;

int main(void)
{
	int c, n, i;

	while (!feof(stdin)) {
		c = fgetc(stdin);
		n = fgetc(stdin);
		if (c == EOF)
			return 0;
		else if (n == EOF)
			return 1;
		for (i = 0; i < n; ++i)
			fputc(c, stdout);
	}
	return 0;
}

