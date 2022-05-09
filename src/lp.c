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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <list.h>
#include <lp.h>
#include <ptime.h>

struct distro_series *distro_series_new(const char *name,
					const char *version,
					const char *title,
					const char *displayname)
{
	struct distro_series *d;

	d = malloc(sizeof(struct distro_series));

	d->name = strdup(name);
	d->version = strdup(version);
	d->title = strdup(title);
	d->displayname = strdup(displayname);

	return d;
}

void distro_series_free(struct distro_series *d)
{
	if (d) {
		free(d->name);
		free(d->version);
		free(d->title);
		free(d->displayname);

		free(d);
	}
}

void bpph_free(struct bpph *b)
{
	if (b) {
		free(b->binary_package_name);
		free(b->binary_package_version);
		free(b->distro_arch_series_link);
		free(b->self_link);
		free(b->status);
		free(b);
	}
}

struct bpph *bpph_new(const char *binary_package_name,
		      const char *binary_package_version,
		      const char *distro_arch_series_link,
		      const char *self_link,
		      const char *status,
		      int architecture_specific,
		      time_t date_created)
{
	struct bpph *h;

	h = malloc(sizeof(struct bpph));

	h->binary_package_name = strdup(binary_package_name);
	h->binary_package_version = strdup(binary_package_version);
	h->distro_arch_series_link = strdup(distro_arch_series_link);
	h->self_link = strdup(self_link);
	h->architecture_specific = architecture_specific;
	h->status = strdup(status);
	h->date_created = date_created;

	return h;
}

void bpph_list_free(struct bpph **list)
{
	struct bpph **cur;

	if (list) {
		cur = list;

		while (*cur) {
			bpph_free(*cur);
			cur++;
		}

		free(list);
	}
}

char *get_archive_url(const char *owner, const char *ppa)
{
	char *url = malloc(strlen(URL_BASE_LP)
			   +strlen("/~")
			   +strlen(owner)
			   +strlen("/+archive/")
			   +strlen(ppa)
			   +1);

	strcpy(url, URL_BASE_LP);
	strcat(url, "/~");
	strcat(url, owner);
	strcat(url, "/+archive/");
	strcat(url, ppa);

	return url;
}

struct distro_arch_series *distro_arch_series_new(const char *display_name,
						  const char *title,
						  const char *architecture_tag,
						  int is_nominated_arch_indep,
						  const char *distroseries_link)
{
	struct distro_arch_series *d;

	d = malloc(sizeof(struct distro_arch_series));

	d->display_name = strdup(display_name);
	d->title = strdup(title);
	d->architecture_tag = strdup(architecture_tag);
	d->is_nominated_arch_indep = is_nominated_arch_indep;
	d->distroseries_link = strdup(distroseries_link);

	return d;
}

void distro_arch_series_free(struct distro_arch_series *d)
{
	free(d->display_name);
	free(d->title);
	free(d->architecture_tag);
	free(d->distroseries_link);

	free(d);
}

void distro_arch_series_list_free(struct distro_arch_series **list)
{
	struct distro_arch_series **cur;

	if (list) {
		cur = list;
		while (*cur) {
			distro_arch_series_free(*cur);
			cur++;
		}
		free(list);
	}
}

void daily_download_total_list_free(struct daily_download_total **list)
{
	if (list) {
		struct daily_download_total **cur = list;

		while (*cur) {
			free(*cur);
			cur++;
		}

		free(list);
	}
}

struct bpph **bpph_list_add(struct bpph **list, struct bpph *new)
{
	struct bpph **cur, *bpph, **result;

	if (list)
		for (cur = list; *cur; cur++) {
			bpph = *cur;

			if (!strcmp(bpph->self_link, new->self_link))
				return list;
		}

	result = (struct bpph **)list_add((void **)list, new);

	free(list);

	return result;
}

struct bpph **bpph_list_append_list(struct bpph **list1, struct bpph **list2)
{
	struct bpph **cur;

	if (!list2)
		return list1;

	for (cur = list2; *cur; cur++)
		list1 = bpph_list_add(list1, *cur);

	return list1;
}

time_t ddts_get_last_date(struct daily_download_total **ddts)
{
	struct daily_download_total **cur;
	time_t t, last_t;

	if (!ddts)
		return 0;

	last_t = 0;
	for (cur = ddts; *cur; cur++) {
		t = mktime(&(*cur)->date);
		if (t > last_t)
			last_t = t;
	}

	return last_t;
}

int ddts_length(struct daily_download_total **ddts)
{
	int n;
	struct daily_download_total **cur;

	n = 0;

	if (ddts)
		for (cur = ddts; *cur; cur++)
			n++;

	return n;
}

struct daily_download_total *ddt_clone(struct daily_download_total *ddt)
{
	struct daily_download_total *new;

	new = malloc(sizeof(struct daily_download_total));

	new->date = ddt->date;
	new->count = ddt->count;

	return new;
}

struct daily_download_total **
ddts_clone(struct daily_download_total **ddts)
{
	int n, i;
	struct daily_download_total **new;

	n = ddts_length(ddts);

	new = malloc((n + 1) * sizeof(struct daily_download_total *));

	for (i = 0; i < n; i++)
		new[i] = ddt_clone(ddts[i]);

	new[n] = NULL;

	return new;
}

/*
  Return a newly allocated list with an additional ddt.
  All ddts are cloned.
 */
static struct daily_download_total **add_ddt
(struct daily_download_total **totals, struct daily_download_total *total)
{
	struct daily_download_total **cur, **ddts, **result;
	struct daily_download_total *item;

	if (totals) {
		cur = totals;
		while (*cur) {
			item = *cur;

			if (item->date.tm_year == total->date.tm_year &&
			    item->date.tm_mon == total->date.tm_mon &&
			    item->date.tm_mday == total->date.tm_mday) {
				item->count = total->count;
				return totals;
			}

			cur++;
		}
	}

	ddts = ddts_clone(totals);

	result = (struct daily_download_total **)
		list_add((void **)ddts, ddt_clone((void *)total));

	free(ddts);

	return result;
}

struct daily_download_total **
ddts_merge(struct daily_download_total **ddts1,
	   struct daily_download_total **ddts2)
{
	struct daily_download_total **ddts, **cur, **tmp;

	if (ddts1) {
		ddts = ddts_clone(ddts1);
	} else {
		ddts = malloc(sizeof(struct daily_download_total *));
		ddts[0] = NULL;
	}

	if (ddts2)
		for (cur = ddts2; *cur; cur++) {
			tmp = add_ddt(ddts, *cur);
			if (tmp != ddts) {
				daily_download_total_list_free(ddts);
				ddts = tmp;
			}
		}

	return ddts;
}

int ddts_get_count(struct daily_download_total **ddts)
{
	struct daily_download_total **cur;
	int i;

	i = 0;
	for (cur = ddts; *cur; cur++)
		i += (*cur)->count;

	return i;
}
