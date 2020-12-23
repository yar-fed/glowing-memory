/* Игрок бросает две кости и подсчитывает сумму набранных очков. Если она 7 или 11, то он выиграл, если - 2,8,12, то проиграл. Если выпала другая сумма - его пойнт. Если выпадает пойнт, то игрок бросает кости до тех пор, пока не выпадет 7 - это означает проигрыш или пока не выпадет пойнт - игрок выиграл. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int res, point;

	if (argc == 2) {
		point = atoi(argv[0]);
	} else {
		printf("point: ");
		scanf("%d", &point);
	}
	res = (rand() % 12) + (rand() % 12) + 2;
	switch (res) {
	case 7: case 11:
		printf("%d Win!\n", res);
		break;
	case 2: case 8: case 12:
		printf("%d Lose!\n", res);
		break;
	default:
		printf("%d, continuing...\n", res);
		break;
	}
	res = (rand() % 12) + (rand() % 12) + 2;
	if (res == point) {
		printf("%d, rolling for point\n", res);
		res = (rand() % 12) + (rand() % 12) + 2;
		while (res != point && res != 7) {
			printf("%d, continuing...\n", res);
			res = (rand() % 12) + (rand() % 12) + 2;
		}
		if (res == point)
			printf("%d Win!\n", res);
		else
			printf("%d Lose!\n", res);
	} else {
		printf("%d Lose!\n", res);
	}
	return 0;
}

