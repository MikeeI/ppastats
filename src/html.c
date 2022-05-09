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
#include <sys/stat.h>
#include <sys/types.h>

#include <json.h>

#include "html.h"
#include "lp.h"
#include <lp_json.h>
#include "lp_ws.h"
#include "ppastats.h"
#include <pio.h>
#include <plog.h>
#include <pstr.h>

static char *css_dir;
static char *js_dir;
static char *tpl_dir;

static char *footer;
static char *ppa_body;
static char *pkg_body;
static char *pkg_version_body;
static char *header;

void html_set_theme_dir(const char *theme_dir)
{
	css_dir = path_append(theme_dir, "css");
	js_dir = path_append(theme_dir, "js");
	tpl_dir = path_append(theme_dir, "templates");
}

static char *path_new(const char *dir, const char *file, const char *suffixe)
{
	char *path;

	/* [dir]/[file][suffixe] */
	path = malloc(strlen(dir)+1+
		      strlen(file)+
		      (suffixe ? strlen(suffixe) : 0) +
		      1);

	strcpy(path, dir);
	strcat(path, "/");
	strcat(path, file);
	strcat(path, suffixe);

	return path;
}

static char *get_header(const char *title, const char *script)
{
	char *res, *tmp, *path;

	if (!header) {
		path = path_append(tpl_dir, "header.tpl");
		header = file_get_content(path);
	} else {
		path = NULL;
	}

	if (header) {
		tmp = strdup(header);
		res = strrep(tmp, "@SCRIPT@", script);

		if (res != tmp)
			free(tmp);

		tmp = res;
		res = strrep(tmp, "@TITLE@", title);

		if (res != tmp)
			free(tmp);
	} else {
		log_err("Failed to read header template: %s", path);
		res = NULL;
	}

	free(path);

	return res;
}

static const char *get_footer()
{
	char *path;

	if (!footer) {
		path = path_append(tpl_dir, "footer.tpl");
		footer = file_get_content(path);

		if (!footer)
			log_err("Failed to read footer template: %s", path);

		free(path);
	}

	return footer;
}

static const char *get_pkg_version_body()
{
	char *path;

	if (!pkg_version_body) {
		path = path_append(tpl_dir, "pkg_version.tpl");
		pkg_version_body = file_get_content(path);

		if (!pkg_version_body)
			log_err("Failed to read package version template: %s",
				path);

		free(path);
	}

	return pkg_version_body;
}
static const char *get_ppa_body()
{
	char *path;

	if (!ppa_body) {
		path = path_append(tpl_dir, "ppa.tpl");
		ppa_body = file_get_content(path);

		if (!ppa_body)
			log_err("Failed to read PPA template: %s", path);

		free(path);
	}

	return ppa_body;
}

static const char *get_pkg_body()
{
	char *path;

	if (!pkg_body) {
		path = path_append(tpl_dir, "pkg.tpl");
		pkg_body = file_get_content(path);

		if (!pkg_body)
			log_err("Failed to read package template: %s", path);

		free(path);
	}

	return pkg_body;
}

static void json_add_ddts(json_object *json,
			  struct daily_download_total **ddts)
{
	json_object_object_add(json, "ddts", ddts_to_json(ddts));
}

static json_object *distro_to_json(struct distro_stats *d)
{
	json_object *json;

	json = json_object_new_object();

	json_object_object_add(json,
			       "name",
			       json_object_new_string(d->name));

	json_object_object_add(json,
			       "count",
			       json_object_new_int(d->download_count));

	json_add_ddts(json, d->ddts);

	return json;
}

static int version_cmp(const void *o1, const void *o2)
{
	struct version_stats **v1, **v2;

	v1 = (struct version_stats **)o1;
	v2 = (struct version_stats **)o2;

	return (*v1)->date_created <= (*v2)->date_created;
}

static struct version_stats **sort_versions(struct version_stats **vers)
{
	size_t n;
	struct version_stats **tmp, **result;

	tmp = vers;
	n = 0;
	while (*tmp) {
		n++;
		tmp++;
	}

	result = malloc((n + 1) * sizeof(struct version_stats *));
	memcpy(result, vers, n * sizeof(struct version_stats *));
	result[n] = NULL;

	qsort(result, n, sizeof(struct version_stats *), version_cmp);

	return result;
}

static json_object *
pkg_to_json(struct ppa_stats *ppa, struct package_stats *pkg)
{
	json_object *json, *json_versions, *json_distros, *json_distro;
	struct version_stats **versions, **tmp;
	struct distro_stats **distros, *d;

	json = json_object_new_object();

	json_object_object_add(json,
			       "ppa_name", json_object_new_string(ppa->name));
	json_object_object_add(json,
			       "ppa_owner",
			       json_object_new_string(ppa->owner));

	json_object_object_add(json,
			       "name", json_object_new_string(pkg->name));

	json_versions = json_object_new_array();
	json_object_object_add(json, "versions", json_versions);
	versions = sort_versions(pkg->versions);
	tmp = versions;
	while (*tmp) {
		json_object_array_add
			(json_versions,
			 json_object_new_string((*tmp)->version));

		tmp++;
	}
	free(versions);

	distros = pkg->distros;
	if (distros) {
		json_distros = json_object_new_array();
		json_object_object_add(json, "distros", json_distros);

		while (*distros) {
			d = *distros;

			if (d->download_count) {
				json_distro = distro_to_json(d);

				json_object_array_add(json_distros,
						      json_distro);
			}

			distros++;
		}
	}

	json_add_ddts(json, pkg->daily_download_totals);

	return json;
}

static char *version_to_json(struct ppa_stats *ppa,
			     struct package_stats *pkg,
			     struct version_stats *ver)
{
	char *ret;
	struct distro_stats **distros, *distro;
	json_object *json, *json_distros, *json_distro, *json_archs, *json_arch;
	struct arch_stats **archs;

	json = json_object_new_object();

	json_object_object_add(json,
			       "ppa_name", json_object_new_string(ppa->name));
	json_object_object_add(json,
			       "ppa_owner",
			       json_object_new_string(ppa->owner));

	json_object_object_add(json,
			       "pkg_name", json_object_new_string(pkg->name));

	json_object_object_add(json,
			       "name", json_object_new_string(ver->version));

	json_object_object_add(json,
			       "date_created", time_to_json(ver->date_created));

	json_add_ddts(json, ver->daily_download_totals);

	distros = ver->distros;
	json_distros = json_object_new_array();
	json_object_object_add(json, "distros", json_distros);
	while (*distros) {
		distro = *distros;
		json_distro = json_object_new_object();

		json_object_array_add(json_distros, json_distro);

		json_object_object_add(json_distro,
				       "name",
				       json_object_new_string(distro->name));

		archs = distro->archs;
		json_archs = json_object_new_array();
		json_object_object_add(json_distro, "archs", json_archs);
		while (*archs) {
			json_arch = json_object_new_object();

			json_object_object_add
				(json_arch,
				 "name",
				 json_object_new_string((*archs)->name));

			json_object_object_add
				(json_arch,
				 "count",
				 json_object_new_int((*archs)->download_count));

			json_object_array_add(json_archs, json_arch);
			archs++;
		}

		distros++;
	}

	ret = strdup(json_object_to_json_string(json));

	json_object_put(json);

	return ret;
}

static json_object *ppa_to_json(struct ppa_stats *ppa)
{
	json_object *json, *json_pkgs, *json_pkg;
	struct package_stats **pkgs;

	json = json_object_new_object();

	json_object_object_add(json,
			       "ppa_name", json_object_new_string(ppa->name));
	json_object_object_add(json,
			       "ppa_owner",
			       json_object_new_string(ppa->owner));

	json_add_ddts(json, ppa->daily_download_totals);

	pkgs = ppa->packages;
	json_pkgs = json_object_new_array();
	json_object_object_add(json, "packages", json_pkgs);
	while (*pkgs) {
		json_pkg = json_object_new_object();
		json_object_array_add(json_pkgs, json_pkg);

		json_object_object_add(json_pkg, "name",
				       json_object_new_string((*pkgs)->name));

		json_object_object_add
			(json_pkg, "count",
			 json_object_new_int((*pkgs)->download_count));

		pkgs++;
	}

	return json;
}

static void
create_html(const char *path,
	    const char *title,
	    const char *body_template,
	    const char *script)
{
	FILE *f;
	const char *footer;
	char *header;

	f = NULL;

	header = get_header(title, script);
	if (!header) {
		log_err(_("Failed to get the header template"));
		goto on_error;
	}

	f = fopen(path, "w");

	if (!f) {
		log_err(_("Failed to open: %s"), path);
		goto on_error;
	}

	fputs(header, f);
	fputs(body_template, f);

	footer = get_footer();
	if (footer)
		fputs(footer, f);

 on_error:
	if (header)
		free(header);

	if (f)
		fclose(f);
}

static char *ppa_display_name(const struct ppa_stats *ppa)
{
	char *ret;

	ret = malloc(4+strlen(ppa->name)+1+strlen(ppa->owner)+1);

	sprintf(ret, "ppa:%s/%s", ppa->owner, ppa->name);

	return ret;
}

static void
index_to_html(struct ppa_stats *ppa, const char *dir)
{
	char *path, *json_path, *dname;
	json_object *json;
	const char *body;

	body = get_ppa_body();
	if (!body) {
		log_err("Failed to create PPA page");
		return ;
	}

	json = ppa_to_json(ppa);
	json_path = path_new(dir, "index", ".json");

	log_debug(_("generating %s"), json_path);
	json_object_to_file(json_path, json);
	json_object_put(json);
	free(json_path);

	path = path_new(dir, "index", ".html");
	dname = ppa_display_name(ppa);
	create_html(path, dname, body, "ppastats_ppa();");
	free(path);
	free(dname);
}

static void
version_to_html(struct ppa_stats *ppa,
		struct package_stats *pkg,
		struct version_stats *version,
		const char *dir)
{
	char *f_name, *path;
	const char *body;
	const char *script_tpl;
	char *script, *json;

	body = get_pkg_version_body();
	if (!body) {
		log_err("Failed to create package version page");
		return ;
	}

	json = version_to_json(ppa, pkg, version);
	if (!json) {
		log_err("Failed to create package version page");
		return ;
	}

	f_name = malloc(strlen(pkg->name)+1+strlen(version->version)+1);
	sprintf(f_name, "%s_%s", pkg->name, version->version);

	path = path_new(dir, f_name, ".html");

	script_tpl = "var data = %s;\n ppastats_ver();";
	script = malloc(strlen(script_tpl) - 2 + strlen(json) + 1);
	sprintf(script, script_tpl, json);

	create_html(path, f_name, body, script);

	free(script);
	free(json);
	free(path);
	free(f_name);
}

static void
pkg_to_html(struct ppa_stats *ppa, struct package_stats *pkg, const char *dir)
{
	char *path, *json_path, *script;
	json_object *json;
	const char *body;

	body = get_pkg_body();
	if (!body) {
		log_err("Failed to create package page: %s", pkg->name);
		return ;
	}

	json_path = path_new(dir, pkg->name, ".json");
	json = pkg_to_json(ppa, pkg);
	log_debug(_("Generating %s"), json_path);

	json_object_to_file(json_path, json);
	json_object_put(json);
	free(json_path);

	path = path_new(dir, pkg->name, ".html");
	script = malloc(strlen("ppastats_pkg(\"\");")+
			strlen(pkg->name)+
			strlen(".json")+
			1);
	sprintf(script, "ppastats_pkg(\"%s%s\");", pkg->name, ".json");

	log_debug(_("Generating %s"), path);

	create_html(path, pkg->name, body, script);
	free(path);
	free(script);
}

static void
pkgs_to_html(struct ppa_stats *ppa,
	     struct package_stats **pkgs,
	     const char *dir)
{
	struct version_stats **versions;

	while (*pkgs) {
		pkg_to_html(ppa, *pkgs, dir);

		versions = (*pkgs)->versions;
		while (*versions) {
			version_to_html(ppa, *pkgs, *versions, dir);

			versions++;
		}

		pkgs++;
	}
}

void
ppa_to_html(const char *owner,
	    const char *ppa,
	    const char *package_status,
	    const char *output_dir,
	    const int install_static_files,
	    int ws_size)
{
	struct ppa_stats *ppastats;
	char *path;
	char *css_odir, *js_odir;

	mkdirs(output_dir, 0777);

	if (install_static_files) {
		css_odir = path_append(output_dir, "css");
		js_odir = path_append(output_dir, "js");

		dir_rcopy(css_dir, css_odir);
		dir_rcopy(js_dir, js_odir);

		free(css_odir);
		free(js_odir);
	}

	ppastats = create_ppa_stats(owner, ppa, package_status, ws_size);

	path = path_new(output_dir, "ppa", ".html");

	pkgs_to_html(ppastats, ppastats->packages, output_dir);

	index_to_html(ppastats, output_dir);

	ppa_stats_free(ppastats);

	free(path);
}

void html_cleanup()
{
	free(header);
	free(footer);
	free(ppa_body);
	free(pkg_body);
	free(pkg_version_body);
	free(js_dir);
	free(css_dir);
	free(tpl_dir);
}
