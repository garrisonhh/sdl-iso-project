#include <stdlib.h>
#include <string.h>
#include "mystring.h"

struct string_t {
	char *data;
	size_t length;
};

string_t *string_create(const char *initial_str) {
	string_t *str = malloc(sizeof(string_t));

	str->length = 0;
	str->data = malloc(sizeof(char));
	str->data[0] = '\0';

	if (initial_str != NULL)
		string_set(str, initial_str);

	return str;
}

void string_destroy(string_t *str) {
	free(str->data);
	free(str);
}

char *string_get(string_t *str) {
	return str->data;
}

size_t string_length(string_t *str) {
	return str->length;
}

char string_index(string_t *str, size_t i) {
	return str->data[i];
}

void string_set(string_t *str, const char *data) {
	free(str->data);

	str->length = strlen(data);
	str->data = malloc(sizeof(char) * (str->length + 1));
	
	strcpy(str->data, data);
}

void string_append(string_t *str, const char *other) {
	size_t old_len = str->length;

	str->length += strlen(other);
	str->data = realloc(str->data, str->length + 1);

	strcpy(str->data + old_len, other);
}

void string_replace(string_t *str, const char *substr, const char *replacement) {
	size_t i = 0;
	size_t substr_len = strlen(substr);
	string_t *new_str = string_create(NULL);

	while (i < str->length) {
		if (!strncmp(str->data + i, substr, substr_len)) {
			string_append(new_str, replacement);

			i += substr_len;
		} else {
			char c[2] = {str->data[i++], '\0'};

			string_append(new_str, c);
		}
	}

	free(str->data);

	str->data = new_str->data;

	free(new_str);
}
