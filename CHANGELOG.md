# Changelog

All notable changes to this project will be documented in this file.

Dates are in YYYY-MM-DD format.

---

## [1.1.3] - 2026-06-28

### Added

* Radio playback support.
* Support for loading radio playlists (`.m3u`).
* Long command-line options:

  * `--play`
  * `--radio`
  * `--help`
* Conky integration through `/tmp/tmuzika_*` status files.
* Color configuration menu.
* `INSTALL.md` installation guide.
* English and Serbian manual pages.
* Improved documentation.

### Changed

* Migrated user data from `~/.tmuzika` to the XDG Base Directory specification:

  * `~/.config/tmuzika`
  * `~/.cache/tmuzika`
  * `~/.local/share/tmuzika`
* Improved file manager functionality and usability.
* Improved playlist handling.
* Updated CLI interface.
* Updated documentation and manual pages.
* Improved internationalization (i18n).

### Fixed

* Various file manager issues.
* Improved playback stability.
* Fixed playlist-related issues.
* General bug fixes and code cleanup.

---

## [1.0] - 2025-12-01

### Added

* Initial public release of **tmuzika**.
* Terminal user interface built with **ncurses**.
* Audio playback using **GStreamer**.
* Integrated file manager:

  * Copy, cut, paste, rename, and delete files and folders.
  * Undo support.
  * Bookmark management (add, remove, and jump to bookmarks 1–9).
  * Toggle hidden files.
  * Create new files and folders.
* Recursive folder import for adding all supported music files.
* Playlist support:

  * Save playlists as `.m3u`.
  * Load `.m3u` playlists.
  * Repeat current song.
  * Repeat playlist.
* Search functionality for songs and files.
* Remember the last played song.
* Full keyboard control.
* Keyboard and mouse wheel scrolling.
* Command-line interface:

  * `tmuzika -p song.mp3`
  * `tmuzika -p playlist.m3u`
* Internationalization (i18n):

  * English (default)
  * Serbian Cyrillic
  * Serbian Latin
* Manual pages:

  * English (`tmuzika.1`)
  * Serbian Cyrillic (`tmuzika.sr.1`)
  * Serbian Latin (`tmuzika.sr_Latn.1`)
* Documentation:

  * `README.md`
  * `README.sr.md`
  * `README.sr-cy.md`
* Desktop entry (`tmuzika.desktop`).
* Support for the `~/.tmuzika` user data directory.

---

## [Unreleased]

### Planned

* Visual theming support.
* Configuration file support.
* Playlist sorting.
* Additional audio format support.
* Performance optimizations.