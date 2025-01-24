#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static int **create_slot(size_t num_of_lines, size_t num_of_columns)
{
	int **slot = malloc(sizeof(int *) * num_of_lines);

	for(size_t i = 0; i < num_of_lines; ++i) {
		slot[i] = malloc(sizeof(int) * num_of_columns);

		printf("Insira os valores da linha %lu:\n", i+1);

		for(size_t j = 0; j < num_of_columns; ++j) {
			scanf("%i", &slot[i][j]);
		}
	}

	return slot;
}

static int **create_prize(size_t num_of_lines, size_t num_of_columns)
{
	int **premio = malloc(sizeof(int *) * num_of_lines);

	for(size_t i = 0; i < num_of_lines; ++i) {
		premio[i] = calloc(num_of_columns, sizeof(int));
	}

	//números mágicos somente nessa configuração
	assert(num_of_lines == 3);
	assert(num_of_columns == 5);

	/*
	[1, 0, 0, 0, 1]
	[0, 1, 0, 1, 0]
	[0, 0, 1, 0, 0]
	*/

	premio[0][0] = 1;
	premio[0][4] = 1;

	premio[1][1] = 1;
	premio[1][3] = 1;

	premio[2][2] = 1;

	return premio;
}

int main(int argc, char *argv[argc])
{
	const size_t num_of_lines = 3;
	const size_t num_of_columns = 5;

	int **slot = create_slot(num_of_lines, num_of_columns);
	int **prize = create_prize(num_of_lines, num_of_columns);

	for(size_t i = 0; i < num_of_lines; ++i) {
		size_t num_equal = 0;
		for(size_t j = 0; j < num_of_columns; ++j) {
			if(slot[i][j] == prize[i][j]) {
				++num_equal;
			}
		}
		if(num_equal == num_of_columns) {
			puts("Ganhou!");
		}
	}

	for(size_t i = 0; i < num_of_lines; ++i) {
		free(prize[i]);
		free(slot[i]);
	}

	free(prize);
	free(slot);

	return EXIT_SUCCESS;
}
