
# Do not create dependencies while compiling
AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = License README.md

AM_CFLAGS = -Wall -Wextra -Iinclude/ $(XML_CPPFLAGS) $(LIBCURL_CPPFLAGS)

fernlibdir = $(libdir)/
fernincdir = $(includedir)/fern

fernlib_LIBRARIES = libfern.a
ferninc_HEADERS   = array.h request.h event.h station.h \
                    datareq.h meta.h \
                    miniseed_sac.h cprint.h fern.h

bin_PROGRAMS = fern

fern_SOURCES = fern.c
fern_LDADD = libfern.a $(XML_LIBS) $(LIBCURL)

libfern_a_SOURCES = array.c array.h \
										cJSON.c cJSON.h \
										chash.c chash.h \
										cprint.c cprint.h \
                    datareq.c datareq.h \
										event.c event.h \
										json.c json.h \
										meta.c meta.h \
										miniseed_sac.c miniseed_sac.h \
										quake_xml.c \
										request.c request.h \
										slurp.c slurp.h \
										station.c station.h \
										strip.c strip.h \
										xml.c xml.h \
                    fern.h

# TESTS

TEST_EXTENSIONS = .sh
TESTS = t/test_event.sh t/test_station.sh t/test_station_event.sh \
        t/test_avail.sh t/test_miniseed.sh t/test_sac.sh
EXTRA_DIST += t config

#check_PROGRAMS = t/iotest t/compat

#t_iotest_SOURCES = t/iotest.c
#t_compat_SOURCES = t/compat.c

#t_iotest_CFLAGS = -Iinclude
#t_compat_CFLAGS = -Iinclude
#t_iotest_LDADD = libsacio_bsd.a -lm
#t_compat_LDADD = libsacio_bsd.a -lm

CLEANFILES = t/*.test t/test_miniseed*mseed


doc:
	doxygen docs/Doxyfile

distclean-local:
	-rm -rf autom4te.cache

test: check

disttest:
	make clean
	make
	make test
	make dist
	tar -zxf $(PACKAGE)-$(VERSION).tar.gz
	cd $(PACKAGE)-$(VERSION) && ./configure && make && make test
