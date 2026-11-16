#  <img src="dat/icon/default.png" width="24" height="24" /> render2

Un motore di rendering sperimentale scritto da zero in C, GLAD e GLFW3.

## Funzionalità
Pipeline di rendering 3D accelerata via OpenGL:
- Programmabile via shader GLSL (con preprocessore per direttive `#include` ricorsive);
- Con supporto alle mesh in formato `.obj`, materiali in formato `.mtl`, e texture in formato `.tga` senza compressione RLE (con supporto allo spazio colore sRGB);
- Tipi dedicati al rendering (telecamere con controlli FOV, near e far plane; oggetti "atmosfera" per vettori di illuminazione, luce solare ed ambientale);
- Skybox a cubo e riflessioni ambientali da mappe equirettangolari con filtro mip-map dipendente dalla rugosità ed effetto Fresnel;
- Shading avanzato con supporto a subsurface scattering per la pelle e modello speculare a doppio lobo.

Architettura ad entità:
- Sistema flessibile ed estendibile basato su campi polimorfi (`field`) con vtable dedicate (`int`, `float`, `string`, `float2/3/4`, `mat2/3/4`, `quat`, `transform`, `camera`, `atmosphere`, `texture`, `mesh`, `material`, `script`, `shader`);
- Gerarchia ad albero delle entità con composizione delle trasformazioni (posizione, rotazione su quaternioni e scala).

Motore di scripting Scheme/Lisp integrato:
- Interprete Lisp completo sviluppato da zero con allocatore di memoria ad arena dedicato;
- Hook di ciclo di vita (`start` e `update`) con calcolo di `delta-time` e binding all'entità (`self`);
- Supporto matematico esteso direttamente da script (vettori, matrici con determinante e inversa, quaternioni ed angoli di Eulero);
- Manipolazione dell'albero delle entità (`find-entity`, `get-entity-child`, ecc.) e accesso ai campi (`get-field`, `set-field`, `require-field`);
- Accesso in tempo reale allo stato di input di tastiera e mouse.

Interfaccia utente grafica ed editor:
- Toolkit GUI autonomo con backend OpenGL e supporto a temi visivi (Dark, Light, Neo, Tux, Tokyo);
- Editor multi-finestra con vista scena, gerarchia ad albero ed ispettore dei campi;
- Finestre di selezione interattiva per le risorse (texture, mesh, materiali, script, shader) e file explorer integrato.

Serializzazione dei dati:
- Parser e serializer JSON dedicato per il salvataggio e caricamento dello stato di scene e singole entità prefabbricate.

Gestione del sistema ed eccezioni:
- Meccanismo di eccezioni `try`/`catch`/`throw` basato su `setjmp`/`longjmp`;
- Sistema di logging strutturato con livelli di severità e categorie di sottosistema;
- Gestione dello stato di input globale (tasti, bottoni mouse, coordinate e rotella di scorrimento).

L'intero progetto è stato sviluppato con l'obiettivo di essere il più educativo possibile:
- Tutto è sviluppato da zero, in modo da mostrare come funziona ogni passo dello sviluppo di un motore grafico;
- Le soluzioni adottate sono standard e (idealmente) semplici da capire per chi ha un po' di familiarità con altri motori (come Unity);
- Il motore è stato sviluppato per essere versatile e gestire progetti di vario tipo.

## Requisiti di Sistema

Su distribuzioni Linux (Debian/Ubuntu e derivate), puoi installare le dipendenze (GLFW3, OpenGL e libm) con:
```
sudo apt update
sudo apt install build-essential libglfw3-dev libgl1-mesa-dev
```

Quindi hai bisogno di un compilatore C (GCC o Clang vanno bene).

## Compilare

Per ottenere la repository puoi fare la `clone`:
```bash
git clone https://github.com/seggiani-luca/render2.git
```

Quindi c'è un `Makefile` per compilare tutto:
```bash
make run    # compila il progetto e lo esegue
make        # compila soltanto
make debug  # avvia GDB per il debugging, con qualche soppressione abilitata
            # (controlla il Makefile)
make asan   # compila ed esegue con AddressSanitizer, reindirizzando su tst/asan.txt
make clean  # ripulisce gli oggetti
```

## Struttura

L'idea è che puoi fare la `clone` dell'intera repository render2, ed usare la directory madre come base per il tuo progetto.

L'organizzazione che è stata adottata in fase di sviluppo è la seguente:
```
├── dat                # asset (texture, mesh, ecc...)
│   ├── entity         # entità serializzate (.json)
│   ├── icon           # icone delle finestre
│   ├── material       # materiali (.mtl)
│   ├── model          # modelli (.obj)
│   ├── scene          # scene serializzate (.json)
│   ├── script         # script Scheme/Lisp (.scm)
│   │   └── includes   # include e libreria di base per script
│   ├── shader         # shader frag e vert (.glsl)
│   │   └── includes   # include e funzioni comuni per shader
│   └── texture        # texture (.tga senza RLE) e cubemap
├── lib                # librerie e componenti disaccoppiati
│   ├── glad           # libreria GLAD
│   ├── gui            # toolkit GUI dedicato con temi e widget
│   ├── json           # motore di serializzazione/deserializzazione JSON dedicato
│   └── window         # wrapper finestre OpenGL su GLFW
├── src                # codice sorgente
│   ├── arena          # allocatore di memoria bump/arena
│   ├── data           # caricamento dati dal disco e tabelle dei riferimenti
│   │   ├── material   # caricamento e decodifica materiali
│   │   ├── mesh       # caricamento e decodifica mesh, upload alla GPU
│   │   ├── script     # caricamento e decodifica script
│   │   ├── shader     # caricamento, preprocessamento e compilazione shader
│   │   └── texture    # caricamento e decodifica texture
│   ├── editor         # interfaccia utente dell'editor
│   │   ├── hierarchy  # GUI della gerarchia delle entità
│   │   ├── inspector  # GUI dell'ispettore dei campi
│   │   ├── selector   # selettori di asset e file explorer
│   │   └── widget     # widget specializzati per l'editor
│   ├── exception      # gestione eccezioni (try/catch/throw) e logging strutturato
│   ├── input          # gestione dello stato di tastiera e mouse
│   ├── math           # libreria di algebra lineare (vettori, matrici, quaternioni, transform)
│   ├── parse          # utilità di parsing e preprocessore #include
│   ├── render         # rendering della scena e skybox
│   ├── scene          # gestione delle scene (grafi di entità)
│   │   └── entity     # definizione di entità con campi polimorfi
│   ├── script         # motore di scripting Scheme/Lisp
│   │   ├── field      # binding campi entità per lo scripting
│   │   ├── lisp       # interprete Scheme/Lisp (eval, ambienti, valori)
│   │   └── math       # funzioni matematiche per lo scripting
│   ├── serial         # serializzazione e deserializzazione di scene ed entità
│   └── time           # calcolo del delta time e gestione temporale
└── tst                # soppressioni GDB, test vari
```
