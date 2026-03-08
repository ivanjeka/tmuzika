# tmuzika

Терминални музички плејер написан у C, користи **ncurses** и **GStreamer**.

tmuzika је дизајниран за брзо и једноставно пуштање музике директно у терминалу, уз потпуну контролу преко тастатуре.
Корисничке конфигурације и подаци се чувају у `~/.tmuzika`.

---

## Слике екрана

### tmuzika

#### Енглески
![tmuzika Енглески](../images/tmuzika-en.png)

#### Српски латиница
![tmuzika латиница](../images/tmuzika-sr-latin.png)

#### Српски ћирилица
![tmuzika ћирилица](../images/tmuzika-sr-cy.png)


### tmuzika-radio

#### Енглески
![tmuzika-radio Енглески](../images/tmuzika-radio-en.png)

#### Српски латиница
![tmuzika-radio латиница](../images/tmuzika-radio-sr-latin.png)

#### Српски ћирилица
![tmuzika-radio ћирилица](../images/tmuzika-radio-sr-cy.png)


### tmuzika-fm

#### Енглески
![tmuzika-fm English](../images/tmuzika-fm-en.png)

#### Српски латиница
![tmuzika-fm Latin](../images/tmuzika-fm-sr-latin.png)

#### Српски ћирилица
![tmuzika-fm Cyrillic](../images/tmuzika-fm-sr-cy.png)

---

## Карактеристике

- Репродукција музике у терминалу (ncurses интерфејс)
- Репродукција радио станица у терминалу (ncurses интерфејс)
- Интегрисани фајл менаџер (копирање, сечење, лепљење, преименовање, брисање, враћање, обележивачи) 
- Додавање појединачних фајлова или целих фолдера (рекурзивно) 
- Снимање / Учитавање `.m3u` плејлиста 
- Претрага песама / радио станица
- Памти последње пуштену песму / радио станицу
- Скроловање кроз листе помоћу тастатуре или скрол точка

Подржани аудио формати: `mp3`, `wav`, `flac`, `ogg`, `m4a`, `aac`, `opus`

---

## Тастерске комбинације

### Репродукција

| Тастер    | Акција                          |
|-----------|---------------------------------|
| ENTER     | Пушта изабрану песму            |
| SPACE     | Пауза / Наставак                |
| b / n     | Помери лево/десно               |
| z         | Заустави                        |
| > / <     | Следећа / Претходна песма       |
| + / -     | Појачај / Стишај                |
| <- / ->   | Скок ±10с                       |
| HOME / END| Прва / Последња песма           |
| p         | Понови песму                    |
| l         | Понови целу плејлисту           |
| e         | Насумично                       |
| s         | Претрага                        |
| m         | Фајл менаџер                    |
| q         | Сачувај плејлисту (.m3u)        |
| u         | Учитај плејлисту                |
| v         | Иди на тренутну песму           |
| x         | Уклони песму из плејлисте       |
| DELETE    | Уклони све песме из плејлисте   |
| k         | Излаз из програма               |


### Reprodukcija Radio

| Тастер    | Акција                                 |
|-----------|----------------------------------------|
| ENTER     | Пусти радио станицу                    |
| SPACE     | Пауза / Наставак                       |
| z         | Заустави                               |
| > / <     | Следећа / Претходна                    |
| + / -     | Појачај / Стишај                       |
| d         | Додај радио станицу                    |
| HOME / END| Прва / Последња станица                |
| s         | Претрага                               |
| m         | Фајл менаџер                           |
| q         | Сачувај плејлисту (.m3u)               |
| u         | Учитај плејлисту                       |
| v         | Иди на тренутну радио станицу          |
| x         | Уклони радио станицу из плејлисте      |
| DELETE    | Уклони све радио станице из плејлисте  |
| ESC       | Излаз из Радија                        |


### Фајл менаџер

| Тастер       | Акција                                        |
|--------------|-----------------------------------------------|
| ENTER        | Уђи у директоријум / додај песму / додај .m3u |
| BACKSPACE    | Врати се назад                                |
| ctrl+h       | Прикажи / Сакриј скривене фајлове             |
| f / F        | Додај фолдер / додај тренутни фолдер          |
| m            | Изабери фајлове / фолдере                     |
| ctrl+a       | Изабери све фајлове                           |
| d            | Додај више фајлова                            |
| s            | Претрага                                      |
| n            | Креирај нови фолдер                           |
| t            | Креирај нови фајл                             |
| F2           | Преименуј фајл / фолдер                       |
| c            | Копирај                                       |
| x            | Исече                                         |
| v            | Залепи                                        |
| u            | Опозови последњу акцију (undo)                |
| ctrl+p       | Промени дозволе фајла (chmod)                 |
| ctrl+o       | Отвори терминал                    |
| DELETE       | Уклони фајл / фолдер                          |
| ctrl+b       | Додај обележивач                              |
| ctrl+d       | Уклони обележивач                             |
| 1-9          | Иди на обележивач 1–9                         |
| ESC          | Излаз из фајл менаџера                        |

---

## CLI коришћење

Пуштање песме или плејлисте директно из терминала:

```bash
tmuzika -p песма.mp3
tmuzika --play песма.mp3
tmuzika -p плејлиста.m3u
tmuzika --play плејлиста.m3u
tmuzika -p Фасцикла/
tmuzika --play Фасцикла/
```
Пуштанје радио станице или радио плејлисте директно из терминала:

```bash
tmuzika -r радио.mp3
tmuzika --radio радио.mp3
tmuzika -r http://радиостаница
tmuzika --radio http://радиостаница
```
Помоћ:

```bash
tmuzika -h
tmuzika --help
```

## Conky Интеграција

![tmuzika Conky desktop](../images/tmuzika-conky.png)

Пример десктопа са Conky-јем који приказује информације о репродукцији tmuzika-е.

tmuzika уписује тренутне информације о репродукцији у привремене фајлове у /tmp.
Фајлови су привремени и постоје само док tmuzika ради (могу бити уклоњени након рестартовања система).
Ово омогућава спољним алатима као што је Conky да приказују информације о тренутно репродукованој песми.

Захтеви:
- Conky мора бити инсталиран и покренут
- tmuzika мора да репродукује аудио

Шта tmuzika извозује:
```
/tmp/tmuzika_status
/tmp/tmuzika_title
/tmp/tmuzika_artist
/tmp/tmuzika_album
/tmp/tmuzika_track
/tmp/tmuzika_elapsed
/tmp/tmuzika_length
```

Пример Conky скрипта:

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

### Решавање проблема

Ако се ништа не приказује:
- проверите да ли tmuzika ради
- проверите да ли фајлови /tmp/tmuzika_* постоје
- проверите интервал освежавања Conky-ја

## Аутор и Лиценца

Иван Јанковић — ivan.jankovic.unix@gmail.com

Лиценца: **GPL v3 или новија**