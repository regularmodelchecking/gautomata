AUTOMAKE_OPTIONS = 1.4

SUBDIRS = automaton nfa bnfa refnfa mnfa nfta refnfta bnfta

INCLUDES = @GBDD_CFLAGS@ -I$(srcdir)/bnfa -I$(srcdir)/refnfa -I$(srcdir)/mnfa -I$(srcdir)/bnfta -I$(srcdir)/refnfta

EXTRA_DIST = doxygen_mainpage.h

lib_LTLIBRARIES = libgautomata.la
libgautomata_la_SOURCES =
libgautomata_la_LDFLAGS = -version-info 4:0:0
libgautomata_la_LIBADD = @GBDD_LIBS@ automaton/libautomaton.la nfa/libnfa.la bnfa/libbnfa.la refnfa/librefnfa.la  mnfa/libmnfa.la nfta/libnfta.la refnfta/librefnfta.la bnfta/libbnfta.la

libgautomataincludedir = $(includedir)/gautomata
libgautomatainclude_HEADERS = \
	gautomata.h

test_programs = test-nfa measure-nfa test-bnfta test-relations test-tree-relations

noinst_PROGRAMS = $(test_programs)

test_nfa_SOURCES = test-nfa.cc
test_nfa_LDADD = libgautomata.la

measure_nfa_SOURCES = measure-nfa.cc
measure_nfa_LDADD = libgautomata.la

test_bnfta_SOURCES = test-bnfta.cc
test_bnfta_LDADD = libgautomata.la

test_relations_SOURCES = test-relations.cc
test_relations_LDADD = libgautomata.la

test_tree_relations_SOURCES = test-tree-relations.cc
test_tree_relations_LDADD = libgautomata.la
