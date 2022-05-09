/*
 * Copyright (C) 2010-2011 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pstr.h>

static int test_strrep(const char *str,
		       const char *old,
		       const char *new,
		       const char *ref)
{
	char *result, *s;

	if (str)
		s = strdup(str);
	else
		s = NULL;

	result = strrep(s, old, new);

	if (result == ref ||
	    (result != NULL && ref != NULL && !strcmp(result, ref)))
		return 0;

	fprintf(stderr, "strrep(%s, %s, %s) = %s\n", str, old, new, result);

	return 1;
}

static int tests_strrep()
{
	int failures;

	failures = 0;

	failures += test_strrep(NULL, NULL, NULL, NULL);
	failures += test_strrep("astring", NULL, NULL, "astring");
	failures += test_strrep("astring", "astring", NULL, "astring");
	failures += test_strrep("astring", NULL, "astring", "astring");
	failures += test_strrep("astring", "astring", "astring", "astring");
	failures += test_strrep(NULL, "astring", "astring", NULL);
	failures += test_strrep(NULL, NULL, "astring", NULL);
	failures += test_strrep(NULL, "astring", NULL, NULL);

	failures += test_strrep("astring", "astring", "", "");
	failures += test_strrep("DastringE", "astring", "", "DE");
	failures += test_strrep("Dastring", "astring", "", "D");
	failures += test_strrep("astringE", "astring", "", "E");

	failures += test_strrep("astring", "old", "new", "astring");
	failures += test_strrep("aoldstring", "old", "new", "anewstring");
	failures += test_strrep("astring", "astring", "thenewstring",
				"thenewstring");
	failures += test_strrep("Dastring", "astring", "thenewstring",
				"Dthenewstring");
	failures += test_strrep("astringE", "astring", "thenewstring",
				"thenewstringE");

	return failures;
}

int main(int argc, char **argv)
{
	int failures;

	failures = 0;

	failures += tests_strrep();

	if (failures)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
