#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ghh/array.h>
#include <ghh/string.h>
#include "l_system.h"

/*
 * stuff to potentially do
 * - stochastic rules
 * - parametric rules
 * - contextual (? forget name) rules
 */

typedef struct l_rule {
	string_t *symbol;
	const char *replacement;
} l_rule_t;

struct l_system {
	const char *axiom;
	array_t *rules;
};

l_system_t *l_system_create(const char *axiom) {
	l_system_t *lsys = malloc(sizeof(l_system_t));

	lsys->axiom = axiom;
	lsys->rules = array_create(0);

	return lsys;
}

void l_system_destroy(l_system_t *lsys) {
	for (size_t i = 0; i < array_size(lsys->rules); ++i)
		string_destroy(((l_rule_t *)array_get(lsys->rules, i))->symbol);

	array_destroy(lsys->rules, true);
	free(lsys);
}

void l_system_add_rule(l_system_t *lsys, const char *symbol, const char *replacement) {
	l_rule_t *rule = malloc(sizeof(l_rule_t));

	rule->symbol = string_create(symbol);
	rule->replacement = replacement;

	array_push(lsys->rules, rule);
}

const char *l_system_generate(l_system_t *lsys, int iterations) {
	int i, j, k;
	bool rule_applied;
	char c[2], *result;
	string_t *str, *next_str;
	l_rule_t *rule;

	str = string_create(lsys->axiom);

	for (i = 0; i < iterations; ++i) {
		next_str = string_create(NULL);

		j = 0;

		while (j < string_length(str)) {
			rule_applied = false;

			for (k = 0; k < array_size(lsys->rules); ++k) {
				rule = array_get(lsys->rules, k);

				if (!strncmp(string_raw(str) + j, string_raw(rule->symbol), string_length(rule->symbol))) {
					string_append(next_str, rule->replacement);
					j += string_length(rule->symbol);
					rule_applied = true;

					break;
				}
			}

			if (!rule_applied) {
				sprintf(c, "%c", string_raw(str)[j]);
				string_append(next_str, c);
				++j;
			}
		}

		string_destroy(str);
		str = next_str;
	}

	result = string_copy(str);

	string_destroy(str);

	return result;
}
