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

#define MAKE_STR(x) #x

#define DEFAULT_INITIAL_COLOR 0x00001AFF
#define DEFAULT_FINAL_COLOR   0xFF001AFF
#define DEFAULT_STEPS 3

#define DEFAULT_INITIAL_COLOR_STR MAKE_STR(DEFAULT_INITIAL_COLOR)
#define DEFAULT_FINAL_COLOR_STR   MAKE_STR(DEFAULT_FINAL_COLOR)
#define DEFAULT_STEPS_STR MAKE_STR(DEFAULT_STEPS)

static void print_help(const char *program)
{
	printf(
		"Usage: %s [OPÇOES]...\n"
		"Exibe um gradiente RGB em hex\n"
		"OPÇOES:\n"
		"  -i, --cor_inicial[=]<valor>    cor inicial do gradiente [o valor padrão e: '" DEFAULT_INITIAL_COLOR_STR "']\n"
		"  -f, --cor_final[=]<valor>      cor final do gradiente [o valor padrão e: '" DEFAULT_FINAL_COLOR_STR "']\n"
		"  -s, --steps[=]<valor>          numero de passos no gradiente [o valor padrão e: '" DEFAULT_STEPS_STR "']\n"
		"  -h, --help                     exibe esta mensagem\n"
		, program
	);
}

static void expand_color(unsigned int clr, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
	*r = ((clr & 0xFF000000) >> 24);
	*g = ((clr & 0x00FF0000) >> 16);
	*b = ((clr & 0x0000FF00) >> 8);
	*a =  (clr & 0x000000FF);
}

static void pack_color(unsigned int *clr, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	*clr =  (r << 24);
	*clr |= (g << 16);
	*clr |= (b << 8);
	*clr |= a;
}

static char *get_arg_value(size_t *i, size_t equal_off, int argc, char *argv[argc])
{
	if(argv[*i][equal_off] == '=') {
		if(strlen(&argv[*i][equal_off]) == 1) {
			if(*i < argc-1) {
				char *arg_value = argv[*i+1];
				*i = *i+2;
				return arg_value;
			}
		} else {
			char *arg_value = &argv[*i][equal_off+1];
			++*i;
			return arg_value;
		}
	} else {
		if(*i < argc-1) {
			if(strncmp(argv[*i+1], "=", 1) == 0) {
				if(strlen(argv[*i+1]) == 1) {
					if(*i < argc-2) {
						char *arg_value = argv[*i+2];
						*i = *i+3;
						return arg_value;
					}
				} else {
					char *arg_value = &argv[*i+1][1];
					*i = *i+2;
					return arg_value;
				}
			} else {
				char *arg_value = argv[*i+1];
				*i = *i+2;
				return arg_value;
			}
		}
	}
	return NULL;
}

static bool get_int_from_str_hex(char *str, unsigned int *ptr)
{
	size_t len = strlen(str);
	char *str_end = str + len;

	char *read_end = str_end;
	unsigned long value = strtoul(str, &read_end, 16);
	if(errno == ERANGE || read_end != str_end) {
		return false;
	}

	*ptr = (unsigned int)value;
	return true;
}

static bool get_int_from_str_sz(char *str, size_t *ptr)
{
	size_t len = strlen(str);
	char *str_end = str + len;

	char *read_end = str_end;
	unsigned long value = strtoul(str, &read_end, 10);
	if(errno == ERANGE || read_end != str_end) {
		return false;
	}

	*ptr = (size_t)value;
	return true;
}

int main(int argc, char *argv[argc])
{
	//TODO!!! extrair somente o nome do programa
	const char *program = argv[0];

	if(argc > 4) {
		print_help(program);
		return EXIT_FAILURE;
	}

	unsigned int initial_color = DEFAULT_INITIAL_COLOR;
	unsigned int final_color = DEFAULT_FINAL_COLOR;
	size_t steps = DEFAULT_STEPS;

	for(size_t i = 1; i < argc;) {
		if(strcmp(argv[i], "--help") == 0 ||
			strcmp(argv[i], "-h") == 0) {
			print_help(program);
			return EXIT_SUCCESS;
		} else if(strncmp(argv[i], "--cor_inicial", 13) == 0 ||
					strncmp(argv[i], "-i", 2) == 0) {
			size_t equal_off = (argv[i][1] == 'i') ? 2 : 13;
			char *arg_value_str = get_arg_value(&i, equal_off, argc, argv);
			if(!arg_value_str) {
				puts("argumento mal formado para");
				return EXIT_FAILURE;
			}

			if(!get_int_from_str_hex(arg_value_str, &initial_color)) {
				puts("argumento mal formado");
				return EXIT_FAILURE;
			}
		} else if(strncmp(argv[i], "--cor_final", 11) == 0 ||
					strncmp(argv[i], "-f", 2) == 0) {
			size_t equal_off = (argv[i][1] == 'f') ? 2 : 11;
			char *arg_value_str = get_arg_value(&i, equal_off, argc, argv);
			if(!arg_value_str) {
				puts("argumento mal formado");
				return EXIT_FAILURE;
			}

			if(!get_int_from_str_hex(arg_value_str, &final_color)) {
				puts("argumento mal formado");
				return EXIT_FAILURE;
			}
		} else if(strncmp(argv[i], "--steps", 7) == 0 ||
					strncmp(argv[i], "-s", 2) == 0) {
			size_t equal_off = (argv[i][1] == 's') ? 2 : 7;
			char *arg_value_str = get_arg_value(&i, equal_off, argc, argv);
			if(!arg_value_str) {
				puts("argumento mal formado");
				return EXIT_FAILURE;
			}

			if(!get_int_from_str_sz(arg_value_str, &steps)) {
				puts("argumento mal formado");
				return EXIT_FAILURE;
			}

			if(steps == 0) {
				puts("argumento mal formado");
				return EXIT_FAILURE;
			}
		} else {
			print_help(program);
			return EXIT_FAILURE;
		}
	}

	unsigned int curr_color = initial_color;

	unsigned char initial_r, initial_g, initial_b, initial_a;
	expand_color(initial_color, &initial_r, &initial_g, &initial_b, &initial_a);

	unsigned char final_r, final_g, final_b, final_a;
	expand_color(final_color, &final_r, &final_g, &final_b, &final_a);

	bool r_dir = (final_r > initial_r);
	bool g_dir = (final_r > initial_r);
	bool b_dir = (final_r > initial_r);
	bool a_dir = (final_r > initial_r);

	unsigned char r_diff = (r_dir ? (final_r - initial_r) : (initial_r - final_r));
	unsigned char g_diff = (g_dir ? (final_g - initial_g) : (initial_g - final_g));
	unsigned char b_diff = (b_dir ? (final_b - initial_b) : (initial_b - final_b));
	unsigned char a_diff = (a_dir ? (final_a - initial_a) : (initial_a - final_a));

	unsigned char r_step = r_diff / steps;
	unsigned char g_step = g_diff / steps;
	unsigned char b_step = b_diff / steps;
	unsigned char a_step = a_diff / steps;

	printf(
		"%08X[%hhu,%hhu,%hhu,%hhu]\n",
		initial_color, initial_r, initial_g, initial_b, initial_a
	);

	for(size_t i = 0; i < steps; ++i) {
		unsigned char curr_r, curr_g, curr_b, curr_a;
		expand_color(curr_color, &curr_r, &curr_g, &curr_b, &curr_a);

		curr_r += (r_dir ? r_step : -r_step);
		curr_g += (g_dir ? g_step : -g_step);
		curr_b += (b_dir ? b_step : -b_step);
		curr_a += (a_dir ? a_step : -a_step);

		pack_color(&curr_color, curr_r, curr_g, curr_b, curr_a);

		printf(
			"%08X[%hhu,%hhu,%hhu,%hhu]\n",
			curr_color, curr_r, curr_g, curr_b, curr_a
		);
	}

	return EXIT_SUCCESS;
}