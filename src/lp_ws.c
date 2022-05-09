/*
 * Copyright (C) 2011-2015 jeanfi@gmail.com
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <json.h>

#include <cache.h>
#include <fcache.h>
#include <http.h>
#include <list.h>
#include <lp_ws.h>
#include <lp_json.h>
#include <plog.h>
#include <ppastats.h>
#include <ptime.h>

/** Default ws.size value for the getPublishedBinaries request. */
static const int DEFAULT_WS_SIZE = 150;

static const char *QUERY_GET_DOWNLOAD_COUNT = "?ws.op=getDownloadCount";
static const char *
QUERY_GET_DAILY_DOWNLOAD_TOTALS = "?ws.op=getDailyDownloadTotals";

static json_object *get_json_object(const char *url)
{
	json_object *obj = NULL;
	char *body;

	body = get_url_content(url, 0);

	if (body) {
		obj = json_tokener_parse(body);

		free(body);

		return obj;
	}

	return NULL;
}

static char *get_bpph_list_cache_key(const char *archive_url)
{
	char *key;

	key = malloc(strlen(archive_url + 7) + strlen("/bpph") + 1);
	sprintf(key, "%s/bpph", archive_url + 7);

	return key;
}

static char *get_ddts_list_cache_key(const char *url)
{
	char *key;

	key = malloc(strlen(url + 7) + strlen("/ddts") + 1);
	sprintf(key, "%s/ddts", url + 7);

	return key;
}

static struct bpph **get_bpph_list_from_cache(const char *key)
{
	char *content;
	struct bpph **list;
	json_object *json;

	content = fcache_get(key);
	if (!content)
		return NULL;

	json = json_tokener_parse(content);
	if (!json)
		return NULL;

	list = json_object_to_bpph_list(json);

	json_object_put(json);
	free(content);

	return list;
}

static char *get_last_creation_date(struct bpph **list)
{
	time_t last, t;
	struct bpph **cur;

	last = 0;

	if (list)
		for (cur = list; *cur; cur++) {
			t = (*cur)->date_created;
			if (t > last)
				last = t;
		}

	if (last)
		return time_to_ISO8601_time(&last);
	else
		return NULL;
}

/*
 * 'archive_url': LP URL of the archive.
 * 'size': size of the reply array. Between 1-300, else default value is used.
 */
static char *create_query_get_bpph(const char *archive_url,
				   const char *status,
				   int size)
{
	static const char *default_opt = "?ws.op=getPublishedBinaries&ws.size=";
	static const char *status_opt = "&status=";
	char *url;
	size_t n;

	if (size < 1 || size > 300)
		size = DEFAULT_WS_SIZE;

	n = strlen(archive_url) + strlen(default_opt) + 3 + 1;

	if (status)
		n += strlen(status_opt) + strlen(status);

	url = malloc(n);
	sprintf(url, "%s%s%d", archive_url, default_opt, size);

	if (status) {
		strcat(url, status_opt);
		strcat(url, status);
	}

	return url;
}

struct bpph **get_bpph_list(const char *archive_url,
			    const char *pkg_status,
			    int ws_size)
{
	char *url, *key, *tmp;
	struct bpph **result;
	struct json_object *o, *bpph_json, *o_next;
	char *date;
	int ok;

	url = create_query_get_bpph(archive_url, pkg_status, ws_size);

	key = get_bpph_list_cache_key(archive_url);

	result = get_bpph_list_from_cache(key);

	if (result) {
		date = get_last_creation_date(result);

		if (date) {
			tmp = malloc(strlen(url)
				     + strlen("&created_since_date=")
				     + strlen(date)+1);
			strcpy(tmp, url);
			strcat(tmp, "&created_since_date=");
			strcat(tmp, date);

			free(url);
			url = tmp;

			free(date);
		}
	}

	ok = 1;
	while (url) {
		o = get_json_object(url);
		free(url);
		url = NULL;

		if (!o) {
			ok = 0;
			break;
		}

		result = bpph_list_append_list(result,
					       json_object_to_bpph_list(o));

		json_object_object_get_ex(o, "next_collection_link", &o_next);

		if (o_next)
			url = strdup(json_object_get_string(o_next));

		json_object_put(o);
	}

	if (ok) {
		bpph_json = bpph_list_to_json(result);
		fcache_put(key, json_object_to_json_string(bpph_json));
		json_object_put(bpph_json);
	}

	free(key);

	return result;
}

int get_download_count(const char *archive_url)
{
	int n = strlen(archive_url) + strlen(QUERY_GET_DOWNLOAD_COUNT) + 1;
	char *url = malloc(n);
	int result;
	json_object *obj;

	strcpy(url, archive_url);
	strcat(url, QUERY_GET_DOWNLOAD_COUNT);

	obj = get_json_object(url);
	free(url);

	if (!obj)
		return -1;

	result = json_object_get_int(obj);

	json_object_put(obj);

	return result;
}

const struct distro_arch_series *get_distro_arch_series(const char *url)
{
	json_object *obj;
	const struct distro_arch_series *distro;
	char *content;

	distro = cache_get(url);
	if (distro)
		return (struct distro_arch_series *)distro;

	content = get_url_content(url, 1);

	if (!content)
		return NULL;

	obj = json_tokener_parse(content);

	free(content);

	if (!obj)
		return NULL;

	distro = json_object_to_distro_arch_series(obj);

	json_object_put(obj);

	cache_put(url, distro, (void (*)(void *))&distro_arch_series_free);

	return distro;
}

const struct distro_series *get_distro_series(const char *url)
{
	json_object *obj;
	const struct distro_series *distro;
	char *content;

	distro = cache_get(url);
	if (distro)
		return (struct distro_series *)distro;

	content = get_url_content(url, 1);

	if (!content)
		return NULL;

	obj = json_tokener_parse(content);

	free(content);

	if (!obj)
		return NULL;

	distro = json_object_to_distro_series(obj);

	json_object_put(obj);

	cache_put(url, distro, (void (*)(void *))&distro_series_free);

	return distro;
}

/*
  Convert ddts older than 4 weeks to the same JSON representation than
  the LP one.  Newer ddts are not stored in the cache because the data
  may change during following days. It avoids to miss downloads which
  are not yet taken in consideration by LP.
 */
static json_object *ddts_to_json_for_cache(struct daily_download_total **ddts)
{
	json_object *j_ddts;
	struct daily_download_total *ddt;
	char *date;
	struct timeval *tv;
	time_t t;
	double d;

	j_ddts = json_object_new_object();

	tv = malloc(sizeof(struct timeval));
	gettimeofday(tv, NULL);

	while (ddts && *ddts) {
		ddt = *ddts;

		t = mktime(&(ddt->date));

		d = difftime(tv->tv_sec, t);

		if (d > 4 * 7 * 24 * 60 * 60) { /* older than 4 weeks */
			date = tm_to_ISO8601_date(&ddt->date);
			json_object_object_add(j_ddts,
					       date,
					       json_object_new_int(ddt->count));
			free(date);
		}

		ddts++;
	}

	free(tv);

	return j_ddts;
}

char *create_ddts_query(const char *binary_url, time_t st, time_t et)
{
	char *q;
	char *sdate, *edate;

	if (st) {
		sdate = time_to_ISO8601_date(&st);

		q = malloc(strlen(binary_url)
			   + strlen(QUERY_GET_DAILY_DOWNLOAD_TOTALS)
			   + strlen("&start_date=YYYY-MM-DD")
			   + strlen("&end_date=YYYY-MM-DD")
			   + 1);
		strcpy(q, binary_url);
		strcat(q, QUERY_GET_DAILY_DOWNLOAD_TOTALS);
		strcat(q, "&start_date=");
		strcat(q, sdate);

		if (et > 0) {
			edate = time_to_ISO8601_date(&et);
			strcat(q, "&end_date=");
			strcat(q, edate);
			free(edate);
		}

		free(sdate);
	} else {
		q = malloc(strlen(binary_url)
			   + strlen(QUERY_GET_DAILY_DOWNLOAD_TOTALS)
			   + 1);
		strcpy(q, binary_url);
		strcat(q, QUERY_GET_DAILY_DOWNLOAD_TOTALS);
	}

	return q;
}

static struct daily_download_total **retrieve_ddts(const char *binary_url,
						   time_t date_since)
{
	char *url;
	json_object *json;
	struct daily_download_total **ddts, **tmp;
	time_t crt;

	url = create_ddts_query(binary_url, date_since, 0);
	json = get_json_object(url);
	free(url);

	if (json) {
		ddts = json_object_to_daily_download_totals(json);
		json_object_put(json);
	} else {
		crt = time(NULL);
		ddts = NULL;

		while (date_since < crt) {
			url = create_ddts_query(binary_url,
						date_since,
						date_since);
			json = get_json_object(url);
			free(url);

			if (!json)
				break;

			tmp = json_object_to_daily_download_totals(json);
			json_object_put(json);
			ddts = ddts_merge(ddts, tmp);
			free(tmp);

			date_since = date_since + 24 * 60 * 60; /* +1 day */

			url = create_ddts_query(binary_url, date_since, 0);
			json = get_json_object(url);
			free(url);

			if (json) {
				tmp = json_object_to_daily_download_totals
					(json);
				json_object_put(json);
				ddts = ddts_merge(ddts, tmp);
				free(tmp);
				break;
			}
		}
	}

	return ddts;
}

struct daily_download_total **get_daily_download_totals(const char *binary_url,
							time_t date_created)
{
	char *key, *content;
	json_object *j_ddts, *json;
	struct daily_download_total **retrieved_ddts = NULL;
	struct daily_download_total **cached_ddts;
	struct daily_download_total **ddts;
	time_t last_t;

	key = get_ddts_list_cache_key(binary_url);

	content = fcache_get(key);
	if (content) {
		json = json_tokener_parse(content);
		free(content);
	} else {
		json = NULL;
	}

	if (json) {
		cached_ddts = json_object_to_daily_download_totals(json);
		json_object_put(json);
		last_t = ddts_get_last_date(cached_ddts);
	} else {
		last_t = 0;
		cached_ddts = NULL;
	}

	if (last_t > 0)
		retrieved_ddts = retrieve_ddts(binary_url, last_t);
	else
		retrieved_ddts = retrieve_ddts(binary_url, date_created);

	ddts = ddts_merge(cached_ddts, retrieved_ddts);

	if (ddts) {
		j_ddts = ddts_to_json_for_cache(ddts);
		fcache_put(key, json_object_get_string(j_ddts));
		json_object_put(j_ddts);
	}
	free(key);

	if (ddts != cached_ddts)
		daily_download_total_list_free(cached_ddts);
	daily_download_total_list_free(retrieved_ddts);

	return ddts;
}

