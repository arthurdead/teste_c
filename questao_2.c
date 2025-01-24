//c standard
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/*
programa não testado no windows
tanto execução como compilação
*/

//não sei se essas declarações estão disponíveis no windows
#ifndef __linux__
#ifdef _M_X64
typedef long long ssize_t;
#else
typedef int ssize_t;
#endif
#endif

/*
BUGS:

XVX, IXV e similares
são considerados números validos
ate onde eu sei esta syntaticamente correto so não logicamente correto
não sei o que fazer sobre
*/

enum : unsigned char
{
	FLAG_SUBTRACTION = (1 << 0),
	FLAG_ADDITION =    (1 << 1),
	FLAG_REPETITION =  (1 << 2),
};

struct token
{
	unsigned short value : 10;
	unsigned char flags : 3;
};

typedef struct token token_t;

static const unsigned short INVALID_ROMAN_CHAR = (unsigned short)-1;

static unsigned short value_for_roman(char c)
{
	c = (char)toupper((int)c);

	switch(c) {
	case 'I': return 1;
	case 'V': return 5;
	case 'X': return 10;
	case 'L': return 50;
	case 'C': return 100;
	case 'D': return 500;
	case 'M': return 1000;
	default: return INVALID_ROMAN_CHAR;
	}
}

//TODO!!!! usar a livraria curses para um TUI mais bonito

static void print_diagnostic(size_t len, char output[len], size_t idx, size_t range)
{
	for(size_t i = 0; i < idx; ++i) {
		output[i] = '.';
	}
	output[idx] = '^';
	for(size_t i = idx+1; i < len; ++i) {
		if(i <= range || value_for_roman(output[i]) == INVALID_ROMAN_CHAR) {
			output[i] = '^';
		} else {
			output[i] = '.';
		}
	}
	printf("%s: numero romano invalido\n", output);
}

static void print_help(const char *program)
{
	printf(
		"Usage: %s [OPÇOES]...\n"
		"Converte números romanos escritos no input para decimal\n"
		"OPÇOES:\n"
		"  -h, --help            exibe esta mensagem\n"
		, program
	);
}

int main(int argc, char *argv[argc])
{
	//TODO!!! extrair somente o nome do programa
	const char *program = argv[0];

	if(argc > 2) {
		print_help(program);
		return EXIT_FAILURE;
	}

	for(size_t i = 1; i < argc; ++i) {
		if(strcmp(argv[i], "--help") == 0 ||
			strcmp(argv[i], "-h") == 0) {
			print_help(program);
			return EXIT_SUCCESS;
		} else if(strncmp(argv[i], "--", 2) == 0 ||
					strncmp(argv[i], "-", 1) == 0) {
			print_help(program);
			return EXIT_FAILURE;
		} else {
			print_help(program);
			return EXIT_FAILURE;
		}
	}

	puts(
		"escreva um numero romano e ele sera convertido para decimal\n"
		"aperte ESC e ENTER para sair do programa\n"
		"ou escreva um dos comandos: quit, q, exit, e, sair, s"
	);

	const size_t stdin_cache_grow = 16;

	size_t stdin_cache_capacity = 32;
	char *stdin_cache = (char *)malloc(sizeof(char) * stdin_cache_capacity);
	size_t stdin_cache_length = 0;

	for(;;) {
		int c_or_err = fgetc(stdin);
		if(c_or_err == EOF) {
			puts("falha ao ler input");
			free(stdin_cache);
			return EXIT_FAILURE;
		}

		char c = (char)c_or_err;
		if(c == '\n') {
			stdin_cache[stdin_cache_length] = '\0';

			if(stdin_cache_length == 0) {
				puts("input vazio");
				continue;
			}

			if(strcmp(stdin_cache,"\033") == 0 ||
				strcmp(stdin_cache,"quit") == 0 ||
				strcmp(stdin_cache,"q") == 0 ||
				strcmp(stdin_cache,"exit") == 0 ||
				strcmp(stdin_cache,"e") == 0 ||
				strcmp(stdin_cache,"sair") == 0 ||
				strcmp(stdin_cache,"s") == 0) {
				break;
			}

			ssize_t decimal = 0;
			bool invalid_number = false;

			token_t *tokens = (token_t *)calloc(stdin_cache_length, sizeof(token_t));

			bool repeat_check = false;

			for(size_t i = 0; i < stdin_cache_length; ++i) {
				unsigned short curr_value = value_for_roman(stdin_cache[i]);
				if(curr_value == INVALID_ROMAN_CHAR) {
					print_diagnostic(stdin_cache_length, stdin_cache, i, 0);
					invalid_number = true;
					break;
				}

				tokens[i].value = curr_value;

				if(i > 0) {
					unsigned short prev_value = value_for_roman(stdin_cache[i-1]);

					if(prev_value == curr_value) {
						if(curr_value == 5 ||
							curr_value == 50 ||
							curr_value == 500) {
							print_diagnostic(stdin_cache_length, stdin_cache, i-1, i);
							invalid_number = true;
							break;
						}

						if(repeat_check) {
							print_diagnostic(stdin_cache_length, stdin_cache, i-1, i);
							invalid_number = true;
							break;
						}

						tokens[i-1].flags |= FLAG_REPETITION;
						tokens[i].flags |= FLAG_REPETITION;
					}

					if(prev_value < curr_value) {
						tokens[i-1].flags |= FLAG_SUBTRACTION;
						tokens[i].flags |= FLAG_SUBTRACTION;
					}

					if(prev_value >= curr_value) {
						tokens[i-1].flags |= FLAG_ADDITION;
						tokens[i].flags |= FLAG_ADDITION;
					}
				}

				if(i < (stdin_cache_length-1)) {
					unsigned short next_value = value_for_roman(stdin_cache[i+1]);
					if(next_value == INVALID_ROMAN_CHAR) {
						print_diagnostic(stdin_cache_length, stdin_cache, i+1, 0);
						invalid_number = true;
						break;
					}

					if(next_value == curr_value) {
						if(curr_value == 5 ||
							curr_value == 50 ||
							curr_value == 500) {
							print_diagnostic(stdin_cache_length, stdin_cache, i, i+1);
							invalid_number = true;
							break;
						}

						if(repeat_check) {
							print_diagnostic(stdin_cache_length, stdin_cache, i, i+1);
							invalid_number = true;
							break;
						}

						tokens[i+1].flags |= FLAG_REPETITION;
						tokens[i].flags |= FLAG_REPETITION;
					}

					if(next_value > curr_value) {
						tokens[i+1].flags |= FLAG_SUBTRACTION;
						tokens[i].flags |= FLAG_SUBTRACTION;
					}

					if(next_value <= curr_value) {
						tokens[i+1].flags |= FLAG_ADDITION;
						tokens[i].flags |= FLAG_ADDITION;
					}
				}

				if(i >= 2) {
					if(
						(tokens[i-2].flags & FLAG_REPETITION) != 0 &&
						(tokens[i-1].flags & FLAG_REPETITION) != 0 &&
						(tokens[i  ].flags & FLAG_REPETITION) != 0
					) {
						if(!repeat_check) {
							repeat_check = true;
							continue;
						} else {
							print_diagnostic(stdin_cache_length, stdin_cache, i-2, i);
							invalid_number = true;
							break;
						}
					}
				}

				if(repeat_check) {
					repeat_check = false;
				}
			}

			for(size_t i = 0; i < stdin_cache_length;) {
				if((tokens[i].flags & FLAG_SUBTRACTION) == 0) {
					if(i < (stdin_cache_length-1)) {
						if((tokens[i+1].flags & FLAG_SUBTRACTION) == 0) {
							decimal += (tokens[i].value + tokens[i+1].value);
							i += 2;
							continue;
						}
					}

					decimal += tokens[i].value;
					++i;
					continue;
				} else if((tokens[i].flags & FLAG_REPETITION) == 0) {
					if(i < (stdin_cache_length-1)) {
						if((tokens[i+1].flags & FLAG_SUBTRACTION) != 0) {
							decimal += (tokens[i+1].value - tokens[i].value);
							i += 2;
							continue;
						}
					}
				}

				print_diagnostic(stdin_cache_length, stdin_cache, i, 0);
				invalid_number = true;
				break;
			}

			free(tokens);

			stdin_cache_length = 0;

			if(!invalid_number) {
				printf("%lu\n", decimal);
			}
		} else {
			stdin_cache[stdin_cache_length++] = c;
			if(stdin_cache_length == stdin_cache_capacity) {
				stdin_cache_capacity += stdin_cache_grow;
				stdin_cache = (char *)realloc(stdin_cache, sizeof(char) * stdin_cache_capacity);
			}
		}
	}

	free(stdin_cache);

	return EXIT_SUCCESS;
}
