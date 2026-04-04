CC      ?= gcc
CFLAGS  = -Wall -g `pkg-config --cflags gstreamer-1.0 glib-2.0 ncurses`
LDLIBS  = `pkg-config --libs gstreamer-1.0 glib-2.0 ncurses`

TARGET   = tmuzika
BUILDDIR = build
SRC_DIR  = src
SOURCES  = $(wildcard $(SRC_DIR)/*.c)

PREFIX   ?= /usr
DESTDIR  ?=
BINDIR    = $(DESTDIR)$(PREFIX)/bin
LOCALEDIR = $(PREFIX)/share/locale
PODIR     = po
PACKAGE   = tmuzika

# -----------------------
# izgradnja binarnog fajla
# -----------------------
all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(SOURCES)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDLIBS)

# -----------------------
# gettext (.po -> .mo) za sve jezike
# -----------------------
mo:
	@echo "Kompajliram sve prevode (.po -> .mo)"
	@for po in $(PODIR)/*.po; do \
		lang=$$(basename $$po .po); \
		echo "  -> $$lang"; \
		mkdir -p $(PODIR)/$$lang; \
		msgfmt $$po -o $(PODIR)/$$lang/$(PACKAGE).mo; \
	done

# -----------------------
# instalacija binarnog fajla
# -----------------------
install: all mo
	install -Dm755 $(BUILDDIR)/$(TARGET) \
		$(BINDIR)/$(TARGET)

	@echo "Instaliram sve gettext prevode"
	@for langdir in $(PODIR)/*; do \
		[ -d "$$langdir" ] || continue; \
		lang=$$(basename $$langdir); \
		install -Dm644 $$langdir/$(PACKAGE).mo \
			$(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(PACKAGE).mo; \
	done

# -----------------------
# ciscenje
# -----------------------
clean:
	rm -rf $(BUILDDIR) *.o 
	rm -rf $(PODIR)/*/*.mo

# -----------------------
# uklanjanje iz sistema
# -----------------------
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	@for langdir in $(PODIR)/*; do \
		[ -d "$$langdir" ] || continue; \
		lang=$$(basename $$langdir); \
		rm -f $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(PACKAGE).mo; \
	done

.PHONY: all install install-mo mo clean uninstall