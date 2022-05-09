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

#ifndef _PPASTATS_PPASTATS_H_
#define _PPASTATS_PPASTATS_H_

#include "lp.h"

struct arch_stats {
	char *name;

	int download_count;
};

struct distro_stats {
	char *name;

	struct arch_stats **archs;
	int download_count;
	struct daily_download_total **ddts;
};

struct version_stats {
	char *version;
	time_t date_created;

	struct distro_stats **distros;
	int download_count;
	struct daily_download_total **daily_download_totals;
};

struct package_stats {
	char *name;

	struct version_stats **versions;
	int download_count;
	struct daily_download_total **daily_download_totals;
	struct distro_stats **distros;
};

struct ppa_stats {
	char *name;
	char *owner;

	struct package_stats **packages;
	int download_count;
	struct daily_download_total **daily_download_totals;
};

/*
 * 'ws_size': size of the reply array of the getPublishedBinaries request.
 */
struct ppa_stats *create_ppa_stats(const char *owner,
				   const char *ppa,
				   const char *package_status,
				   int ws_size);
void ppa_stats_free(struct ppa_stats *ppastats);

#endif
