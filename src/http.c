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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>

#include "fcache.h"
#include "http.h"
#include <plog.h>

static const int DEFAULT_FETCH_RETRIES = 10;

static CURL *curl;

struct ucontent {
	char *data;
	size_t len;
};

static size_t cbk_curl(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct ucontent *mem = (struct ucontent *)userp;

	mem->data = realloc(mem->data, mem->len + realsize + 1);

	memcpy(&(mem->data[mem->len]), buffer, realsize);
	mem->len += realsize;
	mem->data[mem->len] = 0;

	return realsize;
}

static void init()
{
	if (!curl) {
		log_debug(_("initializing CURL"));
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
	}

	if (!curl)
		exit(EXIT_FAILURE);
}

static char *fetch_url(const char *url)
{
	struct ucontent *content = malloc(sizeof(struct ucontent));
	char *result;
	long code;
	int retries;
	unsigned int s;

	log_debug(_("fetch_url(): %s"), url);

	init();

	result = NULL;

	retries = DEFAULT_FETCH_RETRIES;

 retrieve:
	content->data = malloc(1);
	content->data[0] = '\0';
	content->len = 0;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cbk_curl);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, content);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "ppastats/0.0");
#ifdef CURLOPT_TRANSFER_ENCODING
	/* added since Curl 7.21.7 */
	curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1);
#endif

	if (curl_easy_perform(curl) == CURLE_OK) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

		switch (code) {
		case 200:
			result = content->data;
			break;
		case 500:
		case 502:
		case 503:
		case 504:
			log_err(_("Fetch failed with code %ld for URL %s"),
				code,
				url);

			if (retries) {
				s = 2 * (DEFAULT_FETCH_RETRIES - retries) + 2;
				log_debug(_("Wait %ds before retry"), s);
				sleep(s);

				free(content->data);
				retries--;
				goto retrieve;
			}

			break;
		default:
			log_err(_("Fetch failed with code %ld for URL %s"),
				code,
				url);
		}
	}

	if (!result)
		free(content->data);

	free(content);

	return result;
}

void http_cleanup()
{
	log_fct_enter();

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	log_fct_exit();
}

char *get_url_content(const char *url, unsigned int use_cache)
{
	char *content;

	content = NULL;

	if (use_cache)
		content = fcache_get(url + 7);

	if (!content)
		content = fetch_url(url);

	if (use_cache && content)
		fcache_put(url + 7, content);

	return content;
}
