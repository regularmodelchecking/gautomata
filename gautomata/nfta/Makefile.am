AUTOMAKE_OPTIONS = 1.4

INCLUDES = @GBDD_CFLAGS@ -I$(srcdir)/..

libnfta_la_SOURCES = nfta.cc tree-regular-relation.cc

noinst_LTLIBRARIES = libnfta.la

libgautomataincludedir = $(includedir)/gautomata/nfta
libgautomatainclude_HEADERS = \
	nfta.h \
	tree-regular-relation.h



