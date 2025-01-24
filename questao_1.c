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

typedef long long off_t;
#endif

#define UTF8_MASK      0xFFFFFF
#define UTF8_MAGIC     0xBFBBEF

#define UTF16_MASK     0xFFFF
#define UTF16_LE_MAGIC 0xFEFF
#define UTF16_BE_MAGIC 0xFFFE

#define UTF32_MASK     0xFFFFFFFF
#define UTF32_LE_MAGIC 0x0000FEFF
#define UTF32_BE_MAGIC 0xFFFE0000

#define PRIMARY_FILE "questao_1_strings.txt"
#define SECONDARY_FILE "strings_01.txt"

static void print_help(const char *program)
{
	printf(
		"Usage: %s [OPÇOES]... [ARQUIVO]\n"
		"Encontra políndromos em um arquivo de texto\n"
		"Se um arquivo não for especifica procura por '" PRIMARY_FILE "'\n"
		"Se não for encontrado procura por '" SECONDARY_FILE "'\n"
		"OPÇOES:\n"
		"  -h, --help            exibe esta mensagem\n"
		, program
	);
}

int main(int argc, char *argv[argc])
{
	//TODO!!! extrair somente o nome do programa
	const char *program = argv[0];

	if(argc > 3) {
		print_help(program);
		return EXIT_FAILURE;
	}

	const char *filename = NULL;

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
			filename = argv[i];
		}
	}

	if(!filename) {
		filename = PRIMARY_FILE;
	}

	FILE *strs_file = fopen(filename, "rb");
	if(!strs_file) {
		const char *errstr = strerror(errno);
		printf("falha ao abrir arquivo de texto: '%s': %s\n", filename, errstr);

		if(argc == 1) {
			filename = SECONDARY_FILE;
			printf("tentando arquivo secundário: '%s'\n", filename);

			strs_file = fopen(filename, "rb");
			if(!strs_file) {
				const char *errstr = strerror(errno);
				printf("falha ao abrir arquivo de texto secundário: '%s': %s\n", filename, errstr);
				return EXIT_FAILURE;
			}
		} else {
			return EXIT_FAILURE;
		}
	}

	unsigned int BOM_bytes;
	size_t bom_read = fread(&BOM_bytes, sizeof(unsigned char), 4, strs_file);
	if(bom_read == 0) {
		if(ferror(strs_file) != 0) {
			const char *errstr = strerror(errno);
			printf("falha ao ler arquivo de texto: '%s': %s\n", filename, errstr);
			return EXIT_FAILURE;
		} {
			printf("'%s': aviso: arquivo vazio\n", filename);
			return EXIT_SUCCESS;
		}
	}

	off_t last_offset;

	if( (BOM_bytes & UTF8_MASK ) == UTF8_MAGIC) {
		printf("'%s': aviso: somente ASCII e suportado\n", filename);
		last_offset = 3;
	} else if(
		(BOM_bytes & UTF16_MASK) == UTF16_LE_MAGIC ||
		(BOM_bytes & UTF16_MASK) == UTF16_BE_MAGIC ||
		(BOM_bytes & UTF32_MASK) == UTF32_LE_MAGIC ||
		(BOM_bytes & UTF32_MASK) == UTF32_BE_MAGIC) {
		printf("'%s': error: somente ASCII e suportado\n", filename);
		return EXIT_FAILURE;
	} else {
		last_offset = 0;
	}

	puts(
		"formato:\n"
		"$1 - [$2, $3] - $4\n"
		"  $1 == palavra\n"
		"  $2 == numero de caracteres políndromo encontrados\n"
		"  $3 == total de caracteres políndromo possíveis\n"
		"  $4 == a palavra e totalmente um políndromo? ($2 == $3)"
	);

	size_t total_strs = 0;
	size_t total_polindrome = 0;

	for(;;) {
		int c_or_err = fgetc(strs_file);
		if(c_or_err == EOF && ferror(strs_file) != 0) {
			const char *errstr = strerror(errno);
			printf("falha ao ler arquivo de texto: '%s': %s\n", filename, errstr);
			fclose(strs_file);
			return EXIT_FAILURE;
		}

		bool is_eof = (c_or_err == EOF);
		char c = (char)c_or_err;

		if(c == '\n' || is_eof) {
			off_t pos = ftell(strs_file);
			if(pos == -1l) {
				const char *errstr = strerror(errno);
				printf("falha ao ler arquivo de texto: '%s': %s\n", filename, errstr);
				fclose(strs_file);
				return EXIT_FAILURE;
			}

			ssize_t maybe_len = (ssize_t)pos - last_offset;
			if(c == '\n') {
				--maybe_len;
			}
			if(maybe_len < 0) {
				printf("falha ao ler arquivo de texto: '%s'\n", filename);
				fclose(strs_file);
				return EXIT_FAILURE;
			}

			size_t len = (size_t)maybe_len;
			if(len < 2) {
				last_offset = pos;
				if(is_eof) {
					break;
				} else {
					continue;
				}
			}

			char *str = (char *)malloc(sizeof(char) * (len+1));
			if(!str) {
				puts("falha ao alocar memoria");
				fclose(strs_file);
				return EXIT_FAILURE;
			}

			if(fseek(strs_file, last_offset, SEEK_SET) != 0) {
				printf("falha ao ler arquivo de texto: '%s'\n", filename);
				fclose(strs_file);
				return EXIT_FAILURE;
			}
			ssize_t read = fread(str, sizeof(char), len, strs_file);
			if(ferror(strs_file) != 0) {
				const char *errstr = strerror(errno);
				printf("falha ao ler arquivo de texto: '%s': %s\n", filename, errstr);
				fclose(strs_file);
				return EXIT_FAILURE;
			}
			if(fseek(strs_file, pos, SEEK_SET) != 0) {
				printf("falha ao ler arquivo de texto: '%s'\n", filename);
				fclose(strs_file);
				return EXIT_FAILURE;
			}

			last_offset = pos;

			if(read != (len+1)) {
				str = (char *)realloc(str, sizeof(char) * (read+1));
				if(!str) {
					puts("falha ao alocar memoria");
					fclose(strs_file);
					return EXIT_FAILURE;
				}
			}

			str[read] = '\0';

			++total_strs;

			const size_t half_len = (read / 2);

			size_t palindrome_len = 0;

			for(int i = 0, j = read-1; i < half_len && j >= 0; ++i, --j) {
				if(tolower(str[i]) == tolower(str[j])) {
					++palindrome_len;
				} else {
					break;
				}
			}

			bool is_palindrome = (palindrome_len == half_len);

			if(is_palindrome) {
				++total_polindrome;
			}

			printf(
				"%s - [%lu, %lu] - %s\n",
				str, palindrome_len, half_len, is_palindrome ? "true" : "false"
			);

			free(str);

			if(is_eof) {
				break;
			}
		}
	}

	fclose(strs_file);

	if(total_strs == 0) {
		printf("'%s': aviso: arquivo vazio\n", filename);
		return EXIT_SUCCESS;
	}

	printf(
		"'%s': de %lu palavras encontradas %lu são políndromo\n",
		filename, total_strs, total_polindrome
	);

	return EXIT_SUCCESS;
}
