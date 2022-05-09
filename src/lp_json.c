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
#define _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <lp_json.h>
#include <lp_ws.h>
#include <ptime.h>

static time_t json_to_time(json_object *json)
{
	const char *str;
	struct tm tm;
	char *ret;

	str = json_object_get_string(json);
	if (!str)
		return -1;

	memset(&tm, 0, sizeof(struct tm));
	tm.tm_isdst = -1;
	ret = strptime(str, "%FT%T", &tm);

	if (ret)
		return mktime(&tm);
	else
		return -1;
}

json_object *time_to_json(time_t t)
{
	char *str;
	json_object *j;

	str = time_to_ISO8601_time(&t);

	if (str) {
		j = json_object_new_string(str);
		free(str);
		return j;
	} else {
		return NULL;
	}
}

static struct bpph *json_to_bpph(json_object *o)
{
	const char *binary_package_name;
	const char *binary_package_version;
	const char *distro_arch_series_link;
	const char *self_link;
	int arch_specific;
	struct bpph *bpph;
	const char *status;
	time_t date_created;
	json_object *j;

	json_object_object_get_ex(o, "binary_package_name", &j);
	binary_package_name = json_object_get_string(j);

	json_object_object_get_ex(o, "binary_package_version", &j);
	binary_package_version = json_object_get_string(j);

	json_object_object_get_ex(o, "distro_arch_series_link", &j);
	distro_arch_series_link = json_object_get_string(j);

	json_object_object_get_ex(o, "self_link", &j);
	self_link = json_object_get_string(j);

	json_object_object_get_ex(o, "architecture_specific", &j);
	arch_specific = json_object_get_boolean(j);

	json_object_object_get_ex(o, "date_created", &j);
	date_created = json_to_time(j);

	json_object_object_get_ex(o, "status", &j);
	status = json_object_get_string(j);

	bpph =  bpph_new(binary_package_name,
			 binary_package_version,
			 distro_arch_series_link,
			 self_link,
			 status,
			 arch_specific,
			 date_created);

	return bpph;
}

static json_object *bpph_to_json(struct bpph *bpph)
{
	json_object *json, *time;

	json = json_object_new_object();

	json_object_object_add
		(json,
		 "binary_package_name",
		 json_object_new_string(bpph->binary_package_name));

	json_object_object_add
		(json,
		 "binary_package_version",
		 json_object_new_string(bpph->binary_package_version));

	json_object_object_add
		(json,
		 "distro_arch_series_link",
		 json_object_new_string(bpph->distro_arch_series_link));

	json_object_object_add
		(json, "self_link", json_object_new_string(bpph->self_link));

	json_object_object_add
		(json,
		 "architecture_specific",
		 json_object_new_boolean(bpph->architecture_specific));

	json_object_object_add
		(json, "status", json_object_new_string(bpph->status));

	time = time_to_json(bpph->date_created);
	json_object_object_add
		(json, "date_created", time);

	return json;
}

struct distro_arch_series *json_object_to_distro_arch_series(json_object *o)
{
	const char *display_name;
	const char *title;
	const char *architecture_tag;
	json_bool is_nominated_arch_indep;
	const char *distroseries_link;
	json_object *j;

	json_object_object_get_ex(o, "display_name", &j);
	display_name = json_object_get_string(j);

	json_object_object_get_ex(o, "title", &j);
	title = json_object_get_string(j);

	json_object_object_get_ex(o, "architecture_tag", &j);
	architecture_tag = json_object_get_string(j);

	json_object_object_get_ex(o, "distroseries_link", &j);
	distroseries_link = json_object_get_string(j);

	json_object_object_get_ex(o, "is_nominated_arch_indep", &j);
	is_nominated_arch_indep = json_object_get_boolean
		(j);

	return distro_arch_series_new(display_name,
				      title,
				      architecture_tag,
				      is_nominated_arch_indep,
				      distroseries_link);
}

struct distro_series *json_object_to_distro_series(json_object *o)
{
	const char *displayname;
	const char *title;
	const char *name;
	const char *version;
	json_object *j;

	json_object_object_get_ex(o, "displayname", &j);
	displayname = json_object_get_string(j);

	json_object_object_get_ex(o, "title", &j);
	title = json_object_get_string(j);

	json_object_object_get_ex(o, "version", &j);
	version = json_object_get_string(j);

	json_object_object_get_ex(o, "name", &j);
	name = json_object_get_string(j);

	return distro_series_new(name,
				 version,
				 title,
				 displayname);
}

struct bpph **json_object_to_bpph_list(json_object *o)
{
	json_object *o_entries;
	int i, n, i2;
	struct bpph **entries, *h;
	const struct distro_arch_series *distro;

	json_object_object_get_ex(o, "entries", &o_entries);

	if (!o_entries)
		return NULL;

	n = json_object_array_length(o_entries);

	entries = malloc
		(sizeof(struct bpph *)*(n+1));

	for (i = 0, i2 = 0; i < n; i++) {
		h = json_to_bpph(json_object_array_get_idx(o_entries,
							   i));

		if (!h->architecture_specific) {
			distro = get_distro_arch_series
				(h->distro_arch_series_link);

			if (!distro || !distro->is_nominated_arch_indep) {
				bpph_free(h);
				continue ;
			}
		}

		entries[i2] = h;
		i2++;
	}
	entries[i2] = NULL;

	return entries;
}

json_object *bpph_list_to_json(struct bpph **list)
{
	json_object *result, *entries;
	struct bpph **cur;

	result = json_object_new_object();

	entries = json_object_new_array();
	json_object_object_add(result, "entries", entries);

	if (list)
		for (cur = list; *cur; cur++)
			json_object_array_add(entries, bpph_to_json(*cur));

	return result;
}

struct daily_download_total *
json_object_to_daily_download_total(const char *d, json_object *o_c)
{
	struct daily_download_total *result;

	result = malloc(sizeof(struct daily_download_total));
	result->count = json_object_get_int(o_c);

	memset(&result->date, 0, sizeof(struct tm));
	strptime(d, "%FT%T%z", &result->date);

	return result;
}

static int json_object_get_fields_count(json_object *o)
{
	int n = 0;
	struct lh_entry *entry;

	entry = json_object_get_object(o)->head;
	while (entry) {
		entry = entry->next;
		n++;
	}

	return n;
}

struct daily_download_total * *
json_object_to_daily_download_totals(json_object *o)
{
	int n, i;
	struct daily_download_total **result;

	n = json_object_get_fields_count(o);

	result = malloc
		(sizeof(struct daily_download_total *)*(n+1));

	i = 0;
	json_object_object_foreach(o, key, val) {
		result[i] = json_object_to_daily_download_total(key, val);
		i++;
	}

	result[n] = NULL;

	return result;
}

struct json_object *date_to_json(struct tm *tm)
{
	json_object *json;

	json = json_object_new_array();
	json_object_array_add(json, json_object_new_int(tm->tm_year+1900));
	json_object_array_add(json, json_object_new_int(tm->tm_mon+1));
	json_object_array_add(json, json_object_new_int(tm->tm_mday));

	return json;
}

json_object *ddts_to_json(struct daily_download_total **ddts)
{
	json_object *json_ddt, *json_ddts;
	struct daily_download_total *ddt;

	json_ddts = json_object_new_array();

	while (ddts && *ddts) {
		ddt = *ddts;

		json_ddt = json_object_new_object();
		json_object_object_add(json_ddt,
				       "value",
				       json_object_new_int(ddt->count));
		json_object_object_add(json_ddt,
				       "time",
				       date_to_json(&ddt->date));

		json_object_array_add(json_ddts, json_ddt);

		ddts++;
	}

	return json_ddts;
}
