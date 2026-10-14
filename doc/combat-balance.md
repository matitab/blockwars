# Balance de combate

Editar `combat.cfg` junto al ejecutable y reiniciar el juego. Ejecutarlo desde
esa carpeta: el archivo se busca en el directorio de trabajo. No hace falta
recompilar ni regenerar `blobwars.pak` para cambiar estos valores.

Se conservan los valores anteriores como punto de partida. Esta configuracion
cubre la IA de enemigos comunes y BioMechs; las armas y la vida base siguen
en `data/weapons` y `data/defEnemies`. No cubre todas las reglas de los jefes.

Formato: `CLAVE = numero`, con espacios alrededor del signo igual.
Los comentarios empiezan con `#`. Tiempos: frames de simulacion, 62 por segundo.
Distancias: pixeles. Probabilidades `CHANCE`: porcentajes. `PERMILLE`: por mil.
Booleanos: 0 o 1. Algunas probabilidades se aplican cada frame, no por ataque.

| Ajuste | Claves principales |
| --- | --- |
| Percepcion | `VISION_RANGE`, `VISION_VERTICAL_RANGE`, `BEHIND_RANGE`, `AWARE_DECAY` |
| Distancia de combate | `DIST_CLOSE`, `DIST_MEDIUM`, `DIST_FAR` |
| Granadas | `GRENADE_COOLDOWN`, `GRENADE_CHARGE_FRAMES`, `GRENADE_SAFE_DISTANCE` |
| Esquivas del droid | `DASH_DODGE_CHANCE`, `DASH_COOLDOWN`, `DROID_MAX_DASHES` |
| Escudos | `DROID_SHIELD_POINTS`, `BIOMECH_SHIELD_POINTS`, `DROID_RECHARGE_FRAMES` |
| Laser | `LASER_CLIP_SIZE`, `LASER_RELOAD_FRAMES`, `LASER_BEAM_TICK` |
| Rangos militares | `VETERAN_*`, `SERGEANT_*`, `MAX_SERGEANTS` |

Por ejemplo, `DASH_DODGE_CHANCE = 35` permite probar una probabilidad del 35%
de esquivar un disparo elegible. El valor original es 100.

La probabilidad de veterano es base + misiones completadas * incremento por
mision + dificultad * incremento por dificultad, limitada por `VETERAN_MAX_CHANCE`.
Los sargentos usan la misma formula desde `SERGEANT_MIN_STAGES`; el incremento
por mision cuenta desde ese umbral. `MACHINEGUN_*` controla el reemplazo de pistolas.

Las claves ausentes usan valores predeterminados. Claves desconocidas, numeros
invalidos y valores fuera de limites se ignoran y se informan en stderr.
Si hay rangos contradictorios (por ejemplo radio minimo mayor que el maximo),
se descarta la configuracion completa y se usan los valores predeterminados.
Los limites y valores de respaldo estan en `src/combatBalance.def`.

Prueba del cargador, con un compilador C++ disponible:

```powershell
g++ -Wall -Wextra -std=c++98 tests/combatBalanceTest.cpp -o combatBalanceTest.exe
.\combatBalanceTest.exe combatBalanceTest.fixture.cfg
```

La prueba comprueba que el archivo distribuido mantiene el balance anterior,
ademas de configuraciones parciales, valores invalidos y recuperacion de errores.
