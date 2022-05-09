/*
 * Copyright (C) 2011-2014 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <libintl.h>
#define _(String) gettext(String)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cache.h>
#include <plog.h>
#include <ppastats.h>


/*
  Simple cache implementation but should be enough for storing LP data.
*/

struct entry {
	const char *key;
	const void *value;
	void (*fct_cleanup)(void *);
};

#define CAPACITY 1024

struct cache {
	int size;
	struct entry entries[CAPACITY];
};

static struct cache cache;

const void *cache_get(const char *key)
{
	int i;

	for (i = 0; i < cache.size; i++)
		if (!strcmp(cache.entries[i].key, key)) {
			log_debug(_("cache hit %s"), key);

			return cache.entries[i].value;
		}

	log_fct(_("memory cache miss %s"), key);

	return NULL;
}

void cache_put(const char *key, const void *value,
	       void (*fct_cleanup)(void *))
{
	if (cache.size == CAPACITY) {
		log_warn(_("exceed cache capacity"));
		return ;
	}

	cache.entries[cache.size].key = strdup(key);
	cache.entries[cache.size].value = value;
	cache.entries[cache.size].fct_cleanup = fct_cleanup;

	cache.size++;
}

void cache_cleanup()
{
	int i;

	for (i = 0; i < cache.size; i++) {
		free((char *)cache.entries[i].key);
		cache.entries[i].fct_cleanup((void *)cache.entries[i].value);
	}
}
