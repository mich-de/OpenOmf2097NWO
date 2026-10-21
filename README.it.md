<div align="center">

# ONE MUST FALL 2097 — NWO

### Il classico fighting con i robot del 1994, che gira nativamente nel browser

[English](README.md) · **Italiano**

![OpenOMF WebAssembly](https://img.shields.io/badge/OpenOMF-WebAssembly-0b7285?style=for-the-badge)
![Emscripten SDL2](https://img.shields.io/badge/Emscripten-SDL2-1f6feb?style=for-the-badge)
![WebGL2](https://img.shields.io/badge/WebGL2-GLES3_shim-2e7d32?style=for-the-badge)
![Touch controls](https://img.shields.io/badge/touch-pad-PUNCH-orange?style=for-the-badge)
![Released 1994](https://img.shields.io/badge/released-1994-c62828?style=for-the-badge)

[![▶ GIOCA ONLINE](https://img.shields.io/badge/%E2%96%B6_GIOCA_ONLINE-mich--de.github.io%2FOpenOmf2097NWO-ff6d2e?style=for-the-badge)](https://mich-de.github.io/OpenOmf2097NWO/)

**▶ [Gioca subito nel browser → mich-de.github.io/OpenOmf2097NWO](https://mich-de.github.io/OpenOmf2097NWO/)**

<img src="screenshots/GAMEPLAY_native_fight.png" alt="Trump (Jaguar) e Xi (Flail) combattono in una build WebAssembly nativa di OpenOMF" width="820">

*Build nativa WebAssembly (OpenOMF): niente emulatore, WebGL2, il frame 320x200 del gioco originale.*

</div>

---

## Gioca online

Niente installazione: apri **https://mich-de.github.io/OpenOmf2097NWO/** e premi **PLAY (WASM, NO EMULATOR)**. La pagina scarica il motore e i dati di gioco (~19 MB la prima volta, poi messi in cache dal browser con una voce nuova a ogni deploy) e parte subito. Niente DOS, niente emulatore: è [OpenOMF](https://github.com/omf2097/openomf), la reimplementazione open source, compilata in WebAssembly.

- **Tastiera:** il giocatore 1 usa le frecce, `Enter` per colpire, `Right Shift` per calciare. Il giocatore 2 gioca con `WASD` + `QEZC`, `Left Ctrl` per colpire, `Left Shift` per calciare. `Esc` apre il menu, `Enter` conferma. `F11` o **FULL SCREEN** per lo schermo intero.
- **Telefono e tablet:** appare un pad sulle schermate — croce in basso a sinistra, **PUNCH** e **KICK** in basso a destra, **ESC** in alto a destra. Il pad invia le stesse tasti della tastiera, quindi il gioco non nota differenza.
- **Singolo giocatore e due giocatori locali** sono entrambi supportati; il gioco online non è collegato (NETWORK PLAY nel menu richiede un server che questo sito non gestisce).
- **Reti aziendali:** filtri web come Zscaler possono tagliare il download (il pacchetto dati sembra un archivio di programmi DOS). La pagina lo dice e offre di riprovare; la soluzione è un'altra rete, o chiedere a IT di consentire `mich-de.github.io`.

One Must Fall 2097 è freeware dal 1999. Questo repository è anche il sito: GitHub Pages lo serve dalla radice.

## I piloti

Un'edizione satirica: i piloti sono figure pubbliche dei conflitti e della diplomazia degli anni 2020. Le loro battute sono vanterie da personaggio, non fatti né citazioni, e niente qui è affiliato a loro o approvato da loro. Le statistiche sommano a 30 come quelle dei piloti originali, scelte dall'immagine pubblica di ciascuno: potenza, agilità, resistenza.

| | Pilota | Potenza | Agilità | Resistenza | AI |
|---:|---|---:|---:|---:|---|
| 1 | Trump | 15 | 5 | 10 | avanza e afferra |
| 2 | Musk | 8 | 13 | 9 | sta a distanza e spara |
| 3 | Meloni | 9 | 14 | 7 | salta addosso |
| 4 | Macron | 10 | 13 | 7 | attacca su tutto |
| 5 | Scholz | 8 | 6 | 16 | difensivo |
| 6 | Starmer | 10 | 9 | 11 | metodico |
| 7 | Von der Leyen | 9 | 9 | 12 | logora |
| 8 | Rutte | 7 | 10 | 13 | difensivo |
| 9 | Xi | 12 | 6 | 12 | potenza |
| 10 | Kim | 16 | 7 | 7 | spara da lontano |
| 11 | Putin | 13 | 5 | 12 | potenza |
| 12 | Zelensky | 7 | 16 | 7 | veloce e saltellante |
| 13 | Belousov | 11 | 5 | 14 | logora |
| 14 | Umerov | 8 | 14 | 8 | spara da lontano |
| 15 | Lukashenko | 13 | 6 | 11 | avanza e afferra |
| 16 | Netanyahu | 14 | 8 | 8 | fa di tutto |
| 17 | Katz | 12 | 9 | 9 | costante |
| 18 | Khamenei | 9 | 6 | 15 | spara da lontano |
| 19 | Al-Houthi | 8 | 15 | 7 | salta addosso |
| 20 | Erdogan | 11 | 10 | 9 | attacca su tutto |
| 21 | Bin Salman | 12 | 9 | 9 | costante |
| 22 | Al Thani | 6 | 12 | 12 | equilibrato |
| 23 | Al-Sisi | 9 | 7 | 14 | difensivo |
| 24 | Orban | 8 | 10 | 12 | logora |

## Gameplay

| | |
|:---:|:---:|
| <img src="screenshots/GAMEPLAY_native_menu.png" width="400"><br>Menu principale, build nativa | <img src="screenshots/GAMEPLAY_pilots.png" width="400"><br>24 piloti su due pagine |
| <img src="screenshots/GAMEPLAY_versus.png" width="400"><br>Trump contro Xi: la schermata VS | <img src="screenshots/GAMEPLAY_native_fight.png" width="400"><br>Trump contro Xi: fight dal vivo con HUD e barre della salute |
| <img src="screenshots/GAMEPLAY_touch_menu.png" width="400"><br>Telefono: menu con il pad su schermo | <img src="screenshots/GAMEPLAY_touch_splash.png" width="400"><br>Telefono: splash, controlli agli angoli |
| <img src="screenshots/GAMEPLAY_robots.png" width="400"><br>Dodici robot: Nova e Ocelot nella sesta colonna | <img src="screenshots/GAMEPLAY_ocelot.png" width="400"><br>Ocelot sputa fuoco |

## Per iniziare

```bash
git clone https://github.com/mich-de/OpenOmf2097NWO.git
cd OpenOmf2097NWO
python3 -m http.server 8734
```

Apri **http://127.0.0.1:8734/** e premi **PLAY (WASM, NO EMULATOR)**. Aggiungi `?touch=1` all'indirizzo per provare i controlli mobile al computer (`?touch=0` li nasconde).

## Punti di forza

- **Nativo, niente emulatore.** Il gioco stesso — [OpenOMF](https://github.com/omf2097/openomf) in C con SDL2 — compilato in WebAssembly e reso su WebGL2. Menu, fight e musica girano nella pagina: niente DOS, niente js-dos, niente CPU emulata.
- **Una pagina per desktop e mobile.** Gli schermi tattili ricevono pad e pulsanti; le tastiere funzionano così come sono.
- **Un pad fatto per il fight.** Sintetizza gli stessi eventi DOM della tastiera, quindi il motore riceve esattamente quello che premerebbe un giocatore 1. Una diagonale tiene due frecce, come su tastiera, così il pollice che rotella da down a down-back aggiunge solo indietro e non passa mai per il neutro a metà mossa. Le diagonali verso il basso (blocco accovacciato, quarter circle) hanno settori più larghi, e ogni direzione resta attiva finché il pollice non è passato di 10° oltre il bordo, così un pollice tremolante non fa lampeggiare tra due direzioni.
- **Un launcher fatto con l'arte del gioco.** Sfondo, lettering e pannelli vengono dalle schermate NetArena del 1994 incluse nel gioco; i pulsanti tattili sono disegnati pixel per pixel con lo stesso font bitmap.
- **Mai più un motore vecchio.** La pagina fissa `openomf.js`, `openomf.wasm` e `openomf.data` per ETag, quindi ogni deploy è una voce di cache nuova e i giocatori di ritorno non tengono mai un motore obsoleto.

## La build nativa WebAssembly

`index.html` gira il gioco come WebAssembly invece di emulare la macchina per cui è stato scritto:

- **Il motore.** [OpenOMF](https://github.com/omf2097/openomf) (C, SDL2) pinnato al commit `4fd5be9`, compilato con Emscripten. SDL2, SDL2_mixer, libpng e zlib vengono dai port di Emscripten; enet, libxmp e libconfuse sono costruiti come librerie statiche WebAssembly; libepoxy è sostituito da un piccolo shim che serve `epoxy/gl.h` come GLES3, che è ciò che parla WebGL2.
- **Shader.** Gli shader GLSL 3.00 sono tradotti per GL ES a build time (in `tools/web/build_openomf.sh`): `highp usampler2D`, niente `gl_FragCoord` ridefinito, niente mixing `int`/`float`, bias `textureLod` come float.
- **Colore.** WebGL2 non può renderizzare in `RGBA16`, quindi il render target indicizzato dalla palette è `RGBA16F`; la mantissa half-float tiene ogni indice esatto perché la decodifica quantizza con `int(v * 1023.0 + 0.5)`. Il canale dei remap è arrotondato allo stesso modo (`int(v * 255.0 + 0.5)`): troncato, un 233/255 in half-float tornava 232, la tabella di remap sbagliata, e le ombre di Shadow uscivano nere piene invece che scure e dettagliate.
- **Tempi.** GitHub Pages non serve header `COOP`/`COEP`, quindi niente thread e `SDL_Delay` andrebbe in busy-wait. La build usa ASYNCIFY con main loop bloccante: ogni frame finisce in `emscripten_sleep`, scandito a 60 Hz contro `emscripten_get_now`.
- **La console.** La console di gioco legge stdin con `read(0, ...)`, che in un browser non ritorna mai e bloccava la scheda; è saltata sotto `__EMSCRIPTEN__`. Il log della console va in `window.__omflog` (`omf-debug.html` lo mostra da solo).
- **I file di lingua.** `ENGLISH.DAT2`/`GERMAN.DAT2` non possono essere generati dal `languagetool` in wasm (gira a build time), quindi una copia host dello strumento li produce e finiscono nel pacchetto dati; senza di loro `lang_init()` va in abort.
- **I file.** I dati di gioco sono pacchettizzati in `openomf.data`; punteggi e configurazione vivono nella storage del browser. Il primo caricamento tira su `openomf.js` (239 KB), `openomf.wasm` (~3,1 MB) e `openomf.data` (~15,4 MB), ciascuno fissato per ETag così un deploy non lascia mai una copia vecchia.
- **Dodici robot.** La schermata di scelta del robot ha una sesta colonna: sopra Nova, che così si sceglie come gli altri robot (il trucco della katana funziona ancora), sotto Ocelot. `MELEE.BK` ha la grafica solo per dieci robot: la griglia di marmo viene ritagliata dallo sfondo al caricamento (celle da 51px a 46px di larghezza), e ritratti e animazioni dei due robot in più vengono dai loro file `.AF`. La pagina dei piloti resta a cinque colonne.
- **Ocelot.** Un dodicesimo robot, nuovo in questa build: Jaguar ridipinto a mano come un giaguaro, con il manto maculato, la testa da felino e la coda, e uno sputo di fuoco al posto del concussion cannon (giù, giù-indietro, indietro + pugno). I 154 frame stanno in `tools/web/ocelot/art` come PNG a 4x, uno per sprite di Jaguar; a build time `tools/web/ocelot/ocelot.c` li riporta ai pixel del gioco e scrive `FIGHTR11.AF` (le mosse), `FIGHTR11.X16` (pixel a 16 bit con tutti i colori del disegno, 174) e un ritratto maculato in `VS.BK`. Ocelot combatte con le mosse di Jaguar e l'AI lo gioca come Jaguar. È un robot da melee: il torneo resta con i dieci originali.
- **Full HD e oltre.** Il canvas prende i pixel reali dello schermo (1920x1080 a tutto schermo su uno schermo full HD, di più su quelli ad alta densità) e il gioco disegna in un framebuffer alto quasi altrettanto, 6 volte i 320x200 originali per 1080p, poi lo adatta allo schermo in 4:3 come il monitor dell'epoca. Ridimensionare la finestra o passare a tutto schermo lo riadatta.
- **I piloti NWO.** Al posto dei dieci originali, 24 figure delle guerre e della diplomazia di oggi, su due pagine della griglia piloti (una riga "PAGE 1 OF 2" e frecce lampeggianti ai bordi; oltre il bordo si passa all'altra); l'ultimo avversario resta il Major Kreissack. Ognuno ha statistiche, colori del robot del suo paese, una personalità per l'AI, bio, battute nel VS e finali (`tools/web/nwo-pilots.patch`, `src/game/nwo_pilots.c`). I ritratti sono caricature girate di tre quarti verso destra come i volti originali, disegnate da `tools/web/pilots/portraits.py` (o qualsiasi PNG 240x240 in `tools/web/pilots/art`), 32 colori ciascuno dalla palette estesa.
- **Volti in arena.** In single player l'HUD mostra i volti dei due piloti, come l'originale fa nei tornei, con le luci dei round sotto.
- **Schermo intero, dettagli massimi.** PLAY passa a schermo intero in orizzontale (bloccato dove il browser lo permette, come Android) con framebuffer 6x e robot 4x; il dettaglio cala durante il gioco solo se i frame non reggono, e con WebGL in software parte basso. Le scelte del menu VIDEO (FB SCALE, con il nuovo AUTO; SCALING; ASPECT) restano salvate nel browser e valgono a ogni avvio, anche da telefono; `?quality=low|mid|high` forza un livello, `?debug=1` mostra GPU, dimensioni e log del motore. Il rehit è attivo di default.
- **Colori oltre 256.** Il gioco originale dà a un robot i 47 colori del pilota. OpenOMF disegna già con una palette da 1024 voci, quindi questa build permette a un robot di portare i suoi: un file `.X16` accanto al suo `.AF` contiene pixel a 16 bit e fino a 383 colori per giocatore (quelli del giocatore 1 nelle voci 257..639, del giocatore 2 in 641..1023; lo shader della palette sposta quelli del giocatore 2, come fa con le rampe del pilota). Luci, lampi dei colpi e dissolvenze toccano anche quei colori, e il rilevamento dei colpi li conta come corpo del robot.
- **Poteri del torneo in ogni modalità.** Nell'originale i potenziamenti di un robot (comprati nel torneo) cambiano le sue mosse speciali: Shadow lancia le ombre più lontano e ottiene un'ombra che congela (avanti-giù, giù, giù-indietro, indietro + pugno), Katana aggiunge lame, e così via. Fuori dal torneo questa build dà a ogni robot tutti e tre i potenziamenti, per entrambi i giocatori e per l'AI; il torneo tiene i livelli che compri. La presa d'ombra di Shadow (giù, giù + pugno) funziona come sempre.
- **Kreissack a ogni difficoltà.** In singolo, battuti tutti si affronta il Major Kreissack anche a Rookie; l'originale sotto Veteran ti rimandava a casa.
- **Patch.** Ogni modifica alle sorgenti è una patch in `tools/web/` applicata dallo script di build sul checkout pinnato, così la build resta riproducibile.

```bash
source ~/emsdk/emsdk_env.sh
tools/web/build_deps.sh      # una volta: enet, libxmp, libconfuse
tools/web/build_openomf.sh   # clona/pinna OpenOMF, applica le patch, compia, copia in wasm/
# OMF_EM_DEBUG=1 tools/web/build_openomf.sh   # build con assert e log
```

## I controlli tattili, e come arrivano al motore

Il motore legge gli eventi tastiera di SDL2 dal DOM, quindi il pad non inventa un protocollo nuovo: sintetizza gli stessi `KeyboardEvent` che manderebbe una tastiera, con il `keyCode` legacy che SDL legge ancora, e li dispaccia sul canvas di gioco. Una pressione è un `keydown`, il rilascio un `keyup`; il set dei tasti premuti fa sì che un cambio di direzione prema la nuova freccia prima di rilasciare quella vecchia, esattamente come due tasti che si sovrappongono su tastiera.

| Pulsante | Invia |
|---|---|
| Croce pad | Frecce (le diagonali ne tengono due) |
| **PUNCH** | `Enter` |
| **KICK** | `Right Shift` (`code: "ShiftRight"`) |
| **ESC** | `Esc` |

I controlli compaiono automaticamente sugli schermi tattili; `?touch=1` li forza attivi, `?touch=0` li spegne. Ogni tasto viene rilasciato al blur della finestra e su `visibilitychange`, così un pollice sperduto non lascia il gioco sempre in cammino.

## Costruito con

| Cosa | A cosa serve |
|---|---|
| [OpenOMF](https://github.com/omf2097/openomf), pinnato a `4fd5be9` | Il motore di gioco, compilato in WebAssembly: menu, fight, musica, salvataggi |
| Emscripten (port SDL2, SDL2_mixer, libpng, zlib) | La toolchain e le sue librerie di sistema |
| enet, libxmp, libconfuse, come librerie statiche WebAssembly | Sorgenti di rete, musica moduli (`.PSM`), file di configurazione |
| WebGL2 | Il renderer, tramite un shim GLES3 per libepoxy |
| GitHub Pages | Hosting, dalla radice di questo repository |
| [REA](https://github.com/morluto/rea) + Chrome DevTools Protocol | Test browser headless: tasti, touch, screenshot, log di rete |
| [fontTools](https://github.com/fonttools/fonttools) | Trasformare i font bitmap del gioco in TTF e WOFF2 |

## I font del gioco, come file di font veri

<img src="assets/fonts/specimen.png" alt="I tre font pixel di OMF 2097 renderizzati in Chrome" width="820">

I font bitmap del gioco, convertiti in TTF e WOFF2 in [`assets/fonts/`](assets/fonts/). Ogni pixel è un quadrato pieno, quindi restano nitidi alla loro dimensione e a multipli interi.

| Font | Da | Cella | Caratteri |
|---|---|---|---|
| **OMF 2097 NetArena** | `NETFONT1.PCX`, il lettering NetArena del launcher | 9 px, proporzionale | ASCII |
| **OMF 2097 Big** | `GRAPHCHR.DAT`, il font dei menu e dell'HUD | 8x8 px | ASCII + lettere e simboli code page 437 |
| **OMF 2097 Small** | `CHARSMAL.DAT`, il font piccolo del gioco | 6x6 px | ASCII (solo maiuscole) + lettere code page 437 |

```css
@font-face { font-family: "OMF 2097 Big"; src: url(assets/fonts/omf2097-big.woff2) format("woff2"); }
h1 { font: 32px "OMF 2097 Big"; } /* 4 x 8 px: ogni pixel cade su interi pixel dello schermo */
```

Rigenerali con `pip install fonttools brotli`, poi `python3 tools/build_fonts.py`.

## Come è stato testato

- **La build nativa, screenshot per screenshot.** `cdp_shot.js` avvia la pagina in Chrome headless, clicca **PLAY**, tiene premuto ogni tasto abbastanza a lungo per i menu letti per polling (`SDL_GetKeyboardState` legge lo stato, quindi una pressione brevissima viene persa) e cattura splash → menu principale → selezione personaggio → fight dal vivo con HUD, barre della salute e proiettili. WebGL headless richiede `--use-gl=angle --use-angle=swiftshader --enable-unsafe-swiftshader`.
- **Il pad, con touch veri.** Il Chrome DevTools Protocol manda punti `touchStart`/`touchEnd` reali sulle braccia e sui pulsanti del pad: tenendo premuto il braccio destro e premendo PUNCH il gioco va da splash a menu principale a schermata di selezione dei robot, e il pulsante ESC esce pulito (`GAME CLOSED (code 0)`).
- **Un avversario che non si muove.** Mosse e diagonali verificate in una partita TWO PLAYER GAME, dove il giocatore 2 non tocca tasti.
- **Menu che saltano.** A volte i menu del gioco prendono una pressione due volte, quindi gli script leggono la riga evidenziata dal frame (quella più chiara) e correggono il cursore prima di confermare.

## Cosa c'è nel repository

| Percorso | Cos'è |
|---|---|
| `index.html` | La pagina di gioco: bottone PLAY, launcher, controlli tastiera e tattili |
| `netfont.js` | Il font bitmap `NETFONT1.PCX`, disegnato pixel per pixel: lettering del launcher e pulsanti tattili |
| `wasm/` | La build nativa servita dalla pagina: `openomf.js`, `openomf.wasm`, `openomf.data` |
| `tools/web/` | Script di build web, patch guidate (GL probe, RGBA16F, stdin, frame pacing, opus), shim GLES3, stage probe |
| `omf-debug.html` | Harness minimale solo-log per la build nativa (`window.__omflog`) |
| `assets/` | Arte del launcher presa dal gioco (sfondo `NETMEGA.PCX`, logo) |
| `assets/fonts/` | I tre font bitmap del gioco come TTF e WOFF2, più uno specimen |
| `cdp_shot.js` | Guida Chrome via CDP: carica, attende, clicca, screenshot, eval |
| `screenshots/` | Screenshot di gameplay e quelli di sviluppo mostrati sopra |

## Strumenti

```bash
# Ricostruisci la build WebAssembly nativa (una volta: enet, libxmp, libconfuse)
source ~/emsdk/emsdk_env.sh
tools/web/build_deps.sh
tools/web/build_openomf.sh        # OMF_EM_DEBUG=1 per una build con assert

# Screenshot della pagina in Chrome vero
google-chrome-stable --headless=new --remote-debugging-port=9333 --remote-allow-origins='*' --window-size=1280,720 about:blank &
node cdp_shot.js "http://127.0.0.1:8734/" 20000 out.png [x,y] [waitAfterClickMs]
```
