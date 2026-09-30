# Blob Wars (mod) — Registro de archivos

Documento vivo: cada archivo que vamos viendo se anota acá con su función, qué aspectos del juego maneja y qué hay que tener en cuenta al tocarlo.

**Estado de cada entrada**
- **Visto**: se leyó el código en alguna sesión. No implica que el archivo esté disponible en la sesión actual.
- **Sin ver**: se sabe que existe, pero no hay una entrada con su contenido.
- **Por ubicar**: se sabe que existe o hace falta, pero no se sabe dónde está.

**Última actualización:** 2026-09-30

**Ubicación de los fuentes:** `C:\Users\Usuario\Desktop\Proyectos\blockwars\src`

---

## Índice

| Archivo | Estado | Área del juego |
|---|---|---|
| `headers.h` | Visto | Cabecera maestra: librerías y mapa de todas las clases |
| `main.cpp` | Visto | Arranque, argumentos, bucle de secciones, globales |
| `CGame.cpp` | Visto, **modificado** | Ajustes y progreso de la partida (`Game`) |
| `CPak.cpp` | Visto | Lectura del archivo `.pak` (`Pak`) |
| `init.cpp` | Visto, **modificado** | Arranque: carga y guardado de `config`, SDL, fuentes, licencia, cierre |
| `options.cpp` | Visto, **modificado** | Pantallas de opciones, teclado, joystick, trucos y gameplay |
| `data/optionWidgets` | Visto, **modificado** | Widgets del menú de opciones |
| `data/gameplayWidgets` | **Nuevo** | Widgets del submenú Gameplay |
| `enemies.cpp` | Visto, **modificado** | Enemigos: definiciones, rangos, escuadras, IA, colisión con balas y muerte |
| `effects.cpp` | Visto | Efectos: sangre, humo, fuego, partículas de efecto |
| `particles.cpp` | Visto | Creación, movimiento, colisión y dibujo de partículas |
| `particles.h` | Visto | Dependencias globales de partículas: `Audio`, `Engine`, `Graphics`, `Map` y `player` |
| `CGameData.cpp` | Visto | Objetivos completados y porcentaje del juego (`GameData`) |
| `CGame.h` | Visto, **modificado** | Declaración de `Game` y sus variables |
| `CConfig.cpp` | Visto | Controles: teclado, joystick, pausa y su guardado (`Config`) |
| `CGraphics.cpp` | Visto | Pantalla, sprites, fuentes, dibujo (`Graphics`) |
| `CGraphics.h` | Visto | Declaración de `Graphics`: superficies públicas, tiles, colores y todas las primitivas |
| `CWidget.cpp` | Visto | Clase `Widget` (elementos de menú) |
| `widgets.cpp` | Visto | Dibujo de los widgets de menú |
| `CCollision.cpp` | Visto | Colisiones entre rectángulos, entidades e interruptores (`Collision`) |
| `CMap.cpp` | Visto | Implementación de `Map`: límites y clasificación de tiles sólidos, líquidos y demás |
| `CMap.h` | Visto | Declaración de `Map`, datos del mapa, listas y consultas de tiles |
| `CParticle.h` | Visto | Clase `Particle`: posición, movimiento, sprite, duración, color y flags |
| `CSpawnPoint.cpp` | Visto | Puntos de aparición (datos y temporizador) |
| `spawnPoints.cpp` | Visto, **modificado** | Qué aparece en cada punto, peligros del mapa y temblor de cámara |
| `spawnPoints.h` | Visto | Cabecera de `spawnPoints.cpp`: solo `extern` |
| `map.cpp` | Visto | Mapa, tiles, minimapa, agua, viento |
| `traps.cpp` | Visto, **modificado** | Trampas (minas, pinchos, péndulos, barreras, llamas) |
| `traps.h` | Visto | Cabecera de `traps.cpp`: solo `extern` de dependencias |
| `triggers.cpp` | Visto | Activación de objetos por nombre |
| `weapons.cpp` | Visto, **modificado** | Definición y selección de armas |
| `CWeapon.cpp` | Visto | Clase `Weapon`: valores por defecto y velocidad del disparo |
| `CEntity.cpp` | Visto | Clase `Entity`: valores por defecto, tamaño desde el sprite, animación, gravedad y entorno |
| `CSprite.h` | Visto | Declaración de `Sprite`: hasta 8 fotogramas con su duración |
| `CSprite.cpp` | Visto | Clase `Sprite`: fotogramas, animación y liberación |
| `items.cpp` | Visto | Ítems: soltar, recoger, cargar `defItems` |
| `resources.cpp` | Visto | Carga de recursos de la misión: sprites, sonidos, armas, enemigos, ítems y mapa |
| `objectives.cpp` | Visto | Objetivos de misión, MIAs y avisos de progreso |
| `player.cpp` | Visto, **modificado** | Bob: movimiento, puntería, disparo, munición, puntaje, medallas |
| `bullets.cpp` | Visto, **modificado** | Balas, granadas, puntería con mouse, mira, trayectoria, estela |
| `explosions.cpp` | Visto, **modificado** | Explosiones, daño, temblor de pantalla, aviso a las minas |
| `CEngine.cpp` | Visto, **modificado** | Motor: entrada, cámara, widgets, datos del pak, tiempos |
| `game.cpp` | Visto | Bucle de la misión, fin de misión, menú de pausa, pantalla de Game Over |
| `Makefile.windows` | Visto, **modificado** | Compilación en Windows, dependencias de cabeceras, pak |
| `compilar.bat`, `clean_and_compile.bat` | Visto / **Nuevo** | Scripts de compilación (normal y limpia) |
| `defs.h` | Visto | Constantes y macros globales: límites, secciones, misión, mapa, vista, pak, `debug` |
| `game.h` | Visto | Cabecera de `game.cpp`: solo `extern` |
| `entities.cpp` | Visto | Movimiento de entidades (gravedad, colisión, teletransporte) |
| `pak.cpp` | Visto | Herramienta que arma `blobwars.pak` (`pak.exe`) |
| `data/inGameWidgets` | Visto | Widgets del menú de pausa (`showInGameOptions`) |
| `data/gameOverWidgets` | Visto | Widgets de pantalla de Game Over (`gameover`) |

---

## Estructura del proyecto

Raíz: `C:\Users\Usuario\Desktop\Proyectos\blockwars\`

| Carpeta o archivo | Contenido | Estado |
|---|---|---|
| `src\` | Fuentes `.cpp` y `.h` (ver el índice) | Confirmada |
| `data\` | Archivos de datos del juego; ahí está `optionWidgets` | Confirmada |
| `gfx\`, `sound\` | Imágenes y sonidos; el código los pide como `gfx/main/...` y `sound/...` | Por confirmar si están sueltas o solo dentro del pak |
| `blobwars.pak` | Empaquetado de `data`, `gfx`, `sound` y `music` (índice al final, archivos con zlib; ver `CPak.cpp`) que se usa cuando el juego se compila con `USEPAK` (por defecto `USEPAK ?= 1`). Lo arma `pak.exe` con `make buildpak` | Se genera en la raíz (donde está el Makefile); falta ver `pak.cpp` |
| `Makefile.windows` | Reglas de compilación (ver su entrada) | Visto, modificado |
| `compilar.bat`, `clean_and_compile.bat` | Scripts que llaman a `make` con MSYS2 en el `PATH` | Visto / Nuevo |

**Archivos de `data/` que el código pide por nombre**

| Archivo | Lo lee | Estado |
|---|---|---|
| `data/optionWidgets` | `showOptions` | Visto, modificado |
| `data/gameplayWidgets` | `showGameplayConfig` | Nuevo |
| `data/cheatWidgets` | `showCheatConfig` | Visto |
| `data/keyboardWidgets` | `showKeyConfig` | Visto |
| `data/joystickWidgets` | `showJoystickConfig` | Visto |
| `data/inGameWidgets` | `showInGameOptions` (menú de pausa) | Visto |
| `data/gameOverWidgets` | `gameover` | Visto |
| `data/weapons` | `loadDefWeapons` | Visto |
| `data/defEnemies` | `loadDefEnemies` | Visto |
| `data/defItems` | `loadDefItems` (`items.cpp`) | Visto |
| `data/defines.h` | `Engine::loadDefines` | Visto |
| `data/license` | `checkForLicense` | Visto |
| `data/vera.ttf` | `initSystem` (fuentes) | Confirmado en `data/` |
| `data/mainSprites` | `loadResources` (`resources.cpp`); define todos los sprites del juego, incluidos los íconos de ítems y las balas | Visto |

**Archivos que el juego escribe** (en `engine.userHomeDirectory`): `config` (texto), `keyboard.cfg` y `joystick.cfg` (binarios), y temporales `music.mod` y `font.ttf`. En Unix la carpeta es `~/.parallelrealities/blobwars/`; en Windows no se vio dónde queda.


---

## Archivos vistos

### `headers.h`
**Función:** cabecera maestra. Casi todos los `.cpp` la incluyen (directo o por su propia cabecera), así que sirve de mapa del proyecto.

- **Librerías:** `errno`, `stdio`, `stdlib`, `string`, `math` y `zlib`; SDL2, SDL2_image, SDL2_mixer, SDL2_ttf y SDL2_net (en macOS con framework, las de SDL1). Por eso `memcpy`, `sqrtf` y `SDL_GetTicks` están disponibles en cualquier archivo.
- **Compatibilidad:** `_()` y `gettext` (traducciones; en Windows y macOS son un simple paso), y `strlcpy`/`strlcat` definidos a mano donde el sistema no los trae.
- **Orden de inclusión (cada cabecera define una clase o grupo):**
  - Base: `defs.h` (constantes), `CMath.h`, `CGameObject.h`, `CList.h`.
  - Objetos del juego: `CSprite`, `CData`, `CParticle`, `CWeapon`, `CEntity`, `CBoss`, `CTrain`, `CSpawnPoint`, `CSwitch`, `CEffect`, `CObjective`, `CTeleporter`, `CLineDef`, `CTrap`.
  - Persistencia y mapa: `CPersistData`, `CPersistant`, `CMap`, `CCollision`.
  - Menús y archivos: `CWidget`, `CFileData`, `CPak`.
  - Entrada: `CJoystick`, `CKeyboard`.
  - Motor: `CEngine`, `CGraphics`, `CAudio`.
  - Partida: `CGame`, `CGameData`, `CHub`, `CRadarBlip`, `CCutscene`.
  - Cierre: `CReplayDataHeader`, `CReplayData`, **`CConfig`**, `CMedalServer`.
- **Para las opciones nuevas:** las variables nuevas ya están en `CGame.h`. `CConfig.cpp` guarda controles; el guardado de ajustes del juego está en `init.cpp`.
- **Guardas:** ninguna de estas cabeceras usa guardas visibles desde acá; `headers.h` tampoco las tiene.

### `main.cpp`
**Función:** punto de entrada. Crea los objetos globales, procesa los argumentos y corre el bucle principal por secciones.

- **Globales definidos aquí:** `audio`, `config`, `engine`, `game`, `gameData`, `graphics`, `map`, `replayData`, `medalServer`, `defEnemy[]`, `defItem[]`, `player` (un `Entity`), `weapon[MAX_WEAPONS]`.
- **Argumentos:** `-fullscreen`, `-window`, `-noaudio`, `-mono`, `-map`, `-listmaps`, `-record`, `-playback`, `-credits`, `-version`, `--help`. En debug: `-showsprites`, `-hub`, `-randomscreens`, `-nomonsters`.
- **Bucle principal:** `switch` sobre `SECTION_INTRO / TITLE / HUB / GAME / GAMEOVER / EASYOVER / CREDITS`. Cada sección devuelve la siguiente.
- **Replays:** graban y reproducen partidas usando una semilla (`Math::pSeed = replayData.header.randomSeed`).
- **Dificultad 3:** da `hasAquaLung` y `hasJetPack` desde el inicio.
- **Replays:** dependen de entradas y semilla. La puntería con mouse usa `engine.mouseLeft` directamente y no pasa por `config.isControl`.
- `initConfig()` se llama desde aquí y está definida en `init.cpp`.
- **`SECTION_GAME`:** si no se viene de un checkpoint (`continueFromCheckPoint`), llama `checkStartCutscene()` y `loadResources()` (definida en `resources.cpp`) antes de `doGame()`. **`SECTION_EASYOVER`:** `map.clear()`, `game.clear()`, `easyGameFinished()` y vuelve a `SECTION_TITLE`. `doGame` y `gameover` están en `game.cpp`.
- **Sin `default`:** el `switch` del bucle principal no tiene `default`.

### `CGame.cpp` — modificado
**Función:** clase `Game`: ajustes del jugador y progreso de la partida.

- **Ajustes (constructor):** `musicVol=100`, `soundVol=128`, `output=2`, `brightness=10`, `gore=1`, `skill=1`.
- **`clear()`:** reinicia progreso, estadísticas, combos, `autoSave=1`, mapa por defecto `data/grasslands1`.
- **Tiempo de misión:** `incrementMissionTime`, `totalUpStats`.
- **Checkpoints:** `setCheckPoint`, `getCheckPoint`, `setObjectiveCheckPoint`, `useObjectiveCheckPoint` (`canContinue = 3`).
- **Combos:** `doCombo` (ventana `lastComboTime = 25`, máximo 99).
- **Puntería:** `bulletsFired[5]` y `bulletsHit[5]` por arma, con `getWeaponAccuracy`, `getTotalAccuracy`, `getMostUsedWeapon`.
- **Fin de misión:** `setMissionOver(reason)` con duraciones en múltiplos de `MAX_FPS` según el motivo (`MIS_COMPLETE`, `PLAYEROUT`, `TIMEUP`, `PLAYERDEAD`, `PLAYERESCAPE`, `GAMECOMPLETE`).
- **`setMissionOver` en detalle:** el motivo (`missionOverReason`) siempre se pisa, pero la cuenta regresiva (`missionOver`) solo se fija si valía 0 o si el motivo es `MIS_TIMEUP`. Duraciones (`MAX_FPS` vale 62 en `defs.h`; `PLAYERDEAD` = 310 fotogramas, unos 5 s): `COMPLETE` 1 x `MAX_FPS`, `PLAYEROUT` 2,5 x, `TIMEUP` y `PLAYERDEAD` 5 x, `PLAYERESCAPE` 2 x, `GAMECOMPLETE` 8 x, cualquier otro 1 x. `resetMissionOver()` deja `missionOver` y `missionOverReason` en 0 (lo llama `doGame` al empezar).
- **Sin inicializar:** ni el constructor ni `clear()` tocan `missionOver` ni `missionOverReason`; dependen de que `game` sea global (memoria en cero) y de que `MIS_INPROGRESS` valga 0 (confirmado en `defs.h`: `MIS_INPROGRESS` es 0).
- **`clear()` no toca los ajustes:** `musicVol`, `soundVol`, `output`, `brightness`, `gore` y `skill` solo se fijan en el constructor.
- **Opciones de jugabilidad:** `mouseAim`, `bulletTrail`, `grenadePreview` (0/1), `cameraLead` y `screenShake` (0 a 3), con valores por defecto 1, 1, 1, 2 y 2 en el constructor.
- **Estadísticas por arma (mejora 2, entregada, sin compilar):** `clear`, `getTotalBulletsFired` y `getTotalAccuracy` recorren `MAX_WEAPONS` posiciones en vez de 5. `incBulletsFired`, `incBulletsHit` y `getWeaponAccuracy` comprueban el índice. `getMostUsedWeapon` sigue mirando solo las armas 0 a 4 a propósito, para que los llamadores que indexan tablas propias con el resultado (no vistos) sigan recibiendo un valor en ese rango. 263 líneas, CRLF.

### `CGame.h` — modificado
**Función:** declaración de `Game`. Sin guardas de inclusión.

- **Ajustes guardables (públicos, `int`):** `gore`, `skill`, `soundVol`, `musicVol`, `output`, `brightness`, `autoSaveSlot`, `autoSave`, en una sola línea.
- **Progreso y estadísticas:** puntaje, tiempos, enemigos, ítems, objetivos, MIAs, arma actual, `bulletsFired[5]`, `bulletsHit[5]`, checkpoints, `hasAquaLung`, `hasJetPack`, combos, `missionOver*`, `continuesUsed`, `levelsStarted`, `escapes`, `canContinue`.
- **Privado:** solo `objectiveCheckPointX/Y`.
- **Agregado:** línea con `mouseAim`, `bulletTrail`, `grenadePreview`, `cameraLead` y `screenShake`, debajo de los ajustes existentes.
- **Estado en el proyecto:** `bullets.cpp` usa `game.mouseAim`, `game.bulletTrail`, `game.grenadePreview` y `game.cameraLead`.
- **Estadísticas:** `bulletsFired` y `bulletsHit` pasaron de 5 a `MAX_WEAPONS` (25) posiciones, para que las armas 21 y 22 no escriban fuera del arreglo (mejora 2, entregada, sin compilar). `currentWeapon` es `unsigned char`. El objeto `Game` crece 160 bytes; si `saveGame` o `loadGame` (no vistos) escriben la estructura o los arreglos con `sizeof`, el formato de las partidas guardadas cambia.

### `enemies.cpp`
**Función:** enemigos: definiciones, aparición con rangos y escuadras, IA (alerta, disparo, movimiento), colisión con balas, muerte y dibujo. El adjunto revisado el 2026-09-29 tiene 2094 líneas. Incluye un sistema de IA propio (alerta, rangos, escuadras, ráfagas), no solo la lógica base.

**Estado por enemigo**
- `EnemyAIState` en `std::map<Entity*, EnemyAIState>` dentro del `.cpp`: aviso de disparo (`telegraph`), `fireNow`, conciencia (`awareness`), `lostSight`, `alerted`, `canSee`, última posición vista, detección de atasco, `maxHealth`, ráfaga, turno de ataque, `rank`, `squadLeader`, `confused` y `panicDir`.
- `forgetEnemy` borra el estado de un enemigo (y a quienes lo seguían); se llama al crear un `Entity` nuevo (por si el puntero se reutiliza) y al quitarlo de la lista.

**Alerta (`senseSurroundings`, cada fotograma por enemigo libre)**
- Solo percibe si el jugador está vivo, la misión no terminó, el enemigo está en pantalla (margen 32 px), a menos de 720 px en x y 100 en y, y hay línea de tiro (`hasClearShot`).
- La conciencia sube más rápido cuanto más cerca y a mayor `skill` (`(0.6 + 2.4 * cercanía) * (0.7 + 0.2 * skill)`); de espaldas, a la mitad y solo si está a menos de 96 px o ya sospecha.
- 30 = "?" (va al último punto visto) y 100 = "!" (ataca). Sin ver al jugador retiene 120 fotogramas (sospecha) o 360 (alerta) y luego baja 0,16 por fotograma. `ENT_ALWAYSCHASE` siempre sabe dónde está.
- Al llegar a 100 avisa a los cercanos (300 x 150 px, nivel 65; el sargento grita 1,5 veces más lejos) y, si es sargento, alerta a toda su escuadra.
- Oído: si `player.reload` sube (Bob disparó), alerta a los enemigos a 420 + 40 * `skill` px en x y 220 en y, con nivel 45, incluso a través de paredes.

**Rangos y escuadras (`assignRank`, solo en `addEnemy`)**
- Solo enemigos que caminan con salud base de 3 o menos (se excluyen los que vuelan, nadan, estáticos, jefes, Galdov, inanimados, inmunes) y fuera de misiones de jefe.
- Veterano: salud base + 2, valor x2, probabilidad `10 + 3 * stagesCleared + 4 * skill` (tope 45 %). Sargento: salud base + 4, valor x4, probabilidad `6 + 2 * (stagesCleared - 2) + 2 * skill` (tope 25 %), nula con menos de 2 niveles superados, máximo 5 vivos, y los enemigos que aparecen por `ENT_SPAWNED` nunca lo son.
- Un sargento trae 2 escoltas (3 con `skill` >= 2) a 40, 72 o 104 px a cada lado, elegidos entre `Pistol Blob` y `Machine Gun Blob` (esos nombres deben existir en `defEnemies`). Los escoltas no reciben rango.
- Al morir el sargento, la escuadra queda confundida 150 fotogramas y se mueve al azar.

**Disparo (`lookForPlayer`)**
- **Granadas con carga (2026-09-30):** `isGrenadeWeapon` (`WP_GRENADES` y `WP_ALIENGRENADE`). Un líder solo empieza el ataque si `isGrenadeSafe` y `planEnemyGrenade` (de `bullets.cpp`) dicen que puede alcanzar a Bob. El aviso dura `getWindupFrames` + `power * GRENADE_CHARGE_FRAMES` (45): la sujeta más tiempo cuanto más lejos o más arriba esté Bob, y al soltar lanza con `addEnemyGrenade`. Si Bob se fue fuera de alcance durante la carga, no lanza y espera 30 fotogramas. Estado nuevo en `EnemyAIState`: `telegraphTotal` y `charging`; `drawAwareness` dibuja una barra naranja de carga sobre el enemigo. Los seguidores lanzan al instante, sin carga. Sin compilar ni probar.
- Los líderes esperan un turno de ataque: máximo `1 + skill` atacando a la vez y una pausa de `20 - 4 * skill` fotogramas entre inicios. Avisan antes de disparar (`30 - 4 * skill - 4 * rango` fotogramas, mínimo 10) y luego disparan.
- Ráfagas: 2 disparos (3 con 50 % si `skill` >= 2, +1 en veteranos), limitadas a `4 / daño`; 1 solo si el arma recarga más de 25 o explota; el arma de dispersión, máximo 2. Separación mínima de 6 fotogramas y 50 de pausa al terminar.
- `getAimDY`: puntería con anticipación (`skill / 3`) solo con armas rectas sin `dy` propio y enemigos sin `ENT_AIMS`; tope de ±3.
- Bob no participa: los enemigos disparan con `addBullet(enemy, velocidad, aimDY)`.

**Movimiento (`doAI`)**
- Los jefes salen sin IA; Galdov llama a `doGaldovAI` y sigue. Detecta atascos: a los 20 fotogramas salta, a los 60 abandona el destino. Sin sospecha, deambula hasta 640 px (o cerca de su líder). Voladores y nadadores mueven `dy` hacia `ty`. Quieto durante aviso y ráfaga.

**Colisión y muerte**
- `enemyBulletCollisions`: balas de Bob, del mundo o de jefes. Alerta al golpeado, sus vecinos y su escuadra; cuenta aciertos, combos (medalla `25_Hit_Combo`), puntaje y objetivos (`Combo-<arma>`, `Enemy`, nombre). Con `game.gore` sale sangre; si no, partículas de color por tipo (`getNonGoreParticleColor`).
- `gibEnemy`: `ENT_EXPLODES` explota con radio `10 + 20 * skill`; el resto suelta 25 efectos de sangre con gore o 150 partículas de color sin gore.
- `doEnemies`: solo actualiza dentro de `ACTIVE_W/H`, dibuja dentro de `DRAW_W/H` (con iconos de alerta, barra de salud y galones). Los `ENT_SPAWNED` a más de 1920 x 1440 px se eliminan. Enemigos en limo o lava mueren. Los muertos pasan por una cuenta regresiva de salud hasta -50 y solo se quitan si `referenced` es falso (con `cheatBlood` no se quitan).
- **Drops:** cuando la salud llega a -50 (y tiene `value`), `doEnemies` llama a `dropRandomItems` dentro de `ACTIVE_W/H`, en dos ramas (muerte directa y muerto sin referencias). El drop no mira el rango del enemigo.
- Dibujo: barra de salud solo si hay daño o rango; galones amarillos (veterano) o naranjas dobles (sargento); "?" amarillo y "!" rojo (amarillo parpadeante durante el aviso).

**Carga:** `loadDefEnemies` lee `data/defEnemies` (una línea por enemigo: `"nombre" sprite0 sprite1 sprite2 "arma" salud valor flags`, termina con `@EOF@`). `loadEnemy` hace lo mismo con una línea. `getDefinedEnemy` y `getEnemy` buscan por nombre.

**A verificar (no se probó nada)**
- En el adjunto revisado el 2026-09-29, `hasClearShot` ya traza desde el centro del enemigo al centro de Bob, avanza tile por tile y termina al alcanzar la caja de Bob. La observación anterior sobre el rayo desde las esquinas y el ancho/alto invertidos no aplica a esta versión adjunta; falta probar el comportamiento en el juego.
- En `enemyBulletCollisions`, la rama de `ENT_IMMUNE` compara con `enemy->tx` pero asigna `enemy->owner->tx`.
- `loadDefEnemies` y `loadEnemy` no comprueban `MAX_ENEMIES` al leer, y sin `@EOF@` en `defEnemies` el `strcmp` recibiría un puntero nulo (mismo caso que `weapons`).
- No tiene relación con las opciones nuevas: usa `game.skill`, `game.gore`, `game.stagesCleared` y los combos.

### `CPak.cpp`
**Función:** clase `Pak`: abre `blobwars.pak`, lee su índice y descomprime archivos por nombre. Solo actúa si el juego se compila con `USEPAK`.

- **Formato del pak (según el lector):** los últimos 8 bytes son dos `Uint32`: posición del índice (`listPos`) y cantidad de archivos (`numberOfFiles`). En `listPos` hay `numberOfFiles` registros `FileData` escritos como `struct` crudo (`fread` de `sizeof(FileData)`). Cada archivo está comprimido con zlib (`uncompress`) en `location`, con tamaño comprimido `cSize` y descomprimido `fSize`.
- **Nombres:** `unpack(nombre, &buffer)` compara con `strcmp` contra `fd[i].filename`, sin normalizar: la clave debe ser exactamente la ruta que pide el código (`data/optionWidgets`, `data/gameplayWidgets`, `gfx/main/...`, `sound/...`). Si no la encuentra devuelve `false`; el caller decide qué hacer (`loadWidgets` devuelve `false` y `showOptions` cierra con `showErrorAndExit(ERR_FILE, ...)`).
- **Otras funciones:** `setPakFile` (lee el índice; si falta el pak, `showPakErrorAndExit` termina el programa con `PAKFULLPATH`), `fileExists`, `getUncompressedSize`. El buffer de salida es `fSize + 1` con un 0 al final.
- **Depuración:** en `unpack` hay tres `fprintf(stderr, "DEBUG ...")` (entradas que contienen `mainSprites`, coincidencia encontrada y resultado de un `fread` previo).
- **`uncompress`:** el resultado no se comprueba.
- **Para agregar `data/gameplayWidgets`:** el packer debe escribir un registro `FileData` con ese nombre exacto y actualizar `listPos` y `numberOfFiles` en los últimos 8 bytes.

### `init.cpp` — modificado
**Función:** arranque y cierre del juego: configuración, SDL, ventana, fuentes, licencia, servicio de medallas.

- **`initConfig()`:** en Unix crea `~/.parallelrealities/blobwars/` y llama a `loadConfig()`; su resultado decide si se muestra la licencia (`displayLicense`).
- **`loadConfig()` / `saveConfig()`:** archivo de **texto** `config` dentro de `engine.userHomeDirectory`. Línea 1: `VERSION RELEASE`. Línea 2: `fullScreen musicVol soundVol output brightness extremeAvailable gore`. Si falta el archivo, devuelve `true` (muestra la licencia); si la línea 2 no tiene 7 enteros, también. Después llama a `config.loadKeyConfig()` y `config.loadJoystickConfig()`.
- **Línea 3:** `mouseAim bulletTrail grenadePreview cameraLead screenShake`. Se lee con enteros temporales y solo se aplica si están los 5; luego se limita a 0-1 (los tres primeros) y 0-3 (los dos últimos).
- **`initSystem()`:** superficie de 1280 x 720, ventana redimensionable (se duplica en pantallas grandes), gamma según `game.brightness`, audio, joystick, fuentes de 18 a 36, medallas, licencia y `SDLNet`. Al final pone `engine.saveConfig = true`.
- **`SDL_CreateThread`:** si falla, `initSystem()` hace `return` antes de `engine.saveConfig = true`, y `cleanup()` no guarda el `config`.
- **`cleanup()`:** libera todo; guarda con `saveConfig()` solo si `engine.saveConfig` es `true`.
- **Fin de línea:** CRLF.

### `options.cpp` — modificado
**Función:** pantallas de opciones: `showOptions`, `showKeyConfig`, `showJoystickConfig`, `showCheatConfig` y, agregada, `showGameplayConfig`. Fin de línea LF (no CRLF).

- **Archivos de widgets:** `data/optionWidgets`, `keyboardWidgets`, `joystickWidgets`, `cheatWidgets` y, nuevo, `gameplayWidgets`. Cada submenú recarga `data/optionWidgets` al salir y resalta su botón.
- **Widgets de `optionWidgets` enlazados en `showOptions`:** `fullscreen`, `soundvol`, `musicvol`, `output`, `autosave`, `gamma`, `gore`, `keys`, `joysticks`, `cheats`, `gameplay`, `confirm`. `gore` es el molde para las opciones nuevas.
- **Duplicación:** la lista de `setWidgetVariable` aparece dos veces, al abrir y tras volver de un submenú.
- **`autosave` sin widget:** `showOptions` enlaza `"autosave"` con `game.autoSave`, pero `optionWidgets` no lo define. `setWidgetVariable` ignora los nombres inexistentes. `highlightWidget("gameplay")` con un `optionWidgets` viejo sí cierra el juego (ver `CEngine.cpp`).
- **Reacciones al cambio:** `widgetChanged` de `soundvol`, `musicvol`, `fullscreen` y `gamma` aplica el cambio en el momento.
- **Agregado — submenú Gameplay:** `showGameplayConfig()` carga `data/gameplayWidgets`, enlaza `mouseaim`, `trail`, `preview`, `camlead` y `shake` con `game.mouseAim`, `bulletTrail`, `grenadePreview`, `cameraLead` y `screenShake`, y al salir recarga `optionWidgets` y resalta `gameplay`. Usa los mismos fondo y cabecera (`optionsBackground`, `optionsHeader`). En `showOptions` se agregó la variable `gameplay`, el botón se enlaza en las dos listas y se llama con el mismo esquema que `keys`, `joysticks` y `cheats`.

### `data/optionWidgets` y `data/gameplayWidgets`
**Formato:** una línea por widget, `TIPO nombre grupo "etiqueta" "opciones" x y min max`; termina con `END`. Fin de línea LF. Filas separadas por 35 px, con las etiquetas en `x = 100`.

- **`optionWidgets` original:** `fullscreen`, `soundvol`, `musicvol`, `output`, `gamma`, `gore`, `keys`, `joysticks`, `cheats`, `confirm`, con `y` de 65 a 385.
- **Cambio:** botón nuevo `gameplay` ("Gameplay...") en `y = 275`; `keys` baja a 310, `joysticks` a 345, `cheats` a 380 y `confirm` a 420.
- **`gameplayWidgets` (nuevo):** cinco `RADIO` de grupo `gameplay` (`mouseaim`, `trail`, `preview` con `Off|On`; `camlead` y `shake` con `Off|Low|Mid|High`, valores 0 a 3) y `confirm` en `y = 260`.
- **Pak:** con `USEPAK` el juego lee estos archivos solo del pak, así que hay que regenerarlo; sin `USEPAK` los lee sueltos desde `data/`.

### `data/cheatWidgets`
**Función:** widgets del submenú de trucos (`showCheatConfig`). Fin de línea LF, mismo formato que `optionWidgets`.

- **Nueve `RADIO`** de grupo `cheats`, `Off|On` (0 a 1), etiquetas en `x = 100` y filas cada 35 px desde `y = 75` hasta `y = 355`: `health` (Unlimited Health), `extras` (Always have Aqua Lung / Jetpack), `fuel` (Unlimited Fuel), `rate` (Rapid Firing Rate), `blood` (Extra Blood / Gore), `invulnerable` (etiqueta "Spin Control"), `speed` (Increased Speed), `levels` (Access All Levels) y `skip` (Auto Complete Level (F3)).
- **Botón `confirm`** (grupo `options`, etiqueta "Exit") en `y = 420`, igual que en `optionWidgets`.
- **Cada `RADIO` necesita su `setWidgetVariable` en `showCheatConfig`; sin variable enlazada, izquierda/derecha desreferencia un puntero nulo (ver `CEngine.cpp`).

### `data/keyboardWidgets`
**Función:** widgets del submenú de teclado (`showKeyConfig`). Fin de línea LF, mismo formato que `optionWidgets`.

- **Ocho `KEYBOARD`** de grupo `keyboard`, etiquetas en `x = 100` y filas cada 30 px desde `y = 75` hasta `y = 285`: `left`, `right`, `jump`, `down`, `fire`, `jetpack` (Activate Jetpack), `pause` y `map` (Radar Map). No hay widget `up`.
- **Botones:** `defaults` (Restore Defaults) en `y = 325` y `confirm` (Exit) en `y = 365`, ambos de grupo `keyboard`, con `min = 0` y `max = 100`.
- **Dos `LABEL`** centrados (`x = -1`) en `y = 420` y `y = 450`: nota de que los menús se controlan con las flechas y Enter, Return o Espacio.
- **Sin control de recarga ni de puntería:** el clic derecho de la recarga y el disparo con mouse no pasan por estos widgets.

### `data/joystickWidgets`
**Función:** widgets del submenú de joystick (`showJoystickConfig`). Fin de línea LF.

- **Nueve `JOYPAD`** de grupo `joystick`, etiquetas en `x = 120` y filas cada 35 px desde `y = 65` hasta `y = 345`: `left`, `right`, `up`, `down`, `fire`, `jump`, `jetpack`, `pause` y `map`. A diferencia del teclado, aquí sí hay `up`.
- **`SMOOTH_SLIDER`** `sensitivity` en `y = 380`, con `min = 1` y `max = 320`.
- **Botón `confirm`** (Exit) en `y = 420`.

### `data/defEnemies`
**Función:** definiciones de enemigos que lee `loadDefEnemies`. Fin de línea LF, 13 enemigos separados en grupos por líneas vacías, termina con `@EOF@`.

**Formato:** `"nombre" sprite0 sprite1 sprite2 "arma" salud valor flags`. Los sprites siguen el patrón `<Nombre>Right`, `<Nombre>Left` y `<Nombre>Death` (`<Nombre>Spin` en las arañas).

| Enemigo | Arma | Salud | Valor | Flags |
|---|---|---|---|---|
| Pistol Blob | Aimed Pistol | 2 | 50 | `ENT_AIMS` |
| Grenade Blob | Alien Grenades | 3 | 100 | `ENT_AIMS` |
| Aqua Blob | Aimed Pistol | 2 | 50 | `ENT_AIMS+ENT_SWIMS` |
| Laser Blob | Alien Laser Cannon | 3 | 50 | 0 |
| Machine Gun Blob | Machine Gun | 3 | 50 | `ENT_RAPIDFIRE` |
| Eye Droid V1.0 | Aimed Pistol | 4 | 50 | `ENT_AIMS+ENT_FLIES+ENT_EXPLODES` |
| Eye Droid V2.0 | Machine Gun | 5 | 50 | `ENT_MULTIEXPLODE+ENT_FLIES+ENT_EXPLODES+ENT_RAPIDFIRE` |
| Eye Droid V3.0 | Alien Grenades | 5 | 50 | `ENT_AIMS+ENT_FLIES+ENT_EXPLODES` |
| Eye Droid V4.0 | Rocket Launcher | 6 | 50 | `ENT_AIMS+ENT_FLIES+ENT_EXPLODES+ENT_MULTIEXPLODE` |
| Spider Blob | Aimed Spread Gun | 15 | 50 | `ENT_MULTIEXPLODE+ENT_AIMS+ENT_EXPLODES+ENT_JUMPS` |
| Spider Blob V2.0 | Alien Grenades | 15 | 50 | `ENT_MULTIEXPLODE+ENT_AIMS+ENT_EXPLODES+ENT_JUMPS` |
| Spider Blob V3.0 | Alien Laser Cannon | 15 | 50 | `ENT_MULTIEXPLODE+ENT_EXPLODES+ENT_JUMPS` |
| Spider Blob V4.0 | Machine Gun | 15 | 50 | `ENT_MULTIEXPLODE+ENT_EXPLODES+ENT_JUMPS+ENT_RAPIDFIRE` |

- **Coincidencias con `enemies.cpp`:** `Pistol Blob` y `Machine Gun Blob` (los escoltas del sargento) están definidos, y el archivo termina con `@EOF@`.
- **Armas:** los seis nombres usados (`Aimed Pistol`, `Alien Grenades`, `Alien Laser Cannon`, `Machine Gun`, `Rocket Launcher`, `Aimed Spread Gun`) existen en `data/weapons`.

### `data/defItems`
**Función:** definiciones de ítems (`main.cpp` declara `defItem[]`). Fin de línea LF, 15 entradas (ids 0 a 14), termina con `@EOF@`.

**Formato observado:** `id "nombre" sprite número`.

- **Ids 0 a 4:** armas (`Pistol`, `Machine Gun`, `Laser Gun`, `set of Grenades`, `Three Way Spread`), sprites `PistolIcon`, `MachineIcon`, `LaserIcon`, `GrenadeIcon` y `SpreadIcon`, número 1.
- **Ids 5 a 7:** cerezas (`Cherry`, `pair of Cherries`, `bunch of Cherries`), sprites `Cherry`, `DoubleCherries` y `TripleCherries`, números 1, 2 y 5.
- **Ids 8 a 14:** puntos (`Points` a `Points7`), sprites `PointsPod` a `PointsPod7`, números 25, 50, 75, 100, 125, 150 y 200.

### `data/license`
**Función:** texto de la licencia que muestra el juego (`checkForLicense`). Fin de línea LF.

- **Formato:** cada línea es `número texto`; las líneas vacías no llevan número. El número sube de 20 en 20 (de 50 a 360) contando también las líneas vacías, y la última línea es `-1 END`.
- **Contenido:** el título "Blob Wars : Metal Blob Solid" y el aviso de la licencia GPL versión 2 (20 líneas).

### `data/defines.h`
**Función:** tabla de constantes con nombre que lee `Engine::loadDefines` (consultas con `getValueOfDefine` y `getDefineOfValue`; `getValueOfFlagTokens` suma los flags separados por `+` de los archivos de datos). Fin de línea LF, 87 `#define` agrupados por comentarios `/* ... */`, que el lector ignora. Con `USEPAK` va dentro del pak, como el resto de `data/`.

- **Estados y objetivos:** `INACTIVE` 0 y `ACTIVE` 1; `MIA_NORMAL` 0 y `MIA_AQUA` 1; `OBJ_OPTIONAL` 0 y `OBJ_REQUIRED` 1.
- **Interruptores `SWT_*` (0 a 6):** `NORMAL`, `TOGGLE`, `TIMED`, `PRESSURE`, `RESET`, `WATERLEVEL` y `USED`.
- **Trenes y puertas `TR_*`:** `TR_TRAIN` 0; puertas 1 a 5 (normal, `LOCKED`, `GOLD`, `SILVER`, `BRONZE`); puertas corredizas 6 a 10 con las mismas variantes; posiciones `TR_AT_END` 0 y `TR_AT_START` 1; `TR_DOOR_CLOSED` 0 y `TR_DOOR_OPEN` 1.
- **Puntos de aparición `SPW_*`:** `SPW_HAZARD` 0 con subtipos `HAZARD_LAVABALL` 0, `ROCKFALL` 1, `BOMBS` 2, `EXPLOSION` 3, `POWERBULLETS` 4 y `STALAGTITES` 5; `SPW_ENEMY` 1; `SPW_ITEM` 2; `SPW_BOSSBULLET` 3 con `BOSSBULLET1` a `BOSSBULLET5`; `SPW_NOSUBTYPE` -1.
- **Trampas `TRAP_*`:** acciones `FIRSTACTION` 0, `WAIT1` 1, `SECONDACTION` 2 y `WAIT2` 3; tipos `SPIKE` 0, `MINE` 1, `SWING` 2, `CRUSHER` 3, `BARRIER` 4 y `FLAME` 5.
- **Flags de entidad `ENT_*` (33):** `ENT_NONE` 0, `ENT_INANIMATE` 1 y el resto escritos como `( 2 << n )` con `n` de 0 a 30.

- **Cruce con los archivos de datos:** los 12 flags que usan `defEnemies` y `weapons` (`ENT_AIMS`, `ENT_BOUNCES`, `ENT_EXPLODES`, `ENT_FIRETRAIL`, `ENT_FLIES`, `ENT_JUMPS`, `ENT_MULTIEXPLODE`, `ENT_ONFIRE`, `ENT_PARTICLETRAIL`, `ENT_RAPIDFIRE`, `ENT_SWIMS` y `ENT_WEIGHTLESS`) están definidos acá. Un flag que falte termina el programa salvo que `IGNORE_FLAGTOKEN_ERRORS` esté activo (ver `CEngine.cpp`).
- **`ENT_GALDOVFINAL`:** vale `2 << 30` (2147483648), que no cabe en un `int` con signo.

### `data/weapons` — modificado
**Función:** definiciones de armas que lee `loadDefWeapons`. Fin de línea LF, 22 armas (ids 0 a 7 y 9 a 22; no hay id 8), con líneas vacías entre grupos, termina con `@EOF@`.

**Formato:** `id "nombre" idArma daño saludBala dx dy recarga sprite0 sprite1 sonido flags`. `idArma` se guarda en `weapon[id].id` y de él sale el cargador (`getDefaultClipSize`). En las armas con `ENT_EXPLODES`, `bullets.cpp` usa el `daño` como radio de la explosión.

| Arma | id | `idArma` | Daño | Recarga | Quién la usa |
|---|---|---|---|---|---|
| Pistol | 0 | 0 | 1 | 15 | Bob |
| Machine Gun | 1 | 1 | 1 | 4 | Bob y `Machine Gun Blob`, `Eye Droid V2.0`, `Spider Blob V4.0` |
| Laser Cannon | 2 | 2 | 1 | 40 | Bob |
| Grenades | 3 | 3 | 50 | 20 | Bob |
| Spread Gun | 4 | 4 | 1 | 15 | Bob |
| Rocket Launcher | 5 | 5 | 75 | 45 | `Eye Droid V4.0` |
| Aimed Pistol | 11 | 11 | 1 | 5 | `Pistol Blob`, `Aqua Blob`, `Eye Droid V1.0` |
| Aimed Spread Gun | 12 | 4 | 1 | 15 | `Spider Blob` |
| Alien Laser Cannon | 17 | 2 | 3 | 15 | `Laser Blob`, `Spider Blob V3.0` |
| Alien Grenades | 18 | 18 | 50 | 20 | `Grenade Blob`, `Eye Droid V3.0`, `Spider Blob V2.0` |

- **Otras armas:** Plasma Rifle (6, daño 15), Flame Thrower (7, daño 3), Lava Ball (9 y 10, daño 50), Aimed Machine Gun (19, daño 1), Shells (13, daño 75), Rock (14, daño 25), Stalagtite (15, daño 3), Bomb (16, daño 25) y Mortor Shells (20, daño 50; nombre tal cual figura en el archivo).
- **`idArma` repetido:** `Aimed Spread Gun` (12) tiene `idArma` 4 y `Alien Laser Cannon` (17) tiene `idArma` 2, así que comparten con `Spread Gun` y `Laser Cannon` el cargador por defecto y, en el segundo caso, la comprobación `bullet->id == WP_LASER`.
- **Balas rectas de Bob:** las cuatro armas directas de Bob (Pistol, Machine Gun, Laser Cannon y Spread Gun) hacen 1 de daño por impacto.
- **Armas de enemigos con `dy` distinto de 0:** `Rocket Launcher` (5) y `Plasma Rifle` (6) tienen `dx 8 dy 8` (armas apuntadas). `addBullet` trata todo `dy` distinto de 0 como lanzado (física de granada), así que Bob no puede usarlas tal cual: haría falta una entrada nueva con `dy 0` (ids libres 21 a 24, `MAX_WEAPONS` es 25) que conserve el `idArma` original para el cargador.
- **Flame Thrower (7):** `dx 0`, recarga 0 y sonido -1; con `getSpeed` da velocidad 0. Las trampas de llamas la usan con `engine.world` como dueño.
- **`Aimed Pistol` (11), 2026-09-30:** `dx` y `dy` pasaron de 4 a 5 (entregado, sin probar). Los dos van juntos porque, con `ENT_AIMS`, `addBullet` multiplica la dirección unitaria hacia Bob por `dx` y `dy`; cambiar solo uno deforma la puntería. `Aimed Spread Gun` (12) queda en 4.
- **Granadas (3 y 18), datos del archivo del 2026-09-30:** `Grenades` y `Alien Grenades` tienen `dx 3`, `dy -2`, vida 240, `ENT_BOUNCES+ENT_EXPLODES` y no son `WEIGHTLESS`, así que caen con la gravedad de `doBullets`. Rebotan en paredes y suelo y explotan al acabarse la vida (240 fotogramas, unos 4 s) o al tocar a Bob. Con el disparo original de `ENT_AIMS` (dirección unitaria x `dx`, `dy`) la velocidad es de unos 3,6 como máximo y el alcance en llano ronda los 130 px; por eso las granadas enemigas caían cortas a la distancia preferida de 450 px. `addEnemyGrenade` las reemplaza por un lanzamiento de 5 a 10 de velocidad.
- **Rebote:** `Laser Cannon` (2) y `Alien Laser Cannon` (17) tienen `ENT_BOUNCES`; `Alien Laser Cannon` comparte `idArma` 2. El campo `salud` es la vida de la bala en fotogramas.
- **Fin de línea:** LF; 27 líneas (con las armas 21 y 22); el id 8 (`ICEGUN`) no tiene entrada.
- **Armas de Bob con `dy 0` (nuevas, entregadas, sin probar):** `21 "Player Rocket Launcher"` (`idArma` 5, radio 100 según el archivo subido el 2026-09-30, velocidad 8, recarga 45, vida 240) y `22 "Player Plasma Rifle"` (`idArma` 6, radio 12, velocidad 8, recarga 14, vida 60). Mismos sprites, sonido y flags que las originales. Nada las selecciona todavía: los ítems solo dan las armas 0 a 4. Quedan libres los ids 23 y 24 (`MAX_WEAPONS` es 25). Los nombres deben ser únicos por `getWeaponByName` y por los `Combo-<nombre>` de objetivos; `Lava Ball` (ids 9 y 10) ya está repetido.

### `data/mainSprites`
**Función:** definición de todos los sprites que carga `loadResources`. 166 líneas, LF, ASCII, 125 sprites agrupados por líneas vacías, termina con `@EOF@`.

**Formato:** `Nombre hue sat val archivo1 tiempo1 archivo2 tiempo2 ... @none@ 0` (hasta 8 fotogramas, ver `resources.cpp`). Todas las líneas terminan en `@none@`. Los tiempos son fotogramas por imagen (60 en las estáticas). Las rutas son `gfx/sprites/...`.

- **Reutilización con otro tono:** ya se hace. `AlienLaserBolt` usa `laserBolt1.png` con `hue` 135, `RocketDroid` usa las imágenes de `eyeDroid2` con `hue` 100 y `GrenadeDroid` las mismas con -130; `ItemSignal` y `ItemArrow` reutilizan `miaSignal` y `scannerArrow`.
- **Íconos de ítems:** `PistolIcon`, `MachineIcon`, `GrenadeIcon`, `LaserIcon` y `SpreadIcon`, una imagen cada uno (`pistolIcon.png`, `machineGunIcon.png`, `grenadeIcon.png`, `laserIcon.png`, `spreadIcon.png`). No hay íconos de cohete ni de plasma.
- **Puntos y cerezas:** `PointsPod` a `PointsPod7`, `Cherry`, `DoubleCherries` y `TripleCherries`.
- **Balas y efectos usados por `data/weapons`:** `FlameBulletRight` y `FlameBulletLeft` (el cohete), `PlasmaBolt` (plasma y escopeta), `LaserBolt`, `AlienLaserBolt`, `Grenade`, `AlienGrenade`, `AimedShot`, `Bomb`, `Stalagtite`, `LavaRock` y `Explosion` (en realidad la imagen `onFire`, usada por el lanzallamas).
- **`FlameThrower`:** existe como sprite pero usa `bubble.png`; el arma 7 pide `Explosion`, no este.
- **Otros:** Bob (`BobRight`, `AquaBob*`, `JPBob*`), puertas, llaves, enemigos, MIAs, señales y flechas del radar, `HealthBlock` y `OxygenBlock`.
- **Para agregar un ítem:** una línea nueva en este archivo y regenerar el pak; con `getSprite(nombre, true)` un nombre que falte cierra el juego.

### `effects.cpp`
**Función:** efectos de sangre, humo y fuego, que sueltan partículas mientras viven.

- **`addEffect(x, y, dx, dy, flags)`:** crea un `Effect` y lo agrega al mapa. `addColoredEffect` le suma color y salud al azar de 60 a 90.
- **`addSmokeAndFire(ent, dx, dy, amount)`:** posiciones al azar dentro de la entidad; 3 de cada 4 son fuego (`EFF_TRAILSFIRE`) y el resto humo (`EFF_SMOKES`).
- **`addBlood(ent, dx, dy, amount)`:** solo actúa si `game.gore` es distinto de 0; con `engine.cheatBlood` triplica la cantidad.
- **`doEffects()`:** cada efecto suelta una partícula por fotograma según su tipo (`EFF_BLEEDS`, `TRAILSFIRE`, `SMOKES`, `COLORED`). Se elimina si sale por la izquierda o por arriba de la cámara, o si entra en un tile sólido.
- **Eliminación:** solo comprueba salir por izquierda y arriba (`x < 0`, `y < 0`); por derecha y abajo depende de que `health` llegue a 0 en `update()`.

### `particles.cpp`
**Función:** creación y actualización de partículas. 211 líneas en el adjunto revisado el 2026-09-29.

- **Creación:** partículas de viento, color, fuego, burbujas, fragmentos de estalactita y ladrillo, estelas de fuego y teletransporte.
- **`doParticles()`:** dibuja píxeles o sprites, reduce la vida, mueve las partículas y aplica gravedad salvo a las `PAR_WEIGHTLESS`.
- **Colisión:** `PAR_COLLIDES` las elimina al tocar un tile sólido; `PAR_LIQUID` las elimina al salir de un tile líquido.
- Usa `map.particleList` y `map.addParticle(...)`. Incluye `particles.h` para acceder a los globales del motor.

### `particles.h`
**Función:** expone los globales que necesita `particles.cpp`. 29 líneas en el adjunto revisado el 2026-09-29.

- Incluye `headers.h` y declara como `extern` `audio`, `engine`, `graphics`, `map` y `player`.
- No declara funciones ni tiene guardas de inclusión. Es distinto de `CParticle.h`, que declara la clase `Particle`.

### `CParticle.h`
**Función:** declara `Particle`, la entidad individual de partícula. 40 líneas en el adjunto revisado el 2026-09-29.

- **Estado y movimiento:** `x`, `y`, `dx`, `dy`, `health`, `color` y `flags`.
- **Dibujo animado:** `sprite`, `currentFrame`, `currentTime` y `getFrame()`.
- **Métodos:** constructor, `set(...)`, `setSprite(...)`, `getFrame()` y `move()`.
- El encabezado hereda de `GameObject` y no tiene guardas de inclusión. Es distinto de `particles.h`, que declara los globales utilizados por `particles.cpp`.

### `CGameData.cpp`
**Función:** clase `GameData`, lista de `Data` (clave = nivel, valor = objetivo, `current`, `target`) con lo completado en la partida.

- **Altas:** `addCompletedObjective` (dos versiones, actualiza si ya existe), `setMIARescueCount` (clave `"<nivel> MIAs"`).
- **Consultas:** `MIARescued` (busca `MIA_<nombre>`), `objectiveCompleted`, `getObjectiveValues`, `stagePreviouslyCleared`, `isCompleted`, `levelPrefectlyCleared`, `requiredLevelCleared`.
- **`calculateWorldCompleted()`:** `completedWorld` pasa a `true` si existe algún dato del nivel `BioMech HQ`.
- **`getPercentageComplete()`:** proporción de entradas `Data` completas sobre el total.
- **No guarda nada en disco ni maneja los ajustes:** solo mantiene la lista en memoria.

### `CConfig.cpp`
**Función:** clase `Config`: convierte teclado, joystick y palanca en la lista de órdenes `command[CONTROL::MAX]`, maneja la pausa y guarda/carga los controles. **No guarda ajustes del juego.**

- **`populate()`:** para cada `CONTROL`, `command[i]` = tecla asignada (`engine->keyState`) o botón de joystick (si `joystick.control[i] >= 0`). Los ejes `joyX/joyY` fuera de `joystick.sensitivity` fuerzan `LEFT`, `RIGHT`, `UP` y `DOWN`.
- **`populate(int *data)`:** carga las órdenes desde un replay y fuerza `MAP` y `PAUSE` en 0. Los replays solo llevan los `CONTROL`; el mouse no pasa por acá.
- **`isControl`, `resetControl`:** consulta y borra una orden (tecla, botón y ejes).
- **`doPause()`:** la tecla de pausa alterna `engine->paused`; ESC también la quita.
- **Guardado:** `joystick.cfg` y `keyboard.cfg` dentro de `engine->userHomeDirectory`, escritos como el `struct` completo (`fwrite`/`fread` de `sizeof(Joystick)` y `sizeof(Keyboard)`). `loadKeyConfig` fuerza `keyboard.control[CONTROL::UP] = 0`. `restoreKeyDefaults` llama a `keyboard.setDefaultKeys()`.
- **`sizeof`:** si se agrega un campo a `Keyboard` o `Joystick`, cambia `sizeof` y los `.cfg` viejos fallan al leerse. Las opciones nuevas no van en esas estructuras.

### `CGraphics.cpp`
**Función:** clase `Graphics`: todo lo visual de bajo nivel sobre SDL2.

- **Pantalla:** `updateScreen()` dibuja el aviso de medallas, copia la superficie `screen` a la textura y presenta. También maneja F12 (captura), F10 y Alt+Enter (pantalla completa).
- **Sprites:** `addSprite`, `getSprite(name, required)`, `animateSprites`, `quickSprite`.
- **Carga de imágenes:** `loadImage` (con variante por hue, saturación y valor), `RGBtoHSV`, `HSVtoRGB`, `colorize`, `loadMapTiles`, `loadBackground`.
- **Primitivas:** `putPixel`, `getPixel`, `drawLine`, `drawRect` (dos versiones), `blit(image, x, y, dest, centered)`, `alphaRect`, `createSurface`, `lock`, `unlock`.
- **Texto:** `loadFont`, `setFontColor`, `setFontSize`, `getString`, `drawString` (con y sin cache), chat (`createChatString`, `drawChatString`).
- **Efectos y avisos:** `fade`, `fadeToBlack`, `showMedalMessage`, `showLoading`, `drawWidgetRect`, `showErrorAndExit`, `showLicenseErrorAndExit`, `showRootWarning`.
- **Animación de líquidos:** `getWaterAnim`, `getSlimeAnim`, `getLavaAnim`.
- **Para la mira y los efectos:** `drawLine`, `drawRect` y `blit` son las herramientas para dibujar mira, trayectoria y arcos.
- **Archivo (adjunto del 2026-09-30):** 1273 líneas, LF (no CRLF).
- **`setGameScreenSize(w, h)`** (función libre al final del archivo, línea 1243; no es método de `Graphics`): si el tamaño pedido ya es el actual no hace nada; si no, crea una superficie ARGB de 32 bits y una textura `SDL_PIXELFORMAT_ARGB8888` (`SDL_TEXTUREACCESS_STREAMING`) de `w` x `h`, **libera la superficie y la textura anteriores y reemplaza los punteros `graphics.screen` y `graphics.texture`**, llama a `SDL_RenderSetLogicalSize(renderer, w, h)` y rellena de negro. El contenido anterior se pierde y quien la llama redibuja todo. Si falla la creación, deja lo anterior y avisa con `printf`.
- **Efecto:** cualquier código que guarde el puntero `graphics.screen` en una variable sobrevive mal a un cambio de tamaño; hay que volver a leerlo cada vez.

### `CGraphics.h`
**Función:** declaración de la clase `Graphics`. 132 líneas, CRLF. Sin guardas de inclusión. Se incluye desde `headers.h`.

- **Antes de la clase:** declara `void SDL_SetAlpha(SDL_Surface*, uint8_t)`, un reemplazo propio del que SDL1 traía (se define en `CGraphics.cpp`).
- **Privado:** `engine`, `gRect`, `font[5]`, colores de fuente, `spriteList`, `fontSize`, contadores de animación de agua, limo y lava, `currentLoading`, datos de capturas de pantalla, `chatString[1024]`, mensaje de medalla (`medalMessage`, `medalMessageTimer`, `medalType`), `fadeBlack` e `infoMessage`.
- **Público (datos):** `window`, `renderer`, `texture`, `screen`, `background`, `tile[MAX_TILES]` (una superficie por tile, hasta 256), `medal[4]`, `license[2]`, `infoBar`, `takeRandomScreenShots`, la estructura `SurfaceCache` (`text` y `surface`) y los colores como `int`: `red`, `yellow`, `green`, `darkGreen`, `skyBlue`, `blue`, `cyan`, `white`, `lightGrey`, `grey`, `darkGrey`, `black`.
- **Métodos:** los mismos que ya están en la entrada de `CGraphics.cpp`. Dos versiones de `loadImage`: `loadImage(nombre, srcalpha = false)` y `loadImage(nombre, hue, sat, value)`; dos de `drawRect` (con y sin color de borde) y dos de `drawString` (con y sin `SurfaceCache`). `blit(image, x, y, dest, centered)` recibe coordenadas `int`; `drawLine` recibe `float`.
- **`setGameScreenSize` no está declarada acá:** la declara `game.cpp` por su cuenta.
- **No hay ninguna variable de escala de dibujo.** Agregar un miembro cambia `sizeof(Graphics)`: todos los `.cpp` que incluyen `headers.h` se recompilan (con `-MMD -MP` en el Makefile ya pasa solo; la primera vez conviene `clean_and_compile.bat`).

### `CWidget.cpp`
**Función:** clase `Widget`, un elemento de menú.

- **Datos:** `name`, `groupName`, `label`, `options`, `x`, `y`, `type`, `min`, `max`, `enabled`, `visible`, `changed`, `value` (`int*`), `image`.
- **`setProperties(...)`:** si `name` o `groupName` pasan de 50 caracteres, `label` de 80 u `options` de 100, el juego hace `exit(1)`.
- **`setValue(int*)`:** el widget edita directamente esa variable.
- **`redraw()`:** libera la imagen para que se regenere.

### `widgets.cpp`
**Función:** dibuja todos los widgets del menú.

- **`drawWidgets()`:** genera las imágenes, calcula el ancho máximo, centra si `x == -1`, resalta el widget seleccionado y llama al dibujo de cada tipo.
- **`WG_RADIO`:** `drawOptions` separa las opciones con `|`; el valor es el índice elegido.
- **`WG_SLIDER` y `WG_SMOOTH_SLIDER`:** `drawSlider`, barra de 300 px rellena según `value / max`.
- **`WG_KEYBOARD`:** `drawKeyOption` muestra la tecla asignada.
- **`WG_JOYPAD`:** `drawJoypadButtonOption` (valor -2 = "N/A", menor que -2 = "...", si no "Button #n").
- **Centrado:** usa `1280` fijo, no `screen->w`.
- **Widget con `value == NULL`:** (salvo `WG_LABEL`) se saltea con aviso en debug.

### `CCollision.cpp`
**Función:** clase `Collision`, prueba de solapamiento entre rectángulos alineados a los ejes.

- **`collision(x0, y0, w0, h0, x2, y2, w1, h1)`:** recibe posición, ancho y alto de cada rectángulo, en ese orden. Usa comparaciones estrictas, así que dos rectángulos que solo se tocan cuentan como colisión.
- **`collision(Entity*, Entity*)`:** devuelve `false` si alguna de las dos tiene `ENT_NOCOLLISIONS`; si no, usa `x`, `y`, `width` y `height` de cada una.
- **`collision(Entity*, Switch*)`:** compara la entidad con un rectángulo fijo de 64 x 16 en la posición del interruptor.
- **Orden:** ancho y luego alto; invertirlo da una caja transpuesta sin aviso.

### `CMap.cpp`
**Función:** implementación de `Map`, límites del mapa y clasificación de tiles. Revisado desde el archivo adjunto.

- **`isValid(x, y)`:** acepta coordenadas desde 0 inclusive hasta `MAPWIDTH` y `MAPHEIGHT` exclusivas.
- **`isSolid(x, y)`:** devuelve `false` fuera del mapa; dentro, considera sólidos los índices desde `MAP_BREAKABLE` hasta antes de `MAP_DECORATION`.
- **`isLiquid(x, y)`:** también devuelve `false` fuera del mapa; dentro, incluye agua, limo y lava, tanto en los índices base como en el rango animado definido por las constantes.
- **Implicación para la IA:** comprobar `isValid` por separado antes de tratar un tile no sólido/no líquido como seguro; `isLiquid` no significa “mortal”, porque incluye agua.

### `CMap.h`
**Función:** declara la clase `Map` y sus datos y operaciones. Revisado desde el archivo adjunto.

- `data` guarda los tiles como `unsigned char data[MAPWIDTH][MAPHEIGHT]`.
- Declara `isValid`, `isSolid`, `isBreakable`, `isNoReset`, `isLiquid` e `isTopLayer`, además de las listas de enemigos, balas, obstáculos, trampas y demás entidades del mapa.
- Las dimensiones y el tamaño de tile se definen en `defs.h`.

### `entities.cpp`
**Función:** colisión de entidades con tiles, obstáculos y trenes; gravedad y movimiento. Revisado desde el archivo adjunto.

- **`checkBrickContactX/Y`:** consulta los tiles que tocan los bordes de la entidad y ajusta su posición al encontrar sólidos; también evalúa los atributos del mapa.
- **`moveEntity`:** aplica gravedad a las entidades móviles que no vuelan ni nadan; después resuelve colisiones horizontales y verticales con el mapa, obstáculos y trenes.
- **Implicación para la IA:** este archivo resuelve el movimiento físico, pero no encuentra rutas ni decide por adelantado si un destino es alcanzable.

### `CSpawnPoint.cpp`
**Función:** clase `SpawnPoint`, puntos donde aparecen enemigos.

- `create(...)`: los intervalos se pasan en segundos y se guardan multiplicados por 60.
- `think()`: si está activo, suma al contador hasta `maxInterval`.
- `readyToSpawn()`: verdadero cuando el contador llega al intervalo requerido.
- `reset()`: reinicia el contador y sortea un intervalo entre mínimo y máximo.

### `spawnPoints.cpp` — modificado
**Función:** decide qué hace cada punto de aparición cuando le toca.

- **`okayToSpawnEnemy(name, x, y)`:** rechaza si se pelea con Galdov o `devNoMonsters`; si el punto choca con una puerta; si un enemigo no nadador cae en líquido o uno nadador no está en agua; y, para los que no vuelan, si no hay suelo sólido en 30 tiles hacia abajo o hay líquido en la caída.
- **Comprobación contra puertas:** corregida; usaba `x * BRICKSHIFT` (x5) y ahora usa `x << BRICKSHIFT`. Tras el cambio rechaza apariciones sobre puertas que antes pasaban.
- **`doSpawnPoints()`:** para cada punto activo llama a `think()` y, cuando `readyToSpawn()`, actúa según `spawnType`.
  - `SPW_HAZARD`: bola de lava, caída de rocas, bombas, explosión (que además destruye el tile), balas potentes y estalactitas. Usa `engine.world` como dueño y armas `WP_LAVABALL1`, `WP_ROCK1`, `WP_BOMB`, `WP_SHELLS`, `WP_STALAGTITE`.
  - `SPW_ENEMY`: aparece a hasta 10 tiles de Bob, solo con la misión en curso; a mayor dificultad reaparece más rápido.
  - `SPW_ITEM`: suelta ítems de ayuda cerca de Bob.
  - `SPW_BOSSBULLET`: activa al jefe indicado si sigue vivo.
- **Distancia:** los peligros y `SPW_BOSSBULLET` solo se disparan si Bob está a menos de 700 x 500 px; enemigos e ítems no tienen esa restricción. El temblor de rocas usa 1280 x 720 y el de estalactitas 480 x 720.
- **Temblor de pantalla ya existente:** con caída de rocas o estalactitas cerca, llama a `engine.setPlayerPosition(x + azar, y + azar, limitLeft, limitRight, limitUp, limitDown)` con `MAP_SHAKEAMOUNT`. Es el mecanismo a reutilizar para el temblor por explosión.
- **Dato clave para la cámara:** `engine.setPlayerPosition(...)` es lo que fija `playerPosX/Y` a partir de la posición de Bob y los límites del mapa.
- **Cambios hechos (2026-09-29):** puertas con `<<`; `SPW_ITEM` valida `x < MAPWIDTH` e `y < MAPHEIGHT` (antes leía `map.data` fuera del mapa cerca de los bordes derecho e inferior); `SPW_BOSSBULLET` comprueba que `map.boss[...]` no sea nulo. 307 líneas, CRLF. Sin compilar ni probar.
- **Pendiente:** `y > map.limitDown` compara una fila de tile con un límite en píxeles; un nadador que caiga en lava o limo devuelve `true` porque `isLiquid` los incluye; `getDefinedEnemy` y `getSpawnableEnemy` no se comprueban; el temblor de rocas y estalactitas no respeta `game.screenShake`; el `default` de los peligros usa `printf` en vez de `debug`.
- **Reaparición de enemigos:** tras cada intento se pisa `requiredInterval` con `rrand(1, 30)` (fotogramas, sin x60) con probabilidad `(skill + 1) / (skill + 2)`; puede encadenar apariciones y este archivo no pone tope de enemigos vivos.

### `spawnPoints.h`
**Función:** cabecera de `spawnPoints.cpp`. 37 líneas, CRLF. Incluye `headers.h` y solo declara `extern`. Sin guardas de inclusión.

- **Funciones:** `addBullet`, `addEnemy`, `getDefinedEnemy`, `addTeleportParticles`, `addExplosion` y `dropHelperItems`.
- **Globales:** `engine`, `game`, `map`, `defEnemy[MAX_ENEMIES]`, `player` y `weapon[MAX_WEAPONS]`.
- **No declara** `okayToSpawnEnemy` ni `doSpawnPoints`; esta última la declara `game.h`.

### `map.cpp`
**Función:** dibujo y reglas del mapa.

- **`drawMap()`:** usa `engine.playerPosX/Y` como esquina superior izquierda de la cámara en coordenadas del mundo. Guarda `map.offsetX/Y` y recorta el offset para dibujar tiles. Cubre `screen->w / BRICKSIZE + 2` columnas, así que se adapta al tamaño de pantalla.
- **`drawMapTopLayer()`:** capa superior de tiles.
- **`showMap`, `addBlips`, `addMiniMapDoors`:** panel de mapa de 320x240 centrado, con puntos y puertas (leído solo en parte).
- **`evaluateMapAttribute(ent, attr)`:** cómo cada tile afecta a una entidad (aire, agua, limo y lava; variantes según tileset; cambia sprites de Bob con el tanque de aire). En niveles de hielo, el agua se trata como lava.
- **`raiseWaterLevel()`:** sube el agua convirtiendo filas de tiles; `map.waterLevel` baja 0.1 por llamada.
- **`doWind()`:** viento aleatorio (-3 a 3) que cambia cada 60 a 600 fotogramas.
- **`parseMapDataLine`, `loadMapData`:** carga del archivo del mapa.
- **Para la cámara adelantada:** el corrimiento hacia el cursor va en `engine.setPlayerPosition`.

### `traps.cpp` — modificado
**Función:** trampas del mapa.

- **Tipos:** `TRAP_TYPE_MINE`, `SPIKE`, `SWING` (bola con cadena), `BARRIER` (eléctrica), `FLAME`.
- **`addTrap`:** crea la trampa; espera `-1` significa aleatoria (10 a 60).
- **`toggleTrap`:** invierte `active`; en minas cambia el sprite entre `ActiveMine` e `InActiveMine`.
- **`drawTrapChain`:** dibuja la cadena de las bolas.
- **`doTrapCollisions`:** daña enemigos y a Bob. Si `damage == 10`, mata a Bob. Una mina que hace contacto lanza 10 explosiones.
- **`doTraps`:** recorre las trampas. Barreras y llamas se procesan siempre; el resto solo dentro de `ACTIVE_W/H`. Las minas caen por gravedad hasta un tile sólido y son las únicas que se eliminan al detonar. Las llamas disparan balas del arma `WP_FLAMETHROWER` con `engine.world` como dueño.
- **Agregado:** `requestTrapBlast(x, y, radius)` (pública) y `processTrapBlasts()` (interna). Las explosiones detonan minas activas dentro de su radio con una cola procesada al inicio de `doTraps`; las cadenas avanzan un fotograma por mina.
- **Conexión:** `explosions.cpp` declara `requestTrapBlast` por su cuenta y la llama al comienzo de `addExplosion`.

### `player.cpp` — modificado (versión actual con puntería por mouse)
**Función:** todo lo que hace y le pasa a Bob cada fotograma.

- **`doPlayer()`** (se llama cada fotograma), en este orden:
  1. Trucos de salud, caída fuera del mapa (`MIS_PLAYEROUT` o `MIS_PLAYERDEAD`) y estados que cortan el control: teletransporte, fin de misión, y `health < 1` o `immune > 120`.
  2. Jetpack (alterna con `CONTROL::JETPACK`, necesita combustible y aire), viento en niveles de ventisca, caminar, volar, saltar y bajar.
  3. `moveEntity(&player)` y límites del mapa.
  4. Puntería y disparo: `updatePlayerAim`, recarga con clic derecho, `handlePlayerGrenade` y bala normal o de escopeta.
  5. Agua y oxígeno, `player.think()`, parpadeo por inmunidad, y dibujo de Bob con estela de fuego si vuela.
  6. Solo con la misión en curso: `drawGrenadeCharge`, `drawPlayerAmmo`, `drawPlayerCrosshair`.
- **Munición y recarga:** globales `playerAmmo`, `playerAmmoMax`, `playerReloading`, `playerReloadTotal`. `resetPlayerAmmo` usa `currentWeapon->clip`; `startPlayerReload` dura `reload * 3` fotogramas (mínimo 30) y usa `player.reload` como cuenta regresiva. `drawPlayerAmmo` dibuja una barra de 26 px debajo de Bob (amarilla al recargar, roja con un cuarto o menos).
- **Otras funciones:** `resetPlayer` (vuelve al checkpoint, `immune = 120`, oxígeno y combustible en 7), `gibPlayer` (sangre o chispas amarillas según `game.gore`), `checkPlayerBulletCollisions` (daño de balas enemigas, inmunidad 120), `addPlayerScore` (medallas a 100.000, 250.000 y 500.000 puntos).
- **Medallas:** `presentPlayerMedal` lanza un hilo (`medalWorker`) que consulta a `medalServer`; no corre con trucos activados.
- **Lo que viene de otros archivos:** `updatePlayerAim`, `isMouseAiming`, `takePlayerReloadRequest`, `drawPlayerCrosshair` y las funciones de granada están en `bullets.cpp`.
- **`immune`:** `> 120` es aturdimiento tras un golpe (sin control); `1 a 120` es invulnerable pero con control.
- **Variables de munición y recarga:** son globales.
- **Disparo con mouse:** lee `engine.mouseLeft` directamente y no pasa por `config.isControl`.
- **Disparo de la escopeta:** se decide con `player.currentWeapon == &weapon[WP_SPREAD]`; dispara tres balas (`dy` 0, +2 y -2) y gasta 1 de munición por ráfaga. La recarga completa dura `reload * 3` fotogramas (mínimo 30). 658 líneas, CRLF.
- **Armas de uso limitado (`getLimitedShots`):** granadas (3 disparos) y cohete de Bob (arma 21, 1 disparo); al gastar el último vuelve a la pistola. Desde 2026-09-30 no recargan: `startPlayerReload` no hace nada con ellas y el clic derecho se ignora (`getLimitedShots() == 0` en la condición). Sin compilar ni probar.
- **Muerte:** `doPlayer` solo llama a `setMissionOver` en la caída al vacío (`y > limitDown + 500`: `MIS_PLAYEROUT` si aún queda salud, `MIS_PLAYERDEAD` si no). La muerte por daño la detecta `doGame` (`player.health < 1`). Con `health < 1` Bob queda sin control, `health` baja 1 por fotograma hasta -60 y ahí se llama `gibPlayer`. El daño de balas solo se aplica con `missionOverReason == MIS_INPROGRESS`.

### `bullets.cpp` — modificado (versión actual)
**Función:** balas de todos, granadas, y todo lo de puntería con mouse y mira. 996 líneas.

- **Puntería con mouse:** `updatePlayerAim(keyboardFire)` calcula el vector unitario Bob → cursor (`aimDirX/Y`), gira la cara de Bob y detecta el clic derecho (`takePlayerReloadRequest`). `isMouseAiming()` dice si el modo está activo. El mouse se lee con `engine.getMouseX()/getMouseY()`, en coordenadas de `graphics.screen`. La actividad del mouse se ignora si pasaron más de 250 ms sin llamar a la función (menús, pausa).
- **`drawPlayerCrosshair()`:** la mira, hecha con `drawRect` (cruz blanca con borde negro y punto rojo). Es el lugar donde iría el arco de recarga y las balas restantes.
- **`addBullet(owner, dx, dy)`:** crea la bala; si el dueño es Bob usa `getPlayerShotVelocity` (dirección al cursor). Escopeta en abanico alrededor de esa dirección.
- **Ciclo de vida:** `doBullets`, `destroyBullet`, `removeBullet`, `bounceBullet`, `bulletHasCollided`.
- **Efectos:** `addImpactSparks` (chispas de impacto en sentido contrario a la bala), `addBulletTrail` (estela en balas rectas rápidas).
- **Granadas:** carga mientras se mantiene el disparo (`GRENADE_CHARGE_MAX = 45` fotogramas, unos 0,75 s). `handlePlayerGrenade`, `resetGrenadeCharge`, `drawGrenadeCharge`; la velocidad vertical se limita a 8. Previsualización de trayectoria (`drawGrenadeTrajectory`, hasta 36 puntos).
- **Opciones desde `Game`:** `game.mouseAim`, `game.bulletTrail` y `game.grenadePreview` reemplazan a los `static const bool` `MOUSE_AIM_ENABLED`, `BULLET_TRAIL_ENABLED` y `GRENADE_PREVIEW_ENABLED` (que valían `true`). Con `mouseAim` en 0, `updatePlayerAim` apaga la puntería y el teclado funciona como antes.
- **Cámara adelantada:** `updateCameraLead` (se llama desde `updatePlayerAim`) suaviza un corrimiento igual a `(cursor - centro de pantalla) * factor`, con tope por nivel; `getCameraLead` lo entrega a `Engine::setPlayerPosition` y lo apaga suave si `doPlayer` deja de llamar (más de 250 ms). Nivel en `game.cameraLead` (0 apagada, 1 suave, 2 normal, 3 fuerte); la global `cameraLeadLevel` se eliminó.
- **Depende de:** `playerAmmo`, `playerAmmoMax` y `startPlayerReload` (de `player.cpp`).
- **Lanzamiento balístico de granadas enemigas (2026-09-30):** bloque `ENEMY GRENADE`: `solveGrenadeThrow` (ángulo plano que cae sobre el objetivo con la gravedad 0,1 de `doBullets`), `planEnemyGrenade(owner, &power)` (velocidad mínima con margen 1,12, sube si el vuelo supera la vida de la bala; `false` si ni a velocidad máxima llega) y `addEnemyGrenade(owner, power)` (velocidad 5 a 10 según `power`, error de puntería `24 - 6 * skill`, mínimo 4). `addBullet` usa `enemyThrowDX/DY` cuando `enemyThrowActive`, después del bloque `ENT_AIMS`. Simulación aparte contra la física de `doBullets`: falla de 1 a 10 px. No se conocen `dx`, `dy` ni la vida de `Alien Grenades` en `data/weapons`; por eso las velocidades son constantes propias. El archivo subido el 2026-09-30 tiene 1173 líneas (1348 con el cambio). `Alien Grenades` (`data/weapons`): `dx 3`, `dy -2`, vida 240; se usó la vida como fusible en `planEnemyGrenade`.
- **Armas con `dy`:** cualquier arma con `dy` distinto de 0 se trata como lanzada (gravedad, movimiento de Bob y de trenes, y con mouse velocidad vertical limitada a ±8). Bajo el agua solo disparan la pistola y `WP_AIMEDPISTOL`. `game.incBulletsFired()` usa `game.currentWeapon`. 996 líneas, CRLF.

### `traps.h`
**Función:** cabecera de `traps.cpp`. Incluye `headers.h` y declara como `extern` lo que usa `traps.cpp`: `addBullet`, `addExplosion(float x, float y, int radius, Entity *owner)`, `throwAndDamageEntity(Entity*, int damage, int minDX, int maxDX, int DY)`, los globales `audio`, `engine`, `graphics`, `map`, `player` y `weapon[]`.

- **No declara** las funciones propias de `traps.cpp` (`addTrap`, `doTraps`, etc.); eso está en otra cabecera.
- **No tiene** guardas de inclusión.
- **No hace falta tocarla** para `requestTrapBlast`: `explosions.cpp` la declara por su cuenta y `traps.cpp` la define antes de usarla.

### `triggers.cpp`
**Función:** `activateTrigger(linkName, mensaje, active)`, la función que conecta interruptores con objetos por nombre.

- **Nombres especiales:** `@none@` (no hace nada), `WATERLEVEL` (sube el agua si el nivel pedido es menor), `OBSTACLERESET` (devuelve obstáculos a su lugar con partículas de teletransporte).
- **Resto:** busca por nombre en trenes/puertas, puntos de aparición, teletransportadores y trampas.
- **Diferencia:** los puntos de aparición **alternan** su estado (`!sp->active`) y las trampas también (`toggleTrap`); trenes y teletransportadores usan el valor `active` recibido.
- Si nada coincide, avisa en debug.

### `weapons.cpp`
**Función:** armas del juego.

- **`loadDefWeapons()`:** lee `data/weapons` con `strtok` y `sscanf`; llena `weapon[id]` (nombre, daño, salud, dx, dy, recarga, sprites, sonido, flags, cargador). Termina con la línea `@EOF@`.
- **`getDefaultClipSize(id)`:** cargador por arma (pistola 12, ametralladora 30, láser 20, granadas 5, escopeta 8, cohetes 4, plasma 18 (antes 25), lanzallamas 100). Cambio entregado en la mejora 1; solo lo usa Bob, porque los enemigos disparan en ráfagas y no usan cargador.
- **`getRandomStraightWeapon`, `getRandomAimedWeapon`, `getRandomGaldovWeapon`:** arma aleatoria para enemigos.
- **`getWeaponByName(name)`:** acepta `randomStraight` y `randomAimed`; si no encuentra el nombre, devuelve la pistola.
- **`data/weapons`:** si no termina con `@EOF@`, el bucle sigue con un puntero nulo.

### `CWeapon.cpp`
**Función:** clase `Weapon`: valores por defecto y velocidad del disparo. 44 líneas, CRLF.

- **Constructor:** `name` vacío, `id`, `damage`, `reload`, `dx`, `dy`, `flags` y `health` en 0, y `fireSound = 1`. No inicializa `clip` ni `sprite[]`; las posiciones de `weapon[]` sin definición dependen de que el arreglo global arranque en cero.
- **`setName`:** copia con `strlcpy` limitada al tamaño de `name`.
- **`getSpeed(face)`:** devuelve `dx - (dx * 2 * face)`: `+dx` mirando a la derecha (`face` 0) y `-dx` a la izquierda. Con `dx` 0 (Flame Thrower) devuelve 0.
- **`CWeapon.h`:** sin ver.

### `CEntity.cpp`
**Función:** clase `Entity`, la base de Bob, enemigos, balas, ítems y demás. 209 líneas, CRLF, ASCII. Incluye `headers.h`.

- **Constructor:** todo en 0 salvo `immune = 120`, `environment = ENV_AIR`, `oxygen = 7`, `fuel = 7`, `deathSound = -1`, `dead = DEAD_ALIVE`, `owner = this`, `next = NULL` y `sprite[0..2] = NULL`. No inicializa `currentFrame` ni `currentTime`; los fija `setSprites`.
- **`setSprites(s1, s2, s3)`:** guarda los tres sprites (0 = derecha, 1 = izquierda, 2 = muerte o estado especial), pone `currentFrame` al azar entre 0 y `sprite[0]->maxFrames` si `maxFrames > 0`, y **fija `width` y `height` con el tamaño de `sprite[0]->image[0]`** (`->w`, `->h`) y `currentTime = 1`. Es el único lugar visto donde el hitbox sale del tamaño de la imagen: una imagen de otra resolución cambia el hitbox.
- **`animate()`:** baja `currentTime`; al llegar a 0 sube `currentFrame` y llama `sprite[face]->getNextFrame`.
- **`getFaceImage()`:** con `health > 0` e `immune <= 120` devuelve `sprite[face]->image[currentFrame]`; si no, `sprite[2]->getCurrentFrame()` (el fotograma propio del sprite 2, no el de la entidad).
- **Movimiento:** `place` (fija `x`, `tx`, `y`, `ty`), `setVelocity`, `move` (suma `dx` y `dy`), `setRandomVelocity` (`dx` de -2 a 2, `dy` de -15 a 0).
- **`applyGravity()`:** en aire y sin `ENT_WEIGHTLESS` suma 0,5 a `dy`; en cualquier otro caso `dy = 1`. Limita `dy` a -12 y 12.
- **`checkEnvironment()`:** aire, `oxygen` +2 hasta 7. Agua, los que no nadan pierden 1 de `oxygen`; a 0 pierden 1 de salud y `thinktime = 30`. Limo, -1 de salud y lava -2, solo con `immune == 0`.
- **`think()`:** baja `immune`; si `falling`, `immune == 121` y está en aire, lo sube a 122. Envuelve `thinktime` con `baseThink`; **`reload` baja 1 por llamada y se limita a 0 a 255**. `ENT_DYING` resta 1 de salud por llamada. Los que vuelan gastan `fuel` cuando `thinktime == 0` y pierden `ENT_FLIES` al llegar a 0; los demás lo recargan. Llama a `checkEnvironment` cuando `thinktime == 0` salvo que sea `ENT_INANIMATE`.
- **Sin ver:** `CEntity.h`.

### `CSprite.h`
**Función:** declaración de `Sprite`, que hereda de `GameObject`. 46 líneas, CRLF. Sin guardas de inclusión.

- **Datos:** `name[50]`, `image[8]` (`SDL_Surface*`), `frameLength[8]`, `currentFrame`, `currentTime` y `maxFrames` (los tres `unsigned char`) y `randomFrames`.
- **Métodos:** `setFrame`, `animate`, `getNextFrame`, `getCurrentFrame` y `free`.
- **Límites:** hasta 8 fotogramas por sprite; `frameLength` es `unsigned char` (máximo 255 fotogramas por imagen).
- **No guarda el tamaño lógico de cada imagen:** solo el puntero a la superficie.

### `CSprite.cpp`
**Función:** implementación de `Sprite`. 112 líneas, CRLF.

- **`setFrame(i, imagen, tiempo)`:** guarda la imagen y su duración, reinicia `currentFrame = 0` y `currentTime = frameLength[0]`. **`maxFrames` es el índice más alto asignado**, no la cantidad de fotogramas (`Entity::setSprites` usa `maxFrames + 1` como cantidad).
- **`animate()`:** baja `currentTime`; al llegar a 0 sube `currentFrame`, vuelve a 0 si pasa de 7 o si el siguiente tiene `frameLength` 0, y carga su duración.
- **`getNextFrame(&frame, &time)`:** no avanza el fotograma; lo corrige (vuelve a 0 si es 8 o más o si su duración es 0) y entrega la duración. `Entity::animate` incrementa antes de llamarla.
- **`getCurrentFrame()`:** devuelve `image[currentFrame]`.
- **`free()`:** libera cada superficie con `SDL_FreeSurface` y reinicia los contadores. Los sprites de `loadResources` se liberan con `graphics.free()` en cada misión.

### `items.cpp`
**Función:** ítems del mapa: crearlos, soltarlos al morir enemigos, recogerlos y cargar `data/defItems`. 453 líneas, LF, UTF-8 (un comentario de `pickUpItem` tiene un carácter dañado; no lo toques). Incluye `items.h` (sin ver) y declara `resetPlayerAmmo` por su cuenta.

- **`addItem(tipo, nombre, x, y, sprite, salud, valor, flags, movimientoAleatorio)`:** crea el ítem con `ENT_INANIMATE + ENT_BOUNCES + ENT_COLLECTABLE`, lo levanta si nace dentro de suelo y suma `rand % 120` a la salud (vida en fotogramas). `ITEM_MISC_INVISIBLE` pasa a `ITEM_MISC_NOSHOW` si `gameData.completedWorld` o `skill == 3`.
- **`dropRandomItems(x, y)`** (lo llama `doEnemies` al morir un enemigo con `value`, dos veces en `enemies.cpp`): no suelta nada si el tile es sólido; en misión de jefe usa `dropBossItems`. Si no, suelta de 1 a 5 ítems, cada uno de puntos por defecto, con 1 de 8 de ser un arma al azar (`ITEM_PISTOL` a `ITEM_SPREAD`) y luego 1 de 13 de ser cereza (con `cherryChance = 10 + 10 * skill`: 1 triple, 5 dobles, el resto simple). Vida 240 más azar, con `ENT_DYING`. Estimación (sin medir): ~30 % de las muertes sueltan al menos un arma.
- **`dropBossItems`:** 1 de 5; ítem de `ITEM_PISTOL` a `ITEM_DOUBLECHERRY` (solo cerezas si Bob está en agua) y 1 de 10 de triple cereza.
- **`dropHelperItems(x, y)`** (lo llama `SPW_ITEM`): de 1 a 5 ítems entre `ITEM_PISTOL` y `ITEM_TRIPLECHERRY` (armas y cerezas, sin puntos).
- **`pickUpItem`:** con `ITEM_PISTOL` a `ITEM_SPREAD` hace `player.currentWeapon = &weapon[item->id]`, `game.currentWeapon = item->id` y `resetPlayerAmmo()`. **El id del ítem se usa directo como índice de `weapon[]` y de `bulletsFired[]`**, y el arma recogida reemplaza a la actual (incluso la pistola a una ametralladora). Puntos suman `addPlayerScore`; cerezas curan hasta `MAX_HEALTH`; `ITEM_MISC` lo deja cargado por Bob. Muestra "Picked up a/an ..." salvo con puntos o en misión de jefe; llama a `checkObjectives` y da la medalla `LRTS_PART`.
- **`doItems`:** actualiza dentro de `ACTIVE_W/H`, parpadea con salud menor que 60, recoge por colisión si es `ENT_COLLECTABLE` y elimina al llegar a salud 0 salvo que Bob lo cargue.
- **Otras:** `stealCrystal`, `dropCarriedItems` (devuelve lo cargado al último checkpoint), `carryingItem`, `showCarriedItems` (centra con 1280 fijo).
- **`loadDefItems()`:** lee `data/defItems` con `sscanf`, llena `defItem[id]` y pide cada sprite con `getSprite(..., true)`; no comprueba `MAX_ITEMS` y sin `@EOF@` el `strcmp` recibe un puntero nulo.
- **Ids nuevos de ítem:** las constantes `ITEM_*` son un enum de `defs.h` (armas 0 a 4, cerezas 5 a 7, puntos 8 a 14, `ITEM_MISC` = 100). Los ítems de mapa cuentan con `id >= ITEM_MISC`, así que los ids 15 a 24 quedan libres y caben en `defItem[MAX_ITEMS]` (25). `pickUpItem` usa `weapon[item->id]` directo, por lo que un ítem nuevo necesita un mapeo al arma 21 o 22. Su nombre no debe coincidir con el `target` de un objetivo (ver `objectives.cpp`).

### `resources.cpp`
**Función:** carga de todo lo que necesita una misión (`loadResources`), más `loadSprite` y `loadSound`. 180 líneas, LF, ASCII. Incluye `resources.h` (sin ver).

- **`loadSprite(línea)`:** formato `nombre hue sat val archivo1 tiempo1 archivo2 tiempo2 ... @none@`, hasta 8 fotogramas. Pasa `hue`, `sat` y `val` a `graphics.loadImage`, así que **un sprite nuevo puede reutilizar la imagen de otro con otro color**. El `sscanf` lee 8 pares fijos; cada línea debe terminar su lista con `@none@` porque los nombres no leídos quedan sin inicializar y el bucle se detiene al encontrarlo (o en el octavo).
- **`loadResources()`, en este orden:** `graphics.resetLoading`, `audio.free()` y `graphics.free()`; sprites de cabeceras de opciones (`cheatHeader`, `optionsHeader`, `joystickHeader`, `keyHeader`, `optionsBackground`); pantalla "Loading..."; `engine.loadDefines()` (error con `data/defines.h`); `data/mainSprites` hasta `@EOF@` (error `ERR_FILE` si falta); sonidos; `loadDefWeapons`, `loadDefEnemies` y `loadDefItems`; `graphics.loadMapTiles("gfx/common")`; sprites de Bob; `loadMapData(game.mapName)`; `createBoss(game.stageName)`; `game.canContinue = 0`; y al final `engine.defineList.clear()`.
- **Consecuencias del orden:** los sprites que piden `data/weapons`, `defEnemies` y `defItems` (`getSprite(nombre, true)`) tienen que estar en `data/mainSprites`, o el juego se cierra. Como `graphics.free()` corre en cada misión, `weapon[]`, `defEnemy[]` y `defItem[]` se releen cada vez. `getValueOfFlagTokens` solo funciona durante la carga, porque la lista de defines se vacía al final.
- **Sonidos:** carga el archivo de cada `SND_*` con su ruta `sound/...` (`SND_HIT` es `sound/punch`, `SND_SPREADGUN` es `sound/plasma`, `SND_SWITCH1` y `SND_SWITCH2` comparten `sound/switch`). No carga acá `SND_AMBIANCE`, `SND_BOSSCUSTOM1` a `5`, `SND_CHEAT`, `SND_HIGHLIGHT` ni `SND_SELECT`.
- **Detalles:** el texto "Loading..." se centra con 640 x 480 fijos más el desvío de `graphics.screen`; con `USEPAK` agrega un `delay(100)` al final.

### `objectives.cpp`
**Función:** objetivos de la misión y MIAs. 331 líneas, CRLF. Incluye `objectives.h` (sin ver).

- **`adjustObjectives()`:** marca como rescatadas las MIAs ya guardadas en `gameData` y suma `foundMIAs`; si se llega a las requeridas (o `skill == 3`), exige todas. Por objetivo: `skill 0` los vuelve opcionales, `skill >= 3` los vuelve requeridos y, si `needRequired`, también (esto último pisa el caso de `skill 0`). En una fase ya superada restaura `completed` y los valores desde `gameData`, los limita a `0..targetValue` y completa si se alcanzó el objetivo.
- **Consultas:** `allObjectivesCompleted` (MIAs requeridas y objetivos `required` completos), `perfectlyCompleted` (todas las MIAs y todos los objetivos), `requiredEnemy(nombre)` (cualquier objetivo con ese `target`, sin mirar si es requerido o está completo) y `requiredObjectivesCompleted` (como la primera, saltando los de `target` `Exit`).
- **`autoCompleteAllObjectives(todos)`:** el truco F3; completa objetivos, iguala `foundItems` con `totalItems` y pone `health = 0` en tantas MIAs como las requeridas.
- **`checkObjectives(nombre, avisarSiempre)`:** para cada objetivo pendiente con `target` igual a `nombre`: los que contienen `Combo-` toman `game.currentComboHits` (limitado a `targetValue`); el resto suma 1. Al completarse, fuera de misión de jefe muestra "Objective Completed - Check Point Reached!" y llama `game.setObjectiveCheckPoint()`; en jefe solo el mensaje. `Get the Aqua Lung` y `Get the Jetpack` activan `hasAquaLung` y `hasJetPack` y dan las medallas `Aqua_Lung` y `Jetpack`. Si no se completa, avisa el progreso cada 10, en los últimos 10 o con `avisarSiempre`; si no toca avisar, **hace `return` y deja sin revisar los objetivos siguientes** (cuenta bien uno por nombre, pero dos objetivos con el mismo `target` no siempre avanzan juntos).
- **Quién la llama con qué nombre:** `enemies.cpp` con `Combo-<nombre del arma>` (el nombre de la bala, que es el del arma), `Enemy`, el nombre del enemigo y `Galdov`; `pickUpItem` (`items.cpp`) con el nombre de cada ítem que no sea de puntos.
- **Para armas nuevas:** el nombre de un arma nueva genera un `Combo-<nombre>` que ningún objetivo existente espera, así que no cuenta para combos de mapas. El nombre de un ítem nuevo no debe coincidir con el `target` de un objetivo existente.

### `explosions.cpp` — modificado
**Función:** `addExplosion(x, y, radius, owner)`, la explosión y su efecto sobre enemigos y Bob.

- **Partículas:** `radius` partículas con el sprite `Explosion`, más el sonido `SND_GRENADE`.
- **Enemigos:** los que están dentro del radio pierden `radius - distancia` de salud (salvo `ENT_IMMUNE` e `ENT_IMMUNEEXPLODE`), sangran o echan humo si `ENT_EXPLODES`, reciben empuje al azar, y dan puntaje si el dueño es Bob. Se avisa a los objetivos de misión (`checkObjectives`) cuando mueren.
- **Bob:** recibe de 5 (centro) a 1 (borde) de daño, incluso de sus propias granadas, y sale despedido en sentido contrario a la explosión. No recibe daño si está inmune (`immune`), muerto o con la misión terminada.
- **Minas:** al comienzo llama a `requestTrapBlast(x, y, radius)` (definida en `traps.cpp`), así las explosiones detonan las minas cercanas. Alcanza a todas las explosiones: granadas, minas, enemigos que explotan y `HAZARD_EXPLOSION`.
- **Temblor de pantalla:** `addExplosionShake` calcula la distancia de Bob a la explosión (alcance: 5 x radio, mínimo 250 px). Potencia de 2 px (borde) a 12 px (pegado), duración de 180 a 500 ms. `getScreenShake` entrega el desvío actual, medido con `SDL_GetTicks`. Nivel en `game.screenShake` (0 apagado, 1 suave, 2 normal, 3 fuerte); la global `screenShakeLevel` se eliminó.

### `CEngine.cpp` — modificado
**Función:** clase `Engine`: entrada, cámara, menús por widgets, carga de datos del pak, tiempos. 1145 líneas, CRLF. Leído completo en esta sesión.

- **Entrada (`getInput`):** eventos SDL de mouse (botones y movimiento en `mouseX/mouseY`), teclado, joystick y hat; captura de teclas y botones para reasignar controles; cerrar ventana; perder el foco pone `paused = true`. Código de trucos `LOCKANDLOAD`. `getMouseX/Y`, `moveMouse` (limita a 1280 x 720), `clearInput`, `flushInput`, `userAccepts`.
- **Cámara:** `setPlayerPosition(x, y, limitLeft, limitRight, limitUp, limitDown)` calcula `playerPosX/Y = pos - OFFSETX/Y` y recorta con los límites del mapa. Los límites se ajustan al tamaño real de `graphics.screen` (están definidos para 640 x 480). Límites negativos = pantalla sin scroll.
- **Agregado a `setPlayerPosition`:** en pantallas que scrollean suma el corrimiento de la cámara adelantada y el temblor antes de recortar, así nunca se sale del mapa. También inicializa `mouseX = mouseY = 0` en el constructor.
- **Widgets del menú:** `loadWidgets(archivo)` lee las definiciones desde un archivo de datos (formato `TIPO nombre grupo "etiqueta" "opciones" x y min max`); `getWidgetByName`, `showWidget`, `showWidgetGroup`, `enableWidget`, `enableWidgetGroup`, `widgetChanged`, `highlightWidget`. **`setWidgetVariable(nombre, &variable)` enlaza un widget con un `int`**. `processWidgets` cambia el valor con izquierda/derecha dentro de `min` y `max`.
- **Datos:** `loadData` (del pak o del disco), `unpack`, `loadDefines` (`data/defines.h`), `getValueOfDefine`, `getDefineOfValue`, `getValueOfFlagTokens` (flags separados por `+`).
- **Tiempos y mensajes:** `doFrameLoop` (0 a 59), `doTimeDifference`, `delay`, `setInfoMessage` (dura 180 fotogramas).
- **Para las opciones nuevas:** agregar una opción del menú implica una línea nueva en el archivo de datos de widgets de opciones (va dentro del pak, hay que regenerar `blobwars.pak`) y un `setWidgetVariable` en el código de la pantalla de opciones. Los textos de opciones no pueden pasar de 100 caracteres.

**Comprobado al leer el archivo completo**
- **`loadData`:** con `USEPAK` lee **solo** del pak (`unpack`); sin `USEPAK` lee el archivo suelto, con la ruta relativa al directorio de trabajo (`data/optionWidgets`).
- **`setWidgetVariable` con un nombre inexistente:** no hace nada (solo un `debug`).
- **`highlightWidget(nombre)` con un nombre inexistente:** deja `highlightedWidget = NULL`, y el siguiente `processWidgets` lo desreferencia (fallo).
- **Widget de valor nulo:** Intro/Espacio con `value == NULL` solo escribe un `debug`, pero izquierda/derecha sobre un `RADIO` o `SLIDER` sin variable enlazada desreferencia el puntero nulo.
- **Navegación del menú:** solo teclado y joystick (flechas, Intro, Espacio, Ctrl); el mouse no mueve el resaltado. `highlightWidget(1)` salta los widgets con `type == 4`.
- **`loadWidgets`:** el tipo se busca por nombre en `widgetName[]`; un tipo desconocido deja `type` sin asignar. Termina con `END` o al acabarse las líneas. Deja resaltado el primer widget.
- **`loadDefines`:** lee `data/defines.h` ignorando líneas con `/*`; `getValueOfDefine` y `getDefineOfValue` terminan el programa (`exit(1)`) si no encuentran el valor. `getValueOfFlagTokens` suma los flags separados por `+`; un flag desconocido termina el programa salvo que `IGNORE_FLAGTOKEN_ERRORS` esté activo.
- **`strtok_r`:** se define a mano cuando no es Unix (`!UNIX`).
- **Mouse:** `mouseLeft` y `mouseRight` se actualizan con los eventos de botón; `clearInput` los pone en 0. El cursor del sistema está oculto (`SDL_ShowCursor(SDL_DISABLE)` en `initSystem`).
- **Trucos:** el código `LOCKANDLOAD` se busca en las últimas 25 teclas; `cheats` no se inicializa en el constructor.

### `game.cpp`
**Función:** sección de juego: bucle de la misión (`doGame`), fin de misión, menú de pausa, pantalla de Game Over y pantallas auxiliares. 981 líneas, LF. Leído completo. Incluye `game.h` (sin ver).

- **Funciones:** `newGame`, `showInGameOptions`, `doGameStuff`, `gameover`, `showMissionInformation`, `beamInPlayer`, `doGame`.
- **`doGameStuff()`:** entrada, `config.populate`, `replayData.read`; `config.doPause()` solo con `MIS_INPROGRESS`; luego actualiza y dibuja fondo, efectos, trenes, trampas, mapa, líneas, interruptores, ítems, balas, MIAs, jefes, enemigos, obstáculos, teletransportadores, viento y partículas. También la usan `gameover`, `showMissionInformation` y `beamInPlayer`.
- **`doGame()`, por fotograma y en este orden:** cámara (salvo con `MIS_PLAYEROUT`), `doSpawnPoints`, `doGameStuff`, `doPlayer`, `raiseWaterLevel`; luego chequeo de muerte (`player.health < 1` llama `setMissionOver(MIS_PLAYERDEAD)` y apaga música y ambiente; en limo o lava, `MIS_PLAYEROUT` con `immune = 130`); tecla MAP; `drawMapTopLayer`, `doStatusBar`, `doMusicInfo`; tecla ESC (abre `showInGameOptions` solo con `missionOver == 0`; en replay hace `exit(0)`); objetivos completos (`MIS_COMPLETE`); cuenta regresiva; tiempo de misión cada 60 fotogramas; pausa (`engine.paused`); F3 (truco); retardo de fotograma.
- **Cuenta regresiva:** `missionOver > 0` baja 1 por fotograma. Al llegar a 0: `MIS_PLAYEROUT` pone pantalla negra 1 s, llama `resetPlayer()` y `resetMissionOver()` y sigue; `MIS_COMPLETE` pasa a `MIS_PLAYERESCAPE` con 2 s más (en Space Station sale); `MIS_GAMECOMPLETE` pasa a `MIS_PLAYERESCAPE` con 4 s más; cualquier otro motivo (incluidos `PLAYERDEAD`, `PLAYERESCAPE` y `PLAYERQUIT`) hace `break` y sale del bucle.
- **Después del bucle:** vuelve al tamaño de menú, reevalúa `allObjectivesCompleted()` (que pisa el motivo con `COMPLETE` o `GAMECOMPLETE`) y hace `switch` de `missionOverReason`: `COMPLETE` a `SECTION_HUB` (a `SECTION_TITLE` en práctica; en Space Station carga Final Battle); `GAMECOMPLETE` a `SECTION_CREDITS`; `TIMEUP` (pone `canContinue = 0`) y `PLAYERDEAD` a `SECTION_GAMEOVER` (si `health > -60` la pone en -99 y llama `gibPlayer`); `PLAYERESCAPE` a `SECTION_HUB`; `PLAYERRESTART` a `SECTION_GAME`; cualquier otro (p. ej. `PLAYERQUIT`) a `SECTION_TITLE`.
- **`gameover()`:** carga `data/gameOverWidgets` (si falla, `showErrorAndExit`), la imagen `gfx/main/gameover.png` y la música de Game Over. Cada fotograma recalcula la cámara, corre `doGameStuff` y dibuja la imagen centrada. Espera `engine.userAccepts()`; entonces, si es misión de jefe o `canContinue == 0`, sale directo (`quit = 1`, a `SECTION_TITLE`); si no, muestra los widgets `gameOverNo` (enlazado a `cont`) y `gameOverYes` (enlazado a `quit`), con `canContinue` en la etiqueta. Continuar pone `continueFromCheckPoint = true` y devuelve `SECTION_GAME`; en `doGame` eso llama `useObjectiveCheckPoint` (baja `canContinue`, sube `continuesUsed`) y da la mitad de la salud.
- **`showInGameOptions()` (pausa con ESC):** usa `data/inGameWidgets`; grupos `options` (`continue`, `options`, `escape`, `restart`, `quit`, `train`) y confirmaciones `warning`, `restartconf`, `escapeconf`, `quitconf`, `trainconf`. `escape` y `restart` se deshabilitan en jefes, práctica y Space Station. `options` llama `showOptions()` y después sale del menú de pausa (vuelve al juego). Confirmar salir o entrenar llama `setMissionOver(MIS_PLAYERQUIT)`; reiniciar, `MIS_PLAYERRESTART`; escapar, `MIS_PLAYERESCAPE`. ESC o la tecla de pausa cierran el menú.
- **Tamaño de pantalla:** `doGame` llama `setGameScreenSize(GAME_VIEW_W, GAME_VIEW_H)` al empezar y vuelve a `UI_VIEW_W/H` para la pausa, el mapa (`showMap`), el menú de ESC y al salir del bucle; por eso `gameover()` corre con tamaño de menú. La función se declara acá y se define en `CGraphics.cpp` (línea 1243; ver su entrada).
- **Pantalla de Game Over:** solo avanza tras `userAccepts()` (sin ESC ni tiempo límite). El menú de ESC solo se abre con `missionOver == 0`. `setMissionOver` no reinicia la cuenta si `missionOver > 0`. Los nombres `gameOverNo` y `gameOverYes` no coinciden con su efecto (el primero continúa, el segundo sale). `trainno` y `trainyes` comparten variable con `quitno` y `quityes`.

### `defs.h`
**Función:** constantes y macros globales. 416 líneas, CRLF. Leído completo. Incluye `defines.h` de `src` (el que también usan los datos de mapas; sin ver, ver "Por ubicar").

- **Límites:** `MAX_SOUNDS` 75, `MAX_TILES` 256, `MAX_WEAPONS`, `MAX_ITEMS` y `MAX_ENEMIES` 25, `MAX_HEALTH` 10, `MAX_FPS` 62. `PAK_MAX_FILENAME` 60 (largo máximo del nombre de un archivo en el pak).
- **Ítems:** enum `ITEM_PISTOL` 0, `MACHINEGUN`, `LASER`, `GRENADES`, `SPREAD` (4), `CHERRY` 5, `DOUBLECHERRY`, `TRIPLECHERRY` (7), `POINTS` 8 a `POINTS7` (14), `ITEM_MISC` = 100, `ITEM_MISC_NOSHOW` 101 e `ITEM_MISC_INVISIBLE` 102. Confirmado `MAX_WEAPONS` = 25. Los sonidos de las armas (`SND_ROCKET` 11, `SND_SPREADGUN` 34) coinciden con los números de `data/weapons`.
- **Secciones:** `SECTION_INTRO` 0, `TITLE` 1, `HUB` 2, `GAME` 3, `GAMEOVER` 4, `CREDITS` 5, `EASYOVER` 6.
- **Misión:** `MIS_INPROGRESS` 0, `COMPLETE` 1, `PLAYEROUT` 2, `PLAYERDEAD` 3, `PLAYERQUIT` 4, `PLAYERESCAPE` 5, `GAMECOMPLETE` 6, `TIMEUP` 7, `PLAYERRESTART` 8.
- **Vista:** `UI_VIEW_W/H` 1280 x 720 (menús), `GAME_VIEW_W/H` 800 x 600 (misión). `OFFSETX/Y`, `ACTIVE_W/H` (+160, +120) y `DRAW_W/H` (+60, +20) se calculan sobre `graphics.screen`, así que cambian con `setGameScreenSize`.
- **Mapa:** `MAPWIDTH` 400 x `MAPHEIGHT` 300 tiles de `BRICKSIZE` 32; `MAP_*` con los índices de tile (aire, agua, limo, lava, rompibles, sólido, animados, capa superior) y `MAP_SHAKEAMOUNT` 2.
- **Jugador:** `PLAYER_WALK_SPEED` 4, `PLAYER_FLY_SPEED` 8, `PLAYER_JUMP_SPEED` -10,25. Entornos `ENV_AIR`, `WATER`, `SLIME`, `LAVA` (0 a 3).
- **Armas:** `WP_PISTOL` 0, `MACHINEGUN` 1, `LASER` 2, `GRENADES` 3, `SPREAD` 4, `ROCKETS` 5, `PLASMARIFLE` 6, `FLAMETHROWER` 7, `ICEGUN` 8, `LAVABALL1` 9, `LAVABALL2` 10, `AIMEDPISTOL` 11, `ALIENSPREAD` 12, `SHELLS` 13, `ROCK1` 14, `STALAGTITE` 15, `BOMB` 16, `ALIENLASER` 17, `ALIENGRENADE` 18, `AIMEDMACHINE` 19.
- **Controles:** `CONTROL::TYPE` = `UP`, `DOWN`, `LEFT`, `RIGHT`, `FIRE`, `JUMP`, `MAP`, `PAUSE`, `JETPACK`, `MAX`.
- **Widgets:** `widgetName[]` y `WG_BUTTON` 0, `RADIO` 1, `SMOOTH_SLIDER` 2, `SLIDER` 3, `LABEL` 4, `KEYBOARD` 5, `JOYPAD` 6. El `type == 4` que salta `highlightWidget` es `WG_LABEL`.
- **Sonidos y canales:** enum `SND_*` (`SND_CHEAT`, `SND_HIGHLIGHT` y `SND_SELECT` van fijos al final, en `MAX_SOUNDS - 3`, `- 2` y `- 1`); canales `CH_*`.
- **Pak:** `USEPAK` (1 si nadie lo define), `PAKLOCATION`, `PAKNAME` (`blobwars.pak`), `PAKFULLPATH`; enum `PAK_IMG`, `SOUND`, `MUSIC`, `DATA`, `FONT`, `TAGS`.
- **`debug(x)`:** con `DEBUG` distinto de cero imprime; si no, se expande a un bloque vacío. El Makefile no define `DEBUG`, así que no imprime nada.
- **Modificado respecto del original:** `min` y `max` ya no son macros sino plantillas (el propio comentario dice que las macros rompían la biblioteca estándar); admiten tipos mezclados como antes.
- **Cambio de constantes:** al cambiar una constante de este archivo hay que recompilar todo (con las dependencias automáticas del Makefile ya debería pasar solo, pero la primera vez conviene `clean_and_compile.bat`).

### `game.h`
**Función:** cabecera de `game.cpp`. 82 líneas, CRLF. Incluye `headers.h` y solo declara `extern`. Sin guardas de inclusión.

- **Declara (definidas en otros archivos):** dibujo y mapa (`drawMap`, `drawMapTopLayer`, `showMap`, `raiseWaterLevel`, `doWind`), HUD (`doTimeRemaining`, `doStatusBar`, `doMusicInfo`, `doPauseInfo`), actualización por fotograma (`doItems`, `doPlayer`, `doTrains`, `doSwitches`, `doBullets`, `doEffects`, `doParticles`, `doMIAs`, `doObstacles`, `doEnemies`, `doBosses`, `doSpawnPoints`, `doTeleporters`, `doLineDefs`, `doTraps`), menús (`showOptions`, `drawWidgets`), jugador (`dropCarriedItems`, `resetPlayer`, `gibPlayer`, `addTeleportParticles`) y fin de misión (`allObjectivesCompleted`, `showMissionClear`, `autoCompleteAllObjectives`, `checkEndCutscene`, `processPostMissionData`, `clearAllMissionData`, `saveGame`). Los que ya están en el registro son `doPlayer`, `resetPlayer` y `gibPlayer` (`player.cpp`), `doBullets` (`bullets.cpp`), `doEnemies` (`enemies.cpp`), `doEffects` (`effects.cpp`), `doParticles` (`particles.cpp`), `doTraps` (`traps.cpp`), `doSpawnPoints` (`spawnPoints.cpp`) y `drawWidgets` (`widgets.cpp`); el resto está en módulos que el Makefile lista pero no vimos (`allObjectivesCompleted` y `autoCompleteAllObjectives` están en `objectives.cpp`).
- **Declara aunque las define `game.cpp`:** `showMissionInformation`.
- **No declara:** `doGame`, `gameover`, `newGame`, `doGameStuff`, `showInGameOptions` y `beamInPlayer` (las define `game.cpp`; `main.cpp` toma las dos primeras de `main.h`), ni `setGameScreenSize` ni `resetPlayerAmmo` (`game.cpp` las declara por su cuenta).
- **Globales que expone:** `audio`, `config`, `engine`, `graphics`, `map`, `replayData`, `player`, `game`, `gameData`, `weapon[MAX_WEAPONS]`.

### `Makefile.windows` — modificado
**Función:** compilación en Windows con MSYS2 y g++ (SDL2). Está en la raíz del proyecto (busca `src/%.cpp`).

- **Objetivos:** `all` (por defecto: `blobwars.exe` y traducciones `.mo`), `clean`, `pak.exe`, `mapeditor.exe`, `blobwars.pak` y `buildpak`.
- **Variables clave:** `USEPAK ?= 1` (por defecto se compila con pak), `RELEASE ?= 1`, `VERSION = 2.00`, `DATA = data gfx sound music`, `-g -O0`, `-Dmain=SDL_main`, `-DPAKLOCATION=".\\"`. No define `DEBUG`, así que los bloques `#if DEBUG` quedan apagados.
- **Objetos:** `OBJS` (clases `C*.o` y módulos), `GAMEOBJS` = `OBJS` + `main.o`, `MAPOBJS` (editor), `PAKOBJS` = `CFileData.o pak.o`.
- **Regla original:** `%.o: src/%.cpp src/%.h src/defs.h src/defines.h src/headers.h`. Cada `.o` dependía solo de su `.cpp`, su `.h` y esas tres cabeceras.
- **Modificado:** `CXXFLAGS += -MMD -MP` (el compilador genera un `.d` por objeto con las cabeceras reales) y `-include $(GAMEOBJS:.o=.d)` **al final del archivo** (si va antes de `all:`, la primera regla pasa a ser la de un `.o` y `make` sin argumentos deja de compilar todo). `clean` borra también `*.d`. La primera compilación tras el cambio tiene que ser limpia.
- **Pak:** `blobwars.pak` depende de `pak.exe` y se arma con `./pak data gfx sound music blobwars.pak`. `clean` también borra el pak.
- **Regla `%.o`:** exige que exista `src/<nombre>.h` para cada `.cpp`, y también `src/defs.h` y `src/defines.h`.

### `compilar.bat` y `clean_and_compile.bat`
**Función:** compilan desde la raíz con MSYS2 (`...\Proyectos\MSYS2\ucrt64\bin` y `usr\bin` se agregan al `PATH` dentro del script).

- **`compilar.bat` (original):** `make -f Makefile.windows`, borra `blobwars.pak` y corre `make ... buildpak`; si algo falla avisa y hace pausa.
- **`clean_and_compile.bat` (nuevo):** igual, pero antes hace `cd /d "%~dp0"` y `make ... clean`; si el `clean` falla, borra todos los `.o` de la carpeta y las subcarpetas.
- **Pak:** ambos regeneran `blobwars.pak`.

---

## Archivos a pedir en cada sesión

No se suben todos los archivos en cada sesión, y este registro es un mapa del proyecto, no un reemplazo del código.

- Antes de proponer o hacer un cambio, conviene identificar qué archivos hacen falta (con el índice) y pedir los que no estén en la sesión.
- Un archivo marcado como "Visto" que no se subió en la sesión actual se trata como no disponible: no se edita a partir de su entrada.
- Cuando un cambio toca una cabecera de clase o una constante compartida, se piden también los archivos que la usan.

---

## Por ubicar

- **`CFileData.h`:** define `FileData` (nombre, `location`, `cSize`, `fSize`); hace falta su disposición exacta para generar un pak compatible.
- **`items.h`, `resources.h` y `objectives.h`:** sin ver.
- **`items.h`, `weapons.h` y `CWeapon.h`:** sin ver.
- **Barra de estado (`doStatusBar`):** sin ubicar el archivo; muestra el arma actual y podría no contemplar armas nuevas.
- **`defines.h` subido:** coincide con la entrada `data/defines.h` (87 definiciones, LF), pero no se sabe si es la copia de `data/` o la de `src/`, ni si son el mismo archivo.
- **`src/defines.h`:** lo incluye `defs.h`, así que existe; sin ver.
- **`main.h`:** sin ver (declara `doGame`, `gameover`, `checkStartCutscene`, `loadResources`, etc.).

---

## Trabajo en curso

| Mejora | Estado |
|---|---|
| Explosiones que detonan minas | Hecho en `traps.cpp` + `explosions.cpp`. Probado en pantalla: funciona |
| Cámara adelantada hacia el cursor | Hecho en `bullets.cpp` + `CEngine.cpp`. Probado en pantalla: funciona |
| Temblor de pantalla por explosión | Hecho en `explosions.cpp` + `CEngine.cpp`. Probado en pantalla: funciona |
| Opciones de menú (mouse sí/no, previsualización, estela, cámara, temblor) | Variables nuevas en `Game` (`CGame.h` + `CGame.cpp`); `bullets.cpp` y `explosions.cpp` leen de `game`. Guardado en `init.cpp` (línea 3 del `config`); submenú Gameplay en `options.cpp`, `optionWidgets` y `gameplayWidgets`. Probado en pantalla: funciona |
| Salud de enemigos | `defEnemies` con salud base 2 o 3 en Blobs, 4 a 6 en Eye Droids y Spider Blob sin cambio (15); `enemies.cpp` con rangos por bonus (+2 veterano, +4 sargento) y tope de salud base 3. Probado en pantalla: funciona |
| Mira con arco de recarga y balas restantes | Sin empezar |
| Mejoras de IA de enemigos | Propuestas 1 y 2 completadas; granadas estratégicas implementadas; resto pendiente |
| Granadas enemigas con carga | Hecho en `enemies.cpp` + `bullets.cpp` (ver sus entradas). Sin compilar ni probar. Verificado con `data/weapons`: `Alien Grenades` tiene vida 240 y el plan usa como tope de vuelo el 85 % (204 fotogramas); un vuelo de 720 px dura unos 100 fotogramas |
| Sin recarga en granadas y cohetes de Bob | Hecho en `player.cpp`. Sin compilar ni probar |
| Congelamiento al morir | Resuelto tras `clean_and_compile.bat`. Causa probable: `Makefile.windows` no tenía `CGame.h` como dependencia de los demás `.o` |
| Dependencias automáticas en el Makefile | Hecho en `Makefile.windows`. Sin probar compilando |
| Correcciones en `spawnPoints.cpp` | Puertas, límites de `SPW_ITEM` y jefe nulo. Sin compilar ni probar |
| Balance de armas y armas de enemigos soltables | Mejora 1 entregada: `data/weapons` con las armas 21 y 22 de Bob (`dy 0`) y cargador de plasma 18 en `weapons.cpp`. Mejora 2 entregada: estadísticas de `Game` con `MAX_WEAPONS` posiciones en `CGame.h` y `CGame.cpp`. Ambas sin compilar ni probar y sin efecto visible hasta que un ítem dé las armas. Pendiente en orden: ítems 15 y 16 (`defItems`, `mainSprites` con íconos reutilizados, mapeo en `items.cpp`), munición temporal en `player.cpp`, drops por rango en `enemies.cpp`, lanzallamas |

### Propuestas de IA de enemigos

Base actual (`enemies.cpp`): percepción gradual, oído del disparo, memoria del último punto visto, turnos de ataque, aviso antes de disparar, ráfagas, puntería con anticipación y escuadras con sargento.

| # | Propuesta | Estado actual | Riesgo | Archivos a pedir |
|---|---|---|---|---|
| 1 | ~~Línea de visión más confiable~~ | ~~`hasClearShot` lanza el rayo desde la esquina superior izquierda del enemigo hacia la de Bob y pasa `player.height` como ancho y `player.width` como alto (orden confirmado en `CCollision.cpp`). Propuesta: partir del centro del enemigo, apuntar al centro de Bob y pasar ancho y alto en el orden correcto~~ | Bajo | ~~`enemies.cpp`~~ |
| 2 | ~~Distancia y separación~~ | ~~Los enemigos alertados van a la `x` de Bob y solo se detienen mientras avisan o disparan una ráfaga. Propuesta: distancia preferida según el arma (pistola cerca, granadas y cohetes lejos), detenerse al llegar, retroceder si Bob se acerca y un corrimiento por enemigo para que no se apilen~~ | Medio; cambia la sensación del combate, con valores ajustables | ~~`enemies.cpp`, `data/weapons`~~ |
| 3 | Persecución vertical | Los que caminan se detienen en el borde de una plataforma si Bob está abajo y saltan al azar (1 % por fotograma) si está más arriba. Propuesta: bajar cuando Bob está a cierta distancia por debajo y el aterrizaje es seguro (sin líquido ni lava), y saltar hacia arriba con más criterio | Medio; requiere revisar el suelo de abajo | `enemies.cpp`, `map.cpp` |
| 4 | Reacción a lo que hace Bob | Hoy solo reaccionan al oír un disparo. Propuesta: salto de esquive con baja probabilidad ante una bala de Bob que viene de frente, y alejarse de una granada suya por explotar (radio 50) | Bajo a medio; frustra si es muy frecuente | `enemies.cpp`, `bullets.cpp` |
| 5 | Voladores y nadadores | Los Eye Droids y el Aqua Blob van directo a la posición de Bob y se detienen para disparar. Propuesta: mantener altura y distancia con un leve vaivén | Bajo | `enemies.cpp` |

### Mejoras implementadas (2026-09-29)

**Propuesta 1 - Línea de visión más confiable:** Corregido `hasClearShot` para lanzar rayo desde el centro del enemigo hacia el centro de Bob, en lugar de esquina a esquina.

**Propuesta 2 - Distancia y separación:** Implementado:
- Distancia preferida según arma: 150px (pistola/ametralladora), 300px (láser/escopeta), 450px (granadas/cohetes)
- Enemigos alertados se detienen a su distancia preferida, se alejan si están muy cerca, se acercan si están muy lejos
- Separación de 64px entre enemigos para evitar apilamiento

**Granadas estratégicas:** Implementado:
- Cooldown de 120 frames (~2s) entre granadas del mismo enemigo
- Verificación de seguridad: no lanza si está a menos de 150px del jugador o si hay aliados cercanos
- Registro de posición de granada lanzada
- Movimiento para mantener 150px de distancia de granadas activas (propias y de aliados)
- Los enemigos no corren hacia sus propias granadas; esperan a que exploten antes de lanzar otra

### Propuestas nuevas de IA (2026-09-29)

**Origen:** enemigos con más conciencia del terreno (que no caigan en limo, lava ni por precipicios, que puedan saltar y cubrirse detrás del terreno) y sin efecto altura: enemigos en escaleras o plataformas cercanas por encima de Bob que no lo ven. **Base del análisis:** revisión de los adjuntos `enemies.cpp`, `CMap.h`, `CMap.cpp`, `map.cpp`, `entities.cpp`, `CCollision.cpp` y `defs.h` (2026-09-29).

| # | Propuesta | Estado actual (según el registro) | Riesgo | Archivos a pedir |
|---|---|---|---|---|
| 6 | Base común de terreno (prerrequisito de 7, 8 y 9) | Ya hay consultas puntuales: `doAI` comprueba solidez bajo el borde delantero actual y `isGoodEscortSpot` busca suelo y evita líquidos para colocar escoltas; no existe una comprobación reutilizable del paso o aterrizaje seguro. Propuesta: helper local en `enemies.cpp` para validar coordenadas, consultar tiles y buscar suelo hasta un tope | Bajo (solo lectura del mapa) | `enemies.cpp`, `CMap.h`, `CMap.cpp`, `defs.h` |
| 7 | Cuidado con precipicios y líquidos peligrosos | La comprobación actual no evalúa el paso futuro ni el aterrizaje. En limo o lava mueren (`doEnemies`); el agua también se vuelve peligrosa en niveles de hielo. Propuesta: mirar el borde delantero y el suelo de apoyo; si no hay suelo o la caída prevista termina en un tile mortal, frenar o cambiar el destino. Saltar un hueco solo con aterrizaje comprobado. Voladores y nadadores requieren reglas distintas. Los golpes y explosiones de Bob siguen pudiendo empujarlos | Medio. Efecto secundario: menos enemigos muertos por limo o lava (¿afecta al puntaje?) | `enemies.cpp`, `CMap.h`, `CMap.cpp`, `map.cpp`, `entities.cpp` |
| 8 | Cobertura: saltar y esconderse detrás del terreno | El que espera su turno de ataque se queda a la vista; herido no huye ni se cubre (solo hay `confused` en escuadras sin sargento). Propuesta por fases: (a) los que esperan turno (máximo `1 + skill` atacan a la vez) buscan un punto de cobertura cercano (a unos 6 tiles, alcanzable sin riesgos) sin línea de tiro a Bob y esperan ahí; (b) el que recibe el turno asoma a una posición de tiro a 1 o 2 tiles, dispara su ráfaga y vuelve; (c) los heridos (salud baja) se cubren y no atacan un rato; (d) saltar para llegar a la cobertura solo con `ENT_JUMPS` y aterrizaje seguro. Escala con `skill` y con el rango. No aplica a voladores, `ENT_ALWAYSCHASE` ni jefes. Aprovecha lo que ya existe: turnos de ataque y aviso previo | Alto (máquina de estados nueva; choca con atasco, escuadras y turnos) | `enemies.cpp` y los de la 6 |
| 9 | Efecto altura (high ground) | Ver el análisis de abajo. Propuestas: (a) diagnosticar con un registro temporal; (b) ventana vertical de percepción según la línea libre y no un corte fijo; (c) visión desde el ojo del enemigo a varios puntos del cuerpo de Bob; (d) alcance de disparo vertical o reposicionarse | Bajo a medio para (a) a (c); medio para (d) | `enemies.cpp`; para (d) también `data/weapons` y `bullets.cpp` |
| 10 | Búsqueda activa tras perder de vista | Van al último punto visto, retienen 120 o 360 fotogramas y luego la conciencia baja. Propuesta: al llegar, mirar hacia ambos lados y avanzar unos tiles en la dirección que llevaba Bob (guardar su `dx` al perderlo de vista); los de una escuadra se reparten a distintas distancias | Bajo a medio | `enemies.cpp` |
| 11 | Conciencia de la mira y la recarga de Bob | Los enemigos no saben hacia dónde apunta Bob (`aimDirX/Y`, en `bullets.cpp`) ni si recarga (`playerReloading`, en `player.cpp`). Propuesta: (a) con Bob apuntándoles y disparando, los que tienen cobertura cerca (8) se cubren; (b) con Bob recargando, los que tienen turno se adelantan o disparan sin esperar. Hay que exponer un getter de la dirección de puntería | Medio; puede frustrar si reaccionan de más (mismo criterio que la 4) | `enemies.cpp`, `bullets.cpp`, `player.cpp` |

**Análisis del efecto altura**

Síntoma: enemigos en escaleras o plataformas cercanas y por encima de Bob no lo detectan, aunque los tiles estén próximos. Causas posibles:

- **A. Ventana vertical:** `senseSurroundings` solo percibe con diferencia en y menor a 100 px. Por encima de eso el enemigo queda ciego aunque la línea esté libre. El oído (alerta 45) admite 220 px en y.
- **B. Línea de tiro:** el adjunto revisado ya traza `hasClearShot` desde el centro del enemigo al centro de Bob. Si el problema persiste, comprobar en ejecución qué tile bloquea el rayo en escaleras; no se confirmó la hipótesis anterior de esquina a esquina ni de ancho/alto invertidos para esta versión.
- **C. Disparo:** los enemigos con arma recta disparan casi en horizontal (`getAimDY` topa en ±3). Un enemigo en altura vería a Bob pero no lo alcanzaría.

Plan sugerido:

1. Registro temporal para cada enemigo a menos de 400 px en x, con diferencia en y, resultado de `hasClearShot` y la condición que cortó la percepción.
2. Probar en el juego la versión adjunta de la propuesta 1 (rayo de centro a centro); si aún falla, registrar qué tile bloquea la visión.
3. Si falta, ventana vertical dependiente de la línea libre: hasta unos 6 tiles (valor ajustable).
4. Visión multipunto: rayos del ojo del enemigo a la cabeza, el centro y los pies de Bob.
5. Disparo: ampliar el tope de `aimDY` para enemigos por encima de Bob o hacer que se reposicionen.

**Orden sugerido actualizado**

1. Diagnóstico de visión (paso 1 del plan) y propuesta 1.
2. Propuesta 9 (b y c) si el diagnóstico lo pide.
3. Propuesta 6 (base de terreno).
4. Propuesta 7 (precipicios y líquidos).
5. Propuesta 9 (d) junto con la 3.
6. Propuestas 2 y 4, con la 11.
7. Propuesta 8 (cobertura), la más grande; por fases.
8. Propuestas 10 y 5, opcionales.

**Notas transversales**

- **Rendimiento:** `senseSurroundings` corre cada fotograma por enemigo; con varios rayos conviene evaluar cada 3 fotogramas, con desfase por enemigo, y guardar el resultado en `EnemyAIState`.
- **Valores ajustables:** constantes al inicio de `enemies.cpp`, escaladas con `game.skill`.
- **Opción de menú:** una casilla "IA táctica" en el submenú Gameplay (variable nueva en `Game`, por defecto 1) implicaría tocar `CGame.h`, `CGame.cpp`, `options.cpp`, `gameplayWidgets` e `init.cpp`.
- **Archivo grande:** casi todo vive en `enemies.cpp` (2094 líneas, CRLF).

---

## Notas técnicas

- **Compilación en Windows:** con `compilar.bat` (make y pak) o `clean_and_compile.bat` (make clean, make y pak); ver `Makefile.windows`. La regla original **no** rastreaba `CGame.h` ni otras cabeceras de clase, así que tras cambiar una había que borrar los `.o`; con `-MMD -MP` ya se rastrean (la primera compilación tras el cambio debe ser limpia). Como `USEPAK` vale 1 por defecto, `loadData` lee solo del pak, y ambos scripts lo regeneran en cada compilación.
- **`debug()` apagado:** el Makefile no define `DEBUG` y `defs.h` deja `debug(x)` vacío sin él. Para diagnosticar sin `DEBUG` sirve escribir a un archivo con `fopen`. Tampoco se ve `stderr` (el ejecutable se enlaza con `-mwindows`), por eso los `fprintf(stderr, ...)` de `CPak.cpp` no se notan.
- **Saltos de línea:** CRLF en `main.cpp`, `CGame.cpp`, `CGame.h`, `player.cpp`, `bullets.cpp`, `enemies.cpp`, `weapons.cpp`, `CWeapon.cpp`, `spawnPoints.cpp`, `spawnPoints.h`, `CEngine.cpp`, `init.cpp`, `defs.h`, `game.h`, `map.cpp`, `CGraphics.h`, `CEntity.cpp`, `CSprite.h` y `CSprite.cpp`; LF en `traps.cpp`, `game.cpp`, `CPak.cpp`, `options.cpp`, `CGraphics.cpp`, `Makefile.windows` y los archivos de `data/`; `items.cpp` es LF y UTF-8. Al editar hay que conservar los de cada archivo.
- La cámara y el temblor se suman en `Engine::setPlayerPosition`; cualquier pantalla que la llame con límites mayores o iguales a cero los recibe.
