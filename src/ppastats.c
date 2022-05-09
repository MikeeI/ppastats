/*
  Copyright (C) 2011-2014 jeanfi@gmail.com

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA
 */

#include <libintl.h>
#define _(String) gettext(String)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list.h>
#include <lp_ws.h>
#include <plog.h>
#include <ppastats.h>

static void arch_stats_free(struct arch_stats *arch)
{
	free(arch->name);
	free(arch);
}

static struct distro_stats *distro_stats_new(const char *name)
{
	struct distro_stats *d;

	d = malloc(sizeof(struct distro_stats));
	d->name = strdup(name);
	d->archs = NULL;
	d->download_count = 0;
	d->ddts = NULL;

	return d;
}

static void distro_stats_free(struct distro_stats *distro)
{
	struct arch_stats **archs;

	archs = distro->archs;
	if (archs) {
		while (*archs) {
			arch_stats_free(*archs);
			archs++;
		}
		free(distro->archs);
	}

	daily_download_total_list_free(distro->ddts);

	free(distro->name);
	free(distro);
}

static void distro_stats_list_free(struct distro_stats **distros)
{
	struct distro_stats **cur;

	if (distros) {
		cur = distros;
		while (*cur) {
			distro_stats_free(*cur);
			cur++;
		}
		free(distros);
	}
}

static void version_stats_free(struct version_stats *version)
{
	distro_stats_list_free(version->distros);
	daily_download_total_list_free(version->daily_download_totals);

	free(version->version);
	free(version);
}

static void package_stats_free(struct package_stats *package)
{
	struct version_stats **versions;

	versions = package->versions;
	if (versions) {
		while (*versions) {
			version_stats_free(*versions);
			versions++;
		}
		free(package->versions);
	}
	distro_stats_list_free(package->distros);
	daily_download_total_list_free(package->daily_download_totals);
	free(package->name);
	free(package);
}

static struct package_stats *package_stats_new(const char *name)
{
	struct package_stats *p;

	p = malloc(sizeof(struct package_stats));
	p->name = strdup(name);
	p->versions = NULL;
	p->download_count = 0;
	p->daily_download_totals = NULL;
	p->distros = NULL;

	return p;
}

static struct package_stats *get_package_stats(struct ppa_stats *stats,
					       const char *name)

{
	struct package_stats *p, **p_cur, **tmp;

	p_cur = stats->packages;
	while (p_cur && *p_cur) {
		struct package_stats *p = *p_cur;

		if (!strcmp(p->name, name))
			return p;

		p_cur++;
	}

	p = package_stats_new(name);

	tmp = (struct package_stats **)list_add((void **)stats->packages, p);
	free(stats->packages);
	stats->packages = tmp;

	return p;
}

static struct version_stats *version_stats_new(const char *version)
{
	struct version_stats *v;

	v = malloc(sizeof(struct version_stats));
	v->version = strdup(version);
	v->distros = NULL;
	v->download_count = 0;
	v->daily_download_totals = NULL;
	v->date_created = 0;

	return v;
}

static struct version_stats *get_version_stats(struct package_stats *package,
					       const char *version)
{
	struct version_stats *v, **cur, **tmp;

	cur = package->versions;
	while (cur && *cur) {
		struct version_stats *v = *cur;

		if (!strcmp(v->version, version))
			return v;

		cur++;
	}

	v = version_stats_new(version);

	tmp = (struct version_stats **)list_add((void **)package->versions,
						v);
	free((void **)package->versions);
	package->versions = tmp;

	return v;
}

static struct distro_stats *get_distro_stats(struct version_stats *version,
					     const char *name)
{
	struct distro_stats **cur, *d, **tmp;

	cur = version->distros;

	while (cur && *cur) {
		d = *cur;

		if (!strcmp(d->name, name))
			return d;

		cur++;
	}

	d = distro_stats_new(name);

	tmp = (struct distro_stats **)list_add((void **)version->distros,
					       d);
	free(version->distros);
	version->distros = tmp;

	return d;
}

static struct arch_stats *get_arch_stats(struct distro_stats *distro,
					 const char *name)
{
	struct arch_stats **cur, *a, **tmp;

	cur = distro->archs;
	while (cur && *cur) {
		a = *cur;

		if (!strcmp(a->name, name))
			return a;

		cur++;
	}

	a = malloc(sizeof(struct arch_stats));
	a->name = strdup(name);
	a->download_count = 0;

	tmp = (struct arch_stats **)list_add((void **)distro->archs,
					     a);
	free((void **)distro->archs);
	distro->archs = tmp;

	return a;
}


static struct daily_download_total **add_total
(struct daily_download_total **totals, struct daily_download_total *total)
{
	struct daily_download_total **cur, **result, *item;

	if (totals) {
		cur = totals;
		while (*cur) {
			item = *cur;

			if (item->date.tm_year == total->date.tm_year &&
			    item->date.tm_mon == total->date.tm_mon &&
			    item->date.tm_mday == total->date.tm_mday) {
				item->count += total->count;
				return totals;
			}

			cur++;
		}
	}

	result = (struct daily_download_total **)list_add((void **)totals,
							  ddt_clone(total));
	return result;
}

static struct daily_download_total **add_totals
(struct daily_download_total **total1, struct daily_download_total **total2)
{
	struct daily_download_total **cur, **result, **tmp;

	result = total1;
	cur = total2;
	while (*cur) {
		tmp = add_total(result, *cur);
		if (result != total1 && result != tmp)
			free(result);
		result = tmp;
		cur++;
	}

	return result;
}

static void
pkg_add_distro(struct package_stats *pkg,
	       const char *distro_name,
	       int distro_count,
	       struct daily_download_total **ddts)
{
	struct distro_stats **pkg_distros, *pkg_distro, **tmp;
	struct daily_download_total **tmp_ddts;

	pkg_distros = pkg->distros;
	pkg_distro = NULL;

	if (pkg_distros)
		while (*pkg_distros)  {
			if (!strcmp((*pkg_distros)->name, distro_name)) {
				pkg_distro = *pkg_distros;
				break;
			}

			pkg_distros++;
		}

	if (!pkg_distro) {
		pkg_distro = distro_stats_new(distro_name);
		tmp = (struct distro_stats **)list_add((void **)pkg->distros,
						       (void *)pkg_distro);
		if (pkg->distros != tmp)
			free(pkg->distros);
		pkg->distros = tmp;
	}

	pkg_distro->download_count += distro_count;

	tmp_ddts = add_totals(pkg_distro->ddts, ddts);
	if (pkg_distro->ddts && pkg_distro->ddts != tmp_ddts)
		free(pkg_distro->ddts);
	pkg_distro->ddts = tmp_ddts;
}

static struct ppa_stats *ppa_stats_new(const char *owner, const char *ppa_name)
{
	struct ppa_stats *ppa;

	ppa = malloc(sizeof(struct ppa_stats));
	ppa->name = strdup(ppa_name);
	ppa->owner = strdup(owner);
	ppa->packages = NULL;
	ppa->daily_download_totals = NULL;
	ppa->download_count = 0;

	return ppa;
}

struct ppa_stats *
create_ppa_stats(const char *owner,
		 const char *ppa_name,
		 const char *package_status,
		 int ws_size)
{
	struct ppa_stats *ppa;
	struct bpph **history, **h_cur, *h;
	char *ppa_url, *pkg_name, *pkg_version;
	struct package_stats *pkg;
	struct version_stats *version;
	const struct distro_series *distro_series;
	const struct distro_arch_series *arch_series;
	struct distro_stats *distro;
	struct arch_stats *arch;
	int count;
	struct daily_download_total **totals, **tmp;

	ppa_url = get_archive_url(owner, ppa_name);
	history = get_bpph_list(ppa_url, package_status, ws_size);
	free(ppa_url);

	if (!history) {
		log_err(_("Failed to retrieve PPA information"));
		exit(EXIT_FAILURE);
	}

	ppa = ppa_stats_new(owner, ppa_name);

	for (h_cur = history; *h_cur; ++h_cur) {
		h = *h_cur;
		totals = get_daily_download_totals(h->self_link,
						   h->date_created);
		if (!totals) {
			log_err(_("Failed to retrieve download totals for %s"),
				h->self_link);
			continue;
		}
		count = ddts_get_count(totals);
		pkg_name = h->binary_package_name;
		pkg_version = h->binary_package_version;
		arch_series
			= get_distro_arch_series(h->distro_arch_series_link);
		distro_series
			= get_distro_series(arch_series->distroseries_link);

		ppa->download_count += count;
		tmp = add_totals(ppa->daily_download_totals, totals);
		if (ppa->daily_download_totals != tmp)
			free(ppa->daily_download_totals);
		ppa->daily_download_totals = tmp;

		pkg = get_package_stats(ppa, pkg_name);
		pkg->download_count += count;
		tmp = add_totals(pkg->daily_download_totals, totals);
		if (pkg->daily_download_totals != tmp)
			free(pkg->daily_download_totals);
		pkg->daily_download_totals = tmp;

		version = get_version_stats(pkg, pkg_version);
		version->date_created = h->date_created;

		version->download_count += count;
		tmp = add_totals(version->daily_download_totals, totals);
		if (version->daily_download_totals != tmp)
			free(version->daily_download_totals);
		version->daily_download_totals = tmp;

		distro = get_distro_stats(version, distro_series->name);
		distro->download_count += count;

		arch = get_arch_stats(distro, arch_series->architecture_tag);
		arch->download_count += count;

		pkg_add_distro(pkg, distro_series->name, count, totals);

		daily_download_total_list_free(totals);
	}

	bpph_list_free(history);

	return ppa;
}

void ppa_stats_free(struct ppa_stats *ppastats)
{
	struct package_stats **packages;

	packages = ppastats->packages;
	if (packages) {
		while (*packages) {
			package_stats_free(*packages);
			packages++;
		}
		free(ppastats->packages);
	}

	free(ppastats->owner);
	free(ppastats->name);

	daily_download_total_list_free(ppastats->daily_download_totals);

	free(ppastats);
}
