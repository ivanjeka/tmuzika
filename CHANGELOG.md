# Changelog

All notable changes to this project will be documented in this file.

This project follows a simple versioning scheme.
Dates are in format: YYYY-MM-DD.

---

## [1.0] - 2025-12-01

### Added
- Initial public release of **tmuzika**
- Terminal user interface built with **ncurses**
- Audio playback using **GStreamer**
- Integrated file manager:
  - Copy, cut, paste, rename, delete files and folders
  - Undo support
  - Bookmarks support (add, remove, jump 1–9)
  - Toggle hidden files
  - Create new files and folders
  - Change file permissions (chmod)
  - Open text documents from file manager
- Recursive adding of folders with all music inside
- Playlist support:
  - Save playlists to `.m3u`
  - Load `.m3u` playlists
  - Repeat song mode
  - Repeat playlist mode
- Search functionality for songs and files
- Remember last played song
- Keyboard-only control (fully usable without mouse)
- Scroll support with keyboard and mouse wheel
- CLI mode:
  - `tmuzika -p song.mp3`
  - `tmuzika -p playlist.m3u`
- Internationalization (i18n) support:
  - English (default)
  - Serbian Cyrillic
  - Serbian Latin
- Man pages:
  - English (`tmuzika.1`)
  - Serbian Cyrillic (`tmuzika.sr.1`)
  - Serbian Latin (`tmuzika.sr_Latn.1`)
- README documentation:
  - English (`README.md`)
  - Serbian Latin (`README.sr.md`)
  - Serbian Cyrillic (`README.sr-cy.md`)
- Desktop entry file (`tmuzika.desktop`)
- Support for user data directory `~/.tmuzika`

---

## [Unreleased]

### Planned
- Visual theming support
- Config file support
- Playlist sorting
- Additional audio format support
- Performance optimizations
