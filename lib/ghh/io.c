#define GHH_LIB_INTERNAL
#include <stdlib.h>
#include <stdio.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>

// string must be freed
char *load_text_file(const char *filename) {
	FILE *file;
	char *text;
	size_t text_size, i;

	if ((file = fopen(filename, "r")) == NULL)
		ERROR("Could not open file: %s\n", filename);

	// get file size
	text_size = 0;
	while (fgetc(file) != EOF)
		++text_size;

	// copy file data
	rewind(file);
	text = malloc((text_size + 1) * sizeof(*text));

	for (i = 0; i < text_size; ++i)
		text[i] = fgetc(file);

	text[i] = 0;

	fclose(file);

	return text;
}
