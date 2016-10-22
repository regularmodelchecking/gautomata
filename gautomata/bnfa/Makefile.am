AUTOMAKE_OPTIONS = 1.4

INCLUDES = @GBDD_CFLAGS@ -I$(srcdir)/..

libbnfa_la_SOURCES = bnfa.cc product.cc deterministic.cc minimize.cc construct.cc

noinst_LTLIBRARIES = libbnfa.la

libgautomataincludedir = $(includedir)/gautomata
libgautomatainclude_HEADERS = \
	bnfa.h



