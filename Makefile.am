AUTOMAKE_OPTIONS = 1.4

SUBDIRS = gautomata

bin_SCRIPTS = gautomata-config

EXTRA_DIST = gautomata.m4

m4datadir = $(datadir)/aclocal
m4data_DATA = gautomata.m4

if HAS_DOXYGEN

doxygen::
	doxygen Doxyfile

docdir = $(prefix)/share/doc/@PACKAGE@-@VERSION@

install-data-local: doxygen
	$(mkinstalldirs) $(DESTDIR)$(docdir)/html
	$(INSTALL_DATA) $(srcdir)/docs/html/* $(DESTDIR)$(docdir)/html
endif
