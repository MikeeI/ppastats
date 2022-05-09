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

#ifndef _PPASTATS_LP_H_
#define _PPASTATS_LP_H_

#define URL_BASE_LP "https://api.launchpad.net/1.0"

#include <time.h>

struct daily_download_total {
	int count;

	struct tm date;
};

/* (b)inary (p)ackage (p)ublishing (h)istory */
struct bpph {
	char *binary_package_name;
	char *binary_package_version;
	char *distro_arch_series_link;
	char *self_link;
	char *status;
	int architecture_specific;

	time_t date_created;
};

struct distro_arch_series {
	char *display_name;
	char *title;
	char *architecture_tag;
	int is_nominated_arch_indep;
	char *distroseries_link;
};

struct distro_series {
	char *displayname;
	char *name;
	char *version;
	char *title;
};

struct distro_series *distro_series_new(const char *name,
					const char *version,
					const char *title,
					const char *displayname);

void distro_series_free(struct distro_series *distro_series);

void daily_download_total_list_free(struct daily_download_total **);

struct distro_arch_series *
distro_arch_series_new(const char *display_name,
		       const char *title,
		       const char *architecture_tag,
		       int is_nominated_arch_indep,
		       const char *distroseries_link);

void distro_arch_series_free(struct distro_arch_series *);

void bpph_free(struct bpph *b);

struct bpph *bpph_new(const char *binary_package_name,
		      const char *binary_package_version,
		      const char *distro_arch_series_link,
		      const char *self_link,
		      const char *status,
		      int architecture_specific,
		      time_t date_created);

void bpph_list_free(struct bpph **list);

struct bpph **bpph_list_append_list(struct bpph **list1, struct bpph **list2);

char *get_archive_url(const char *owner, const char  *ppa);

time_t ddts_get_last_date(struct daily_download_total **);

struct daily_download_total **
ddts_merge(struct daily_download_total **,
	   struct daily_download_total **);

int ddts_get_count(struct daily_download_total **);

struct daily_download_total *ddt_clone(struct daily_download_total *);
struct daily_download_total **ddts_clone(struct daily_download_total **);

#endif
