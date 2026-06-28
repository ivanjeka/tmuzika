# Installation Guide – tmuzika

This document explains how to build and install **tmuzika** from source.

tmuzika is a terminal-based music player written in C, using **ncurses** and **GStreamer**.

---

## Screenshots

### tmuzika

#### English
![tmuzika English](../images/tmuzika-en.png)

### tmuzika-radio

#### English
![tmuzika-radio English](../images/tmuzika-radio-en.png)


### tmuzika-fm

#### English
![tmuzika-fm English](../images/tmuzika-fm-en.png)

---

## Dependencies

You need the following packages installed:

- gcc
- make
- pkg-config
- ncurses
- glib2
- gstreamer
- gst-plugins-base
- gst-plugins-good

### Arch Linux

```bash
sudo pacman -S gcc make pkgconf ncurses glib2 gstreamer gst-plugins-base gst-plugins-good
```

### DEBIAN / UBUNTU

```bash
sudo apt install build-essential pkg-config libncurses5-dev libglib2.0-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-tools gstreamer1.0-plugins-good
```

### FEDORA

```bash
sudo dnf install gcc make pkg-config ncurses-devel glib2-devel gstreamer1-devel gstreamer1-plugins-base-devel
```

## Troubleshooting

### No audio / playback fails

Make sure you have installed GStreamer plugins:

- gst-plugins-base
- gst-plugins-good

Without these, tmuzika may start but fail to play audio.

### Build from Source

```bash
git clone https://github.com/ivanjeka/tmuzika.git
cd tmuzika
make
sudo make install
```

### Run

```bash
tmuzika
```

### Uninstall

```bash
sudo make uninstall
```

## License

tmuzika is licensed under **GPL v3 or later**.

Author: Ivan Janković — ivan.jankovic.unix@gmail.com
