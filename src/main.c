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

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cache.h>
#include <config.h>
#include <fcache.h>
#include <html.h>
#include <http.h>
#include <lp_ws.h>
#include <pio.h>
#include <plog.h>
#include <ppastats.h>

static const char *program_name;

static void display_published_binaries(const char *owner,
				       const char *ppa,
				       const char *package_status,
				       int ws_size)
{
	struct ppa_stats *ppastats;
	struct package_stats **packages;
	struct version_stats **versions;
	struct distro_stats **distros;
	struct arch_stats **archs;

	ppastats = create_ppa_stats(owner, ppa, package_status, ws_size);
	packages = ppastats->packages;
	while (packages && *packages) {
		struct package_stats *p = *packages;

		printf("%s (%d)\n", p->name, p->download_count);

		versions = p->versions;

		while (*versions) {
			printf("\t%s (%d)\n", (*versions)->version,
			       (*versions)->download_count);

			distros = (*versions)->distros;

			while (*distros) {
				printf("\t\t%s (%d)\n",
				       (*distros)->name,
				       (*distros)->download_count);

				archs = (*distros)->archs;

				while (*archs) {
					printf("\t\t\t%s (%d)\n",
					       (*archs)->name,
					       (*archs)->download_count);

					archs++;
				}

				distros++;
			}

			versions++;
		}

		packages++;
	}

	ppa_stats_free(ppastats);
}

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"output-dir", required_argument, 0, 'o'},
	{"debug", no_argument, 0, 'd'},
	{"status", required_argument, 0, 's'},
	{"skip-js-css", no_argument, 0, 'S'},
	{"get-bpph-size", required_argument, 0, 0},
	{"theme-dir", required_argument, 0, 't'},
	{0, 0, 0, 0}
};

static void print_version()
{
	printf("ppastats %s\n", VERSION);
	printf(_("Copyright (C) %s jeanfi@gmail.com\n"
"License GPLv2: GNU GPL version 2 or later\n"
"<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n"),
	       "2011-2015");
}

static void print_help()
{
	printf(_("Usage: %s [OPTION]... PPA_OWNER PPA_NAME\n"), program_name);

	puts(_(
"ppastats is a command application for generating PPA statistics.\n"));
	puts(_(
"Prints number of downloads for each published packages of a PPA or generates\n"
"an HTML page containing a graph representation."));

	puts("");
	puts(_("Options:"));
	puts(_("  -h, --help          display this help and exit"));
	puts(_("  -v, --version       display version information and exit"));

	puts("");

	puts(_("  -o, --output-dir=[PATH]  generates HTML pages into 'PATH'"));
	puts(_("  -t, --theme-dir=[PATH]   set theme dir to 'PATH'"));

	puts(_(
"  -s, --status=[STATUS]    retrieves only package of the given status\n"
"                           (possible values are: Pending, Published,\n"
"                           Superseded, Deleted or Obsolete)"));

	puts(_(
" -S, --skip-js-css         skip installation of js and css files"));
	puts(_(
" --get-bpph-size=[s]       size of the replies of webservice requests to get\n"
"                           the list of binary packages. Between 1 and 300."));
	puts("");

	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

int main(int argc, char **argv)
{
	char *owner, *ppa, *package_status, *output_dir, *theme_dir, *log, *tmp;
	int optc, output_html, cmdok, install_static_files, ws_size, opti;

	program_name = argv[0];

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	cmdok = 1;
	install_static_files = 1;
	output_dir = NULL;
	package_status = NULL;
	output_html = 0;
	ws_size = -1;
	theme_dir = NULL;

	while ((optc = getopt_long(argc, argv, "vho:t:ds:S", long_options,
				   &opti)) != -1) {
		switch (optc) {
		case 0:
			if (!strcmp(long_options[opti].name, "get-bpph-size"))
				ws_size = atoi(optarg);
			break;
		case 'o':
			output_html = 1;
			output_dir = strdup(optarg);
			break;
		case 'd':
			log_level = LOG_DEBUG;
			break;
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 's':
			if (optarg)
				package_status = strdup(optarg);
			break;
		case 't':
			if (optarg)
				theme_dir = strdup(optarg);
			break;
		case 'S':
			install_static_files = 0;
			break;
		default:
			cmdok = 0;
			break;
		}
	}

	if (!cmdok || optind + 2 != argc) {
		fprintf(stderr,
			_("Try `%s --help' for more information.\n"),
			program_name);
		exit(EXIT_FAILURE);
	}

	tmp = path_append(getenv("HOME"), ".ppastats");
	log = path_append(tmp, "ppastats.log");
	log_open(log);
	free(tmp);
	free(log);

	owner = argv[optind];
	ppa = argv[optind+1];

	if (output_html) {
		if (theme_dir)
			html_set_theme_dir(theme_dir);
		else
			html_set_theme_dir(DEFAULT_THEME_DIR);
		ppa_to_html(owner,
			    ppa,
			    package_status,
			    output_dir,
			    install_static_files,
			    ws_size);
	} else {
		display_published_binaries(owner, ppa, package_status, ws_size);
	}

	/* for valgrind.... */
	free(package_status);
	free(output_dir);
	http_cleanup();
	cache_cleanup();
	fcache_cleanup();
	html_cleanup();

	exit(EXIT_SUCCESS);
}
