AUTOMAKE_OPTIONS = 1.4

INCLUDES = @GBDD_CFLAGS@ -I$(srcdir)/..

libnfa_la_SOURCES = nfa.cc deterministic.cc minimize.cc regular-relation.cc


noinst_LTLIBRARIES = libnfa.la

libgautomataincludedir = $(includedir)/gautomata/nfa
libgautomatainclude_HEADERS = \
	nfa.h \
	regular-relation.h



