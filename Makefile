CC      ?= gcc
CFLAGS  = -Wall -g `pkg-config --cflags gstreamer-1.0 glib-2.0 ncurses`
LDLIBS  = `pkg-config --libs gstreamer-1.0 glib-2.0 ncurses`

TARGET   = tmuzika
BUILDDIR = build
SRC_DIR  = src
SOURCES  = $(wildcard $(SRC_DIR)/*.c)

PREFIX    ?= /usr
DESTDIR   ?=

BINDIR     = $(DESTDIR)$(PREFIX)/bin
LOCALEDIR  = $(DESTDIR)$(PREFIX)/share/locale
DOCDIR     = $(DESTDIR)$(PREFIX)/share/doc/tmuzika
LICENSEDIR = $(DESTDIR)$(PREFIX)/share/licenses/tmuzika
APPDIR     = $(DESTDIR)$(PREFIX)/share/applications
MANDIR     = $(DESTDIR)$(PREFIX)/share/man

PODIR     = po
PACKAGE   = tmuzika

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(SOURCES)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDLIBS)

mo:
	@echo "Compiling translations"
	@for po in $(PODIR)/*.po; do \
		lang=$$(basename $$po .po); \
		mkdir -p $(PODIR)/$$lang; \
		msgfmt $$po -o $(PODIR)/$$lang/$(PACKAGE).mo; \
	done

install: all mo
	@echo "Installing:"
	@echo " --- Binary file"
	@install -Dm755 $(BUILDDIR)/$(TARGET) $(BINDIR)/$(TARGET)

	@echo " --- Translations (.mo files)"
	@for langdir in $(PODIR)/*; do \
		[ -d "$$langdir" ] || continue; \
		lang=$$(basename $$langdir); \
		install -Dm644 $$langdir/$(PACKAGE).mo \
			$(LOCALEDIR)/$$lang/LC_MESSAGES/$(PACKAGE).mo; \
	done

	@echo " --- Desktop entry"
	@install -Dm644 desktop/tmuzika.desktop $(APPDIR)/tmuzika.desktop

	@echo " --- Documentation"
	@install -Dm644 docs/README.md $(DOCDIR)/README.md
	@install -Dm644 docs/README.sr.md $(DOCDIR)/README.sr.md
	@install -Dm644 docs/README.sr-cy.md $(DOCDIR)/README.sr-cy.md
	@install -Dm644 docs/INSTALL.md $(DOCDIR)/INSTALL.md

	@echo " --- License"
	@install -Dm644 docs/LICENSE $(LICENSEDIR)/LICENSE

	@echo " --- Manual pages"
	@install -Dm644 man/tmuzika.1 $(MANDIR)/man1/tmuzika.1
	@install -Dm644 man/tmuzika.sr.1 $(MANDIR)/sr/man1/tmuzika.1
	@install -Dm644 man/tmuzika.sr_Latn.1 $(MANDIR)/sr@latin/man1/tmuzika.1

	@echo "Installation complete"

clean:
	rm -rf $(BUILDDIR)
	rm -rf $(PODIR)/*/*.mo

uninstall:
	rm -f $(BINDIR)/$(TARGET)

	rm -f $(APPDIR)/tmuzika.desktop

	rm -rf $(DOCDIR)
	rm -rf $(LICENSEDIR)

	rm -f $(MANDIR)/man1/tmuzika.1
	rm -f $(MANDIR)/sr/man1/tmuzika.1
	rm -f $(MANDIR)/sr@latin/man1/tmuzika.1

	@for langdir in $(PODIR)/*; do \
		[ -d "$$langdir" ] || continue; \
		lang=$$(basename $$langdir); \
		rm -f $(LOCALEDIR)/$$lang/LC_MESSAGES/$(PACKAGE).mo; \
	done

.PHONY: all install mo clean uninstall