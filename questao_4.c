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

static void print_help(const char *program)
{
	printf(
		"Usage: %s [OPÇOES]...\n"
		"Merga listas\n"
		"OPÇOES:\n"
		"  -h, --help                     exibe esta mensagem\n"
		, program
	);
}

enum token_type : unsigned char
{
	TOK_INVALID,
	TOK_OPEN_ARRAY,
	TOK_CLOSE_ARRAY,
	TOK_VALUE,
};

typedef enum token_type token_type_t;

//apenas usado no parser não existe em memoria
//em uma variable separada para evitar warning do compiler
static const token_type_t TOK_COMMA = (token_type_t)-1;

struct token
{
	ssize_t value : 62;
	token_type_t type : 2;
};

typedef struct token token_t;

struct token_vector
{
	token_t *ptr;
	size_t length;
	size_t capacity;
};

typedef struct token_vector token_vec_t;

static const size_t token_vec_grow = 4;

static void token_vec_free(token_vec_t *vec)
{
	free(vec->ptr);
}

static void token_vec_init(token_vec_t *vec)
{
	vec->capacity = token_vec_grow;
	vec->ptr = (token_t *)malloc(sizeof(token_t) * vec->capacity);
	vec->length = 0;
}

static token_t *token_vec_append(token_vec_t *vec)
{
	if(vec->length++ == vec->capacity) {
		vec->capacity += token_vec_grow;
		vec->ptr = (token_t *)realloc(vec->ptr, sizeof(token_t) * vec->capacity);
	}

	return &vec->ptr[vec->length-1];
}

struct value_list_node;
typedef struct value_list_node value_list_node_t;
struct value_list_node
{
	ssize_t value;
	value_list_node_t *next;
};

struct list_list_node;
typedef struct list_list_node list_list_node_t;
struct list_list_node
{
	value_list_node_t *values;
	list_list_node_t *next;
};

static void print_diagnostic(size_t len, char output[len], size_t idx, size_t range)
{
	for(size_t i = 0; i < idx; ++i) {
		output[i] = '.';
	}
	output[idx] = '^';
	for(size_t i = idx+1; i < len; ++i) {
		if(i <= range) {
			output[i] = '^';
		} else {
			output[i] = '.';
		}
	}
	printf("%s: syntax invalido\n", output);
}

static value_list_node_t *parse_values(token_vec_t *tokens, size_t *i)
{
	value_list_node_t *curr = NULL;

	for(; *i < tokens->length;) {
		token_t *tok = &tokens->ptr[*i];
		if(tok->type == TOK_VALUE) {
			if(!curr) {
				curr = (value_list_node_t *)calloc(1, sizeof(value_list_node_t));
				curr->value = tok->value;
			} else {
				value_list_node_t *next = (value_list_node_t *)calloc(1, sizeof(value_list_node_t));
				next->next = curr;
				next->value = tok->value;
				curr = next;
			}
			++*i;
		} else if(tok->type == TOK_CLOSE_ARRAY) {
			++*i;
			return curr;
		} else {
			puts("token invalido tentando parsear list de numeros");
			break;
		}
	}

	puts("tokens terminados tentando ler lista de numeros");

	while(curr) {
		value_list_node_t *next = curr->next;
		free(curr);
		curr = next;
	}

	return NULL;
}

static list_list_node_t *parse_list(token_vec_t *tokens)
{
	if(tokens->length < 2) {
		puts("syntax malformado nao ha tokens suficientes");
		return NULL;
	}

	token_t *tok = &tokens->ptr[0];
	if(tok->type != TOK_OPEN_ARRAY) {
		puts("syntax malformado começo nao e um abrimento de array");
		return NULL;
	}

	list_list_node_t *curr = NULL;

	bool open = false;

	for(size_t i = 1; i < tokens->length;) {
		tok = &tokens->ptr[i];
		if(tok->type == TOK_OPEN_ARRAY) {
			open = true;

			if(!curr) {
				curr = (list_list_node_t *)calloc(1, sizeof(list_list_node_t));
			} else {
				list_list_node_t *next = (list_list_node_t *)calloc(1, sizeof(list_list_node_t));
				next->next = curr;
				curr = next;
			}

			++i;
			value_list_node_t *values = parse_values(tokens, &i);
			if(!values) {
				puts("falha ao ler lista de numeros");
				break;
			}

			curr->values = values;
		} else if(tok->type == TOK_CLOSE_ARRAY) {
			if(!open) {
				puts("syntax malformado fechamento de array sem abrir");
				break;
			}

			if(i != tokens->length-1) {
				puts("syntax malformado fechamento de array nao no final");
				break;
			}

			return curr;
		} else {
			puts("token invalido tentando parsear list de listas");
			break;
		}
	}

	puts("tokens terminados tentando ler lista de listas");

	while(curr) {
		list_list_node_t *next = curr->next;
		free(curr);
		curr = next;
	}

	return NULL;
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
		"escreva listas para mergar\n"
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

			//[[1,3,8],[1,3,6],[8,9]]

			bool invalid_input = false;

			token_vec_t tokens;
			token_vec_init(&tokens);

			token_type_t last_tok_type = TOK_INVALID;

			for(size_t i = 0; i < stdin_cache_length;) {
				c = stdin_cache[i];

				if(c == ' ' ||
					c == '\t' ||
					c == '\n') {
					++i;
				} else if(c == '[') {
					token_t *tok = token_vec_append(&tokens);
					tok->type = TOK_OPEN_ARRAY;
					last_tok_type = TOK_OPEN_ARRAY;
					++i;
				} else if(c == ']') {
					token_t *tok = token_vec_append(&tokens);
					tok->type = TOK_CLOSE_ARRAY;
					last_tok_type = TOK_CLOSE_ARRAY;
					++i;
				} else if(c == ',') {
					if(last_tok_type != TOK_VALUE &&
						last_tok_type != TOK_CLOSE_ARRAY) {
						print_diagnostic(stdin_cache_length, stdin_cache, i, 0);
						invalid_input = true;
						break;
					}

					last_tok_type = TOK_COMMA;
					++i;
				} else if(c >= '0' && c <= '9') {
					size_t start = i;
					for(;;) {
						c = stdin_cache[++i];
						if(c < '0' || c > '9') {
							break;
						}
					}
					size_t end = i;

					char old_c = stdin_cache[end];
					stdin_cache[end] = '\0';

					char *parse_end;
					long value = strtol(&stdin_cache[start], &parse_end, 10);

					stdin_cache[end] = old_c;

					if(errno == ERANGE || parse_end != &stdin_cache[end]) {
						print_diagnostic(stdin_cache_length, stdin_cache, start, end);
						invalid_input = true;
						break;
					}

					token_t *tok = token_vec_append(&tokens);
					tok->value = (ssize_t)value;
					tok->type = TOK_VALUE;
					last_tok_type = TOK_VALUE;
				} else {
					print_diagnostic(stdin_cache_length, stdin_cache, i, 0);
					invalid_input = true;
					break;
				}
			}

			if(!invalid_input) {
				value_list_node_t *sorted_values = NULL;

				list_list_node_t *list = parse_list(&tokens);
				if(list) {
					while(list) {
						list_list_node_t *next_list = list->next;
						value_list_node_t *value = list->values;
						while(value) {
							value_list_node_t *next_value = value->next;
							if(!sorted_values) {
								sorted_values = (value_list_node_t *)malloc(sizeof(value_list_node_t));
								sorted_values->value = value->value;
							} else {
								value_list_node_t *next = (value_list_node_t *)malloc(sizeof(value_list_node_t));
								next->next = sorted_values;
								next->value = value->value;
								sorted_values = next;
							}
							free(value);
							value = next_value;
						}
						free(list);
						list = next_list;
					}

					if(sorted_values) {
						bool swapped;
						value_list_node_t *curr;
						value_list_node_t *prev = NULL;

						do {
							swapped = false;
							curr = sorted_values;

							while(curr->next != prev) {
								if(curr->value > curr->next->value) {
									ssize_t tmp = curr->value;
									curr->value = curr->next->value;
									curr->next->value = tmp;
									swapped = true;
								}
								curr = curr->next;
							}

							prev = curr;
						} while(swapped);

						printf("%c", '[');
						curr = sorted_values;
						printf("%li", curr->value);
						curr = curr->next;
						while(curr) {
							printf(",%li", curr->value);
							curr = curr->next;
						}
						printf("%c\n", ']');
					}
				}
			}

			token_vec_free(&tokens);

			stdin_cache_length = 0;
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