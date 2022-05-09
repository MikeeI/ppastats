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

#ifndef _PPASTATS_LP_JSON_H_
#define _PPASTATS_LP_JSON_H_

#include <json.h>

#include <config.h>

/* Declares json_bool to have consistent code even with
   old json lib releases using boolean instead of json_bool.*/
#ifndef HAVE_JSON_BOOL
typedef int json_bool;
#endif

#include "lp.h"

struct bpph **json_object_to_bpph_list(json_object *o);
json_object *bpph_list_to_json(struct bpph **);

struct distro_arch_series *json_object_to_distro_arch_series(json_object *o);
struct distro_series *json_object_to_distro_series(json_object *o);

struct daily_download_total * *
json_object_to_daily_download_totals(json_object *o);

json_object *ddts_to_json(struct daily_download_total **);

struct json_object *date_to_json(struct tm *tm);

json_object *time_to_json(time_t t);
#endif
