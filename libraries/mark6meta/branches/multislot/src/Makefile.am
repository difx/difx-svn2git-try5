
AM_CPPFLAGS = -I$(top_srcdir)

h_sources = \
	Mark6.h \
	Mark6DiskDevice.h \
	Mark6Meta.h \
	Mark6Module.h

cpp_sources = \
	Mark6.cpp \
	Mark6DiskDevice.cpp \
	Mark6Meta.cpp \
	Mark6Module.cpp \
	Mark6Controller.cpp

bin_PROGRAMS = test

test_SOURCES = test.cpp \
		$(cpp_sources)

#library_includedir = $(includedir)/mark6meta
#library_include_HEADERS = $(h_sources)

#lib_LTLIBRARIES = \
#	libmark6meta.la

#libmark6meta_la_SOURCES = $(h_sources) $(cpp_sources)
#libmark6meta_la_LDFLAGS = -version-info $(LIBRARY_VERSION)

