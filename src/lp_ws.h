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

#ifndef _PPASTATS_LP_WS_H_
#define _PPASTATS_LP_WS_H_

#include "lp.h"

/*
 * 'ws_size': size of the reply array of the getPublishedBinaries request.
 */
struct bpph **
get_bpph_list(const char *archive_url, const char *package_status, int ws_size);

int get_download_count(const char *archive_url);

const struct distro_arch_series *get_distro_arch_series(const char *url);

struct daily_download_total **get_daily_download_totals(const char *binary_url,
							time_t date_created);

const struct distro_series *get_distro_series(const char *distro_series_url);

void lp_ws_cleanup();

#endif
