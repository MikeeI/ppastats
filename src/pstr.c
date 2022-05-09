/*
 Copyright (C) 2011-2015 jeanfi@gmail.com

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301 USA
 */

#include <string.h>

#include <pstr.h>

char *strrep(char *str, const char *old, const char *new)
{
	char *p, *res;
	int pos;

	if (!str)
		return NULL;

	if (!*str || !old || !*old || !new || !strcmp(old, new))
		return str;

	p = strstr(str, old);

	if (!p)
		return str;

	res = malloc(strlen(str) + (new ? strlen(new) : 0) - strlen(old) + 1);

	pos = p - str;

	strncpy(res, str, pos);
	res[pos] = '\0';

	if (new)
		strcat(res + pos, new);

	strcat(res, str + pos + strlen(old));

	return res;
}

