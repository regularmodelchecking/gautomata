AUTOMAKE_OPTIONS = 1.4

INCLUDES = @GBDD_CFLAGS@ -I$(srcdir)/..

libautomaton_la_SOURCES = automaton.cc  buchi-automaton.cc  simulation-graph.cc  word-automaton.cc

noinst_LTLIBRARIES = libautomaton.la

libgautomataincludedir = $(includedir)/gautomata/automaton
libgautomatainclude_HEADERS = \
	automaton.h \
	buchi-automaton.h \
	simulation-graph.h \
	word-automaton.h



