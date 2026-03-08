# tmuzika

Terminalni muzički plejer napisan u C, koristi **ncurses** i **GStreamer**.

tmuzika je dizajniran za brzo i jednostavno puštanje muzike direktno u terminalu, uz potpunu kontrolu preko tastature.
Korisničke konfiguracije i podaci se čuvaju u `~/.tmuzika`.

---

## Slike ekrana

### tmuzika

#### Engleski
![tmuzika Engleski](../images/tmuzika-en.png)

#### Srpski latinica
![tmuzika Latinica](../images/tmuzika-sr-latin.png)

#### Srpski ćirilica
![tmuzika ćirilica](../images/tmuzika-sr-cy.png)


### tmuzika-radio

#### Engleski
![tmuzika-radio Engleski](../images/tmuzika-radio-en.png)

#### Serpski Latinica
![tmuzika-radio Latinica](../images/tmuzika-radio-sr-latin.png)

#### Srpski ćirilica
![tmuzika-radio ćirilica](../images/tmuzika-radio-sr-cy.png)


### tmuzika-fm

#### Engleski
![tmuzika-fm Engleski](../images/tmuzika-fm-en.png)

#### Srpski latinica
![tmuzika-fm Latinica](../images/tmuzika-fm-sr-latin.png)

#### Srpski ćirilica
![tmuzika-fm ćirilica](../images/tmuzika-fm-sr-cy.png)

---

## Karakteristike

- Reprodukcija muzike u terminalu (ncurses interfejs)
- Reprodukcija radio stanica u terminalu (ncurses interfejs)
- Integrisani fajl menadžer (kopiranje, sečenje, lepljenje, preimenovanje, brisanje, vraćanje, obeleživači) 
- Dodavanje pojedinačnih fajlova ili celih foldera (rekurzivno) 
- Snimanje / Učitavanje `.m3u` plejlista 
- Pretraga pesama / radio stanica
- Pamti poslednje puštenu pesmu / radio stanicu
- Skrolovanje kroz liste pomoću tastature ili skrol točka 

Podržani audio formati: `mp3`, `wav`, `flac`, `ogg`, `m4a`, `aac`, `opus`

---

## Tasterske kombinacije

### Reprodukcija

| Taster    | Akcija                           |
|-----------|----------------------------------|
| ENTER     | Pusti izabranu pesmu             |
| SPACE     | Pauza / Nastavak                 |
| b / n     | Pomeri levo/desno                |
| z         | Zaustavi                         |
| > / <     | Sledeća / Prethodna pesma        |
| + / -     | Pojačaj / Stišaj                 |
| <- / ->   | Skok ±10s                        |
| HOME / END| Prva / Poslednja pesma           |
| p         | Ponovi pesmu                     |
| l         | Ponovi celu plejlistu            |
| e         | Nasumično                        |
| s         | Pretraga                         |
| m         | Fajl menadžer                    |
| q         | Sačuvaj plejlistu (.m3u)         |
| u         | Učitaj plejlistu                 |
| v         | Idi na trenutnu pesmu            |
| x         | Ukloni pesmu iz plejliste        |
| DELETE    | Ukloni sve pesme iz plejliste    |
| k         | Izlaz iz programa                |

### Reprodukcija Radio

| Taster    | Akcija                                 |
|-----------|----------------------------------------|
| ENTER     | Pusti radio stanicu                    |
| SPACE     | Pauza / Nastavak                       |
| z         | Zaustavi                               |
| > / <     | Sledeća / Prethodna                    |
| + / -     | Pojačaj / Stišaj                       |
| d         | Dodaj radio stanicu                    |
| HOME / END| Prva / Poslednja stanica               |
| s         | Pretraga                               |
| m         | Fajl menadžer                          |
| q         | Sačuvaj plejlistu (.m3u)               |
| u         | Učitaj plejlistu                       |
| v         | Idi na trenutnu radio stanicu          |
| x         | Ukloni radio stanicu iz plejliste      |
| DELETE    | Ukloni sve radio stanice iz plejliste  |
| ESC       | Izlaz iz Radija                        |


### Fajl menadžer

| Taster       | Akcija                                        |
|--------------|-----------------------------------------------|
| ENTER        | Uđi u direktorijum / dodaj pesmu / dodaj .m3u |
| BACKSPACE    | Vrati se nazad                                |
| ctrl+h       | Prikaži / Sakrij skrivene fajlove             |
| f / F        | Dodaj folder / dodaj trenutni folder          |
| m            | Izaberi fajlove / foldere                     |
| ctrl+a       | Izaberi sve fajlove                           |
| d            | Dodaj više fajlova                            |
| s            | Pretraga                                      |
| n            | Kreiraj novi folder                           |
| t            | Kreiraj novi fajl                             |
| F2           | Preimenuj fajl / folder                       |
| c            | Kopiraj                                       |
| x            | Iseci                                         |
| v            | Zalepi                                        |
| u            | Opozovi poslednju akciju (undo)               |
| ctrl+p       | Promeni dozvole fajla (chmod)                 |
| ctrl+o       | Otvori terminal                    |
| DELETE       | Ukloni fajl / folder                          |
| ctrl+b       | Dodaj obeleživač                              |
| ctrl+d       | Ukloni obeleživač                             |
| 1-9          | Idi na obeleživač 1–9                         |
| ESC          | Izlaz iz fajl menadžera                       |

---

## CLI korišćenje

Puštanje pesme ili plejliste direktno iz terminala:

```bash
tmuzika -p pesma.mp3
tmuzika --play pesma.mp3
tmuzika -p plejlista.m3u
tmuzika --play plejlista.m3u
tmuzika -p Fascikla/
tmuzika --play Fascikla/
```
Puštanje radio stanice ili radio plejliste direktno iz terminala:

```bash
tmuzika -r radio.mp3
tmuzika --radio radio.mp3
tmuzika -r http://radiostanica
tmuzika --radio http://radiostanica
```
Pomoć:

```bash
tmuzika -h
tmuzika --help
```

## Conky Integracija

![tmuzika Conky desktop](../images/tmuzika-conky.png)

Primer desktopa sa Conky-jem koji prikazuje informacije o reprodukciji tmuzike.

tmuzika upisuje trenutne informacije o reprodukciji u privremene fajlove u /tmp.
Fajlovi su privremeni i postoje samo dok tmuzika radi (mogu biti uklonjeni nakon restartovanja sistema).
Ovo omogućava spoljnim alatima kao što je Conky da prikazuju informacije o trenutno reprodukovanoj pesmi.

Zahtevi:
- Conky mora biti instaliran i pokrenut
- tmuzika mora da reprodukuje audio

Šta tmuzika izvezuje:
```
/tmp/tmuzika_status
/tmp/tmuzika_title
/tmp/tmuzika_artist
/tmp/tmuzika_album
/tmp/tmuzika_track
/tmp/tmuzika_elapsed
/tmp/tmuzika_length
```

Primer Conky skripta:

```
TEXT
${if_running tmuzika}
${if_match "${env LANG}" "sr_RS.UTF-8"}
• Слушам: ${execi 3 grep -m1 . /tmp/tmuzika_status} ${execi 2 grep -m1 . /tmp/tmuzika_title}
Извођач: ${execi 3 grep -m1 . /tmp/tmuzika_artist}
Албум: ${execi 3 grep -m1 . /tmp/tmuzika_album}
Трака бр.: ${execi 3 grep -m1 . /tmp/tmuzika_track}
Преостало: ${execi 3 grep -m1 . /tmp/tmuzika_elapsed} | ${execi 2 grep -m1 . /tmp/tmuzika_length}
${else}${if_match "${env LANG}" "sr_RS@latin.UTF-8"}
• Slušam: ${execi 3 grep -m1 . /tmp/tmuzika_status} ${execi 2 grep -m1 . /tmp/tmuzika_title}
Izvođač: ${execi 3 grep -m1 . /tmp/tmuzika_artist}
Album: ${execi 3 grep -m1 . /tmp/tmuzika_album}
Numera: ${execi 3 grep -m1 . /tmp/tmuzika_track}
Vreme: ${execi 3 grep -m1 . /tmp/tmuzika_elapsed} | ${execi 2 grep -m1 . /tmp/tmuzika_length}
${else}
• Now playing: ${execi 3 grep -m1 . /tmp/tmuzika_status} ${execi 2 grep -m1 . /tmp/tmuzika_title}
Artist: ${execi 3 grep -m1 . /tmp/tmuzika_artist}
Album: ${execi 3 grep -m1 . /tmp/tmuzika_album}
Track: ${execi 3 grep -m1 . /tmp/tmuzika_track}
Time: ${execi 3 grep -m1 . /tmp/tmuzika_elapsed} | ${execi 2 grep -m1 . /tmp/tmuzika_length}
${endif}
${endif}
${else}
${endif}
```

### Rešavanje problema

Ako se ništa ne prikazuje:
- proverite da li tmuzika radi
- proverite da li fajlovi /tmp/tmuzika_* postoje
- proverite interval osvežavanja Conky-ja

## Autor i Licenca

Ivan Janković — ivan.jankovic.unix@gmail.com

Licenca: **GPL v3 ili novija**