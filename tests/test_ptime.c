/*
  Copyright (C) 2010-2011 jeanfi@gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ptime.h>

int test_time_to_ISO8601_time(time_t t, const char *ref)
{
	char *result;
	int failure;

	result = time_to_ISO8601_time(&t);

	failure = !!strcmp(result, ref);

	if (failure)
		fprintf(stderr,
			"test_time_to_ISO8601_time(%ld)=%s instead of %s.\n",
			t,
			result,
			ref);

	free(result);

	return failure;
}

int test_time_to_ISO8601_date(time_t t, const char *ref)
{
	char *result;
	int failure;

	result = time_to_ISO8601_date(&t);

	failure = !!strcmp(result, ref);

	if (failure)
		fprintf(stderr,
			"test_date_to_ISO8601_time(%ld)=%s instead of %s.\n",
			t,
			result,
			ref);

	free(result);

	return failure;
}

static int tests_time_to_ISO8601_time()
{
	int failures;

	failures = 0;

	failures += test_time_to_ISO8601_time(0, "1970-01-01T00:00:00");
	failures += test_time_to_ISO8601_time(83, "1970-01-01T00:01:23");
	failures += test_time_to_ISO8601_time(1392542321,
					      "2014-02-16T09:18:41");

	return failures;
}

static int tests_time_to_ISO8601_date()
{
	int failures;

	failures = 0;

	failures += test_time_to_ISO8601_date(0, "1970-01-01");
	failures += test_time_to_ISO8601_date(83, "1970-01-01");
	failures += test_time_to_ISO8601_date(1392542321, "2014-02-16");

	return failures;
}

int main(int argc, char **argv)
{
	int failures;

	failures = 0;

	failures += tests_time_to_ISO8601_time();
	failures += tests_time_to_ISO8601_date();

	if (failures)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
