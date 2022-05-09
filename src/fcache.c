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

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcache.h>
#include <plog.h>
#include <pio.h>

static const char *cache_dir;

static const char *get_cache_dir()
{
	char *home;

	if (!cache_dir) {
		home = getenv("HOME");

		if (home) {
			cache_dir = malloc(strlen(home)
					   + 1 + strlen(".ppastats/cache")
					   + 1);
			sprintf((char *)cache_dir,
				"%s/%s", home, ".ppastats/cache");

			mkdirs(cache_dir, 0777);
		} else {
			log_warn(_("$HOME not defined"));
		}
	}

	return cache_dir;
}

static char *key_to_path(const char *key)
{
	char *path;
	const char *dir;

	if (!key || !*key || *key != '/')
		return NULL;

	dir = get_cache_dir();
	if (!dir)
		return NULL;

	path = malloc(strlen(dir) + 1 + strlen(key) + strlen("/data") + 1);
	sprintf(path, "%s%s.data", dir, key);

	return path;
}

char *fcache_get(const char *key)
{
	char *path, *content;

	path = key_to_path(key);
	if (!path) {
		log_err(_("file cache, invalid key: %s"), key);
		return NULL;
	}

	content = file_get_content(path);

	if (content)
		log_debug(_("file cache hit %s"), key);
	else
		log_debug(_("file cache miss %s %s"), key, path);

	free(path);

	return content;
}

void fcache_put(const char *key, const char *value)
{
	char *path, *dir, *tmp;
	FILE *f;

	path = key_to_path(key);
	if (!path)
		return ;

	tmp = strdup(path);
	dir = dirname(tmp);
	mkdirs(dir, 0777);
	free(tmp);

	f = fopen(path, "w");

	if (f) {
		fputs(value, f);

		fclose(f);
	} else {
		log_err(_("Failed to open %s"), path);
	}

	free(path);
}

void fcache_cleanup()
{
	free((char *)cache_dir);
}
