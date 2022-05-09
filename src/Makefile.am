
AM_CPPFLAGS = -Wall -std=gnu99 -Werror\
        -DDEFAULT_THEME_DIR=\""$(pkgdatadir)/themes/default"\"\
	$(CURL_CFLAGS) $(JSON_CFLAGS)

LIBS += $(CURL_LIBS) $(JSON_LIBS)

DEFS = -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = ppastats
ppastats_SOURCES = \
	cache.h cache.c\
	fcache.h fcache.c\
	html.h html.c\
	http.h http.c\
	list.h list.c\
	lp.h lp.c\
	lp_json.h lp_json.c\
	lp_ws.h lp_ws.c\
	main.c\
	plog.h plog.c\
	ppastats.h ppastats.c\
	pio.h pio.c\
	pstr.h pstr.c\
	ptime.h ptime.c

dist_man_MANS = ppastats.1

ppastats.1: main.c $(top_srcdir)/configure.ac
	if ! test -z "$(HELP2MAN)"; then \
	 $(MAKE) $(AM_MAKEFLAGS) ppastats$(EXEEXT); \
	 help2man -N --name="PPA Statistics command line tool" --output=ppastats.1 ./ppastats$(EXEEXT); \
	fi;
