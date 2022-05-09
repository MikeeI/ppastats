check-local: checkpatch.pl
	find $(top_srcdir)/src -name \*.c -exec $(srcdir)/checkpatch.pl --show-types -q --no-tree  -emacs -f {} \;
	find $(top_srcdir)/src -name \*.h -exec $(srcdir)/checkpatch.pl --ignore NEW_TYPEDEFS --show-types -q --no-tree  -emacs -f {} \;

EXTRA_DIST = \
	checkpatch.pl \
	test-cppcheck.sh \
	test_strrep.c \
	test_ptime.c

TESTS = test-strrep \
	test-ptime

check_PROGRAMS = test-strrep test-ptime

test_strrep_SOURCES = \
	test_strrep.c \
	$(top_builddir)/src/pstr.h \
	$(top_builddir)/src/pstr.c
test_strrep_CFLAGS = -I$(top_srcdir)/src

test_ptime_SOURCES = \
	test_ptime.c \
	$(top_builddir)/src/ptime.h \
	$(top_builddir)/src/ptime.c
test_ptime_CFLAGS = -I$(top_srcdir)/src

if CPPCHECK
TESTS += test-cppcheck.sh
endif