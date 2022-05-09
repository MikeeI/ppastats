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

#include <stdlib.h>
#include <string.h>

#include <list.h>

int list_length(void **list)
{
	int n;

	if (!list)
		return 0;

	n = 0;
	while (*list) {
		list++;
		n++;
	}

	return n;
}

void **list_add(void **list, void *new_item)
{
	int n;
	void **new_list;

	n = list_length(list);

	new_list = malloc(sizeof(void *)*(n+2));

	if (n)
		memcpy(new_list, list, sizeof(void *)*n);

	new_list[n] = new_item;
	new_list[n+1] = NULL;

	return new_list;
}

void **list_append_list(void **list1, void **list2)
{
	int n1, n2, n;
	void **list;

	n1 = list_length(list1);
	n2 = list_length(list2);

	n = n1 + n2 + 1;

	list = malloc(sizeof(void *)*(n+1));

	memcpy(list, list1, n1*sizeof(void *));
	memcpy(list+n1, list2, n2*sizeof(void *));

	list[n1+n2] = NULL;

	return list;
}

