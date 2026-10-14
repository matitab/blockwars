# Modernizacion de UI: sprint 1

Fecha: 2026-10-14. Alcance: diagnostico del codigo, decisiones tecnicas y
compilacion Windows repetible. La migracion visual comienza en el sprint 2.

## Diagnostico

| Area | Evidencia actual | Consecuencia |
| --- | --- | --- |
| Renderizado | `src/init.cpp`: superficie 1280x720 multiplicada por `HIRES_SCALE=2`; SDL presenta una textura del framebuffer completo | UI y mundo comparten superficie y escalado; cambiar la ventana no recalcula el layout |
| Vista de juego | `src/defs.h` define mundo 800x600 y UI 1280x720; `src/game.cpp` alterna ambas mediante `setGameScreenSize` | No cambiar la vista del mundo para conseguir una UI de 1360x768; afecta camara y lo que se ve del mapa |
| Texto | `src/CGraphics.cpp`: `drawString` tiene fuente de alta resolucion; `getString` usa la fuente pequena | Widgets y titulos construidos como superficies no aprovechan la misma calidad de texto |
| Cache de texto | `drawString` con `SurfaceCache` compara el texto, pero no el estilo ni la escala | El tema y los cambios de escala necesitan invalidacion explicita |
| Controles | `src/CWidget.*`, `src/widgets.cpp`, `Engine::processWidgets` ya comparten controles y foco | Extender esta base; no crear un segundo sistema completo de menus |
| Layout | `drawWidgets` centra con 1280 y convierte `x=-1` en una posicion fija; sliders usan ancho 300; opciones usan cabecera x=320 | Coordenadas y medidas dispersas; el centrado deja de ser adaptable despues del primer dibujo |
| Idiomas | `data/titleWidgets` ubica el primer boton en y=360; `data/es/titleWidgets` en y=240 | La traduccion duplica estructura y layout; corregir una pantalla obliga a mantener varias versiones |
| Entrada | `processWidgets` navega con teclado/mando; eventos de mouse se recogen por separado | El nuevo hit testing debe usar los mismos rectangulos que dibujo y foco |
| Parser | `Engine::loadWidgets` usa buffers fijos y `sscanf` sin comprobar todos los campos | Validar definiciones antes de migrar; mostrar errores con archivo y linea |
| Compilacion | Los batch anteriores fijaban una ruta MSYS2 y borraban el pak; el batch limpio tambien borraba objetos recursivamente | Unificar herramientas y conservar el pak anterior si falla su generacion |

Estos hallazgos vienen de lectura de codigo. No se hizo una revision visual ni
una prueba de navegacion en este sprint. Los problemas de apariencia se deben
confirmar con capturas durante la primera migracion.

## Decisiones

- Referencia de interfaz: 1360x768. Casos de verificacion: 1280x720,
  1360x768, 1920x1080 y ventana 1024x768. En ventanas pequenas, reducir
  espaciado o permitir scroll antes de hacer ilegible el texto.
- Mantener el pixel art y las coordenadas del mundo. Separar la transformacion
  de UI de la camara y del escalado de sprites.
- El viewport de UI se calcula desde el tamano real de salida del renderer.
  Mouse, dibujo y foco comparten una unica conversion de coordenadas,
  incluyendo bandas y cambios de tamano de ventana.
- La ruta de texto de UI mide y renderiza UTF-8 al tamano final. Reutilizar
  SDL_ttf; el cache incluye texto, fuente, tamano, color y revision del tema.
- Un tema define roles tipograficos, colores, espaciado y estados. Cada
  pantalla elige roles; no fija colores y puntos de fuente por separado.
- El layout resuelve filas, columnas y margenes a partir del contenido
  medido. Guardar la intencion de centrado, no reemplazarla por una coordenada.
- Mantener IDs de widgets y enlaces a variables/acciones existentes durante
  la migracion. Teclado, mando y mouse activan la misma accion.
- Separar textos traducidos de layout. Migrar primero titulo y opciones;
  conservar un adaptador para los archivos antiguos mientras siguen en uso.

## Responsabilidades previstas

| Lugar | Responsabilidad |
| --- | --- |
| `src/CGraphics.*` | Renderizado SDL, presentacion y recursos; puente para texto UI |
| `src/uiTheme.*` (nuevo, sprint 2) | Roles y valores compartidos de estilo |
| `src/uiLayout.*` (nuevo, sprint 2) | Medidas, viewport y rectangulos; calculos comprobables sin abrir el juego |
| `src/CWidget.*` y `src/widgets.cpp` | Estado, dibujo y geometria de controles existentes |
| `src/CEngine.cpp` | Lectura de eventos y navegacion comun; conversion coordinada del mouse |
| `src/title.cpp`, `src/options.cpp` | Contenido, enlaces y transiciones de cada pantalla |
| Catalogos de traduccion existentes | Textos traducidos sin posiciones por idioma |

Los nombres de los nuevos modulos son una propuesta concreta para el sprint 2;
solo se agregaran cuando la primera pantalla los necesite.

## Secuencia restante

1. Sprint 2: tema, texto y layout compartidos, pantalla de verificacion,
   pruebas de medidas y conversion de coordenadas.
2. Sprint 3: menu principal, mouse/teclado/mando, textos traducidos separados
   del layout y capturas en la matriz de resoluciones.
3. Sprint 4: opciones, controles comunes y persistencia de escala UI.
4. Sprint 5: HUD, pausa, carga y guardado usando la misma base.
5. Sprint 6: retirar adaptadores reemplazados y documentar nuevas pantallas.

## Compilacion

Desde PowerShell, en la carpeta del proyecto:

```powershell
.\build.ps1
.\build.ps1 -CheckOnly
.\build.ps1 -Rebuild
.\build.ps1 -Repack
.\build.ps1 -MsysRoot C:\msys64
```

`compilar.bat` y `clean_and_compile.bat` son accesos al mismo script; el segundo
fuerza recompilacion sin borrar recursos ni partidas. Ambos devuelven el codigo
de error y funcionan aunque se invoquen desde otra carpeta.

Se busca MSYS2 UCRT64 en `BLOCKWARS_MSYS2`, en una carpeta `MSYS2` junto al
proyecto y en `C:\msys64`; `-MsysRoot` fuerza una ubicacion concreta. El script
verifica compilador, make nativo, SDL2 y las bibliotecas de enlace.

El ejecutable se compila incrementalmente con `Makefile.windows`. El inventario
de recursos detecta nombres, tamanos y fechas, incluyendo altas y bajas; si
cambian, genera un pak temporal y reemplaza el anterior solo al terminar.
El primer uso tambien genera el inventario y el pak. `-Repack` fuerza la
generacion si una herramienta externa preservo fechas y tamanos al editar.
`combat.cfg` queda fuera del pak. El registro del empaquetador y el inventario
se guardan en `.build/`, excluido de Git, junto con el pak anterior como respaldo.

Este comando cubre ejecutable y recursos. Los catalogos gettext `.mo` se
regeneran con el target `all` del Makefile y requieren `msgfmt`; no forman parte
de la compilacion incremental de este script. La instalacion de DLLs y un
paquete distribuible quedan fuera del sprint 1.

## Cierre y verificacion

- Revision de renderizado, texto, widgets, entrada, idiomas y vistas completada.
- Recompilacion completa con `-Rebuild`: correcta; aparecen advertencias
  existentes de truncamiento y variables sin uso. Registro: `.build/rebuild.log`.
- Generacion y reemplazo del pak: correctos; respaldo anterior conservado.
- Ejecucion incremental posterior: ejecutable actualizado y fecha del pak
  sin cambios, comprobada automaticamente.
- Acceso batch desde otra carpeta: correcto.
- Ubicacion de herramientas inexistente: mensaje explicativo y codigo 1.
- Pendiente para sprint 2: prueba visual, medidas de texto y navegacion real;
  este sprint no cambia la apariencia del juego.
