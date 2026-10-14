# UI: sprint 2

Base compartida implementada y pantalla de verificacion independiente.
El menu principal del juego se migra en el sprint 3.

## Modulos

- `src/uiTheme.h`: colores, roles de texto, medidas y revision del tema.
- `src/uiLayout.h`: rectangulos, filas, contenido centrado y conversion entre
  coordenadas de ventana, salida del renderer y UI. Sin dependencia de SDL.
- `src/uiRenderer.*`: texto UTF-8 antialias con SDL_ttf, medicion real y cache
  limitada a 256 texturas. La clave incluye fuente, tamano, color y revision.
  Botones y controles usan rectangulos del mismo layout para dibujar y detectar clicks.
- `src/uiPreview.*`: pantalla interactiva para comprobar botones, foco,
  slider, checkbox, estados deshabilitados y layout en columnas/apilado.

El tamano de fuente depende del rol y de la escala elegida por el usuario,
no del ancho de la ventana. El texto se rasteriza directamente al tamano final,
sin ampliar superficies pequenas. Labels largos se acortan por caracteres UTF-8
con puntos suspensivos y clipping; el metodo devuelve si el texto original cabe.
`measure()` permite elegir un layout alternativo antes de dibujar.

La escala disponible es 0.75 a 1.5; todavia no se guarda en la configuracion del
juego. La pantalla parte de 1360x768, pero calcula el layout de nuevo al cambiar
la ventana y apila los controles si el ancho disponible resulta pequeno.

## Uso

```powershell
.\compilar.bat -UI
.\ui-preview.exe
.\compilar.bat -VerifyUI
```

La preview usa `data/vera.ttf` desde la carpeta del proyecto. Es una herramienta
de desarrollo: sus ajustes no modifican partidas ni opciones del juego.
Tab/flechas cambian el foco, izquierda/derecha ajustan el slider y Space/Enter
activan los controles. El mouse permite click y arrastre del slider. `+`/`-`
cambian la escala y Escape cierra. El mando se integra con la navegacion existente
al migrar la primera pantalla en el sprint 3.

`-VerifyUI` no abre ventanas: usa el renderer de software de SDL y genera
capturas en `.build/ui/`. Prueba 1024x768, 1280x720, 1360x768 y 1920x1080,
cada una en escalas 0.75, 1.0, 1.25 y 1.5.

## Verificacion

- Pruebas de layout: limites, separacion de filas, bordes compartidos y
  conversion del mouse con una salida de doble densidad.
- Medicion real de texto, cambio de tamano de fuente con cache activa y
  truncamiento seguro de texto UTF-8 largo.
- Las 16 combinaciones deben mostrar texto completo y controles dentro de la
  pantalla, tanto en estado inicial como con boton y checkbox activados.
- Comprobacion de pixeles: contenido visible y tonos intermedios de antialias.
- Inspeccion visual de las capturas de referencia y de la ventana pequena
  con escala 1.5. Compilacion Windows correcta; Linux no ejecutado.

## Integracion del sprint 3

`UI::Renderer` recibe el renderer SDL existente y la ruta a la fuente (puede ser
la extraida del pak). Su propietario debe mantener vivos SDL_ttf y el renderer
hasta destruir las texturas/fuentes de UI.

`begin()` inicia una pantalla UI completa: limpia la salida y configura dibujo
en pixeles reales, sin logical size de SDL. Al volver al mundo hay que restaurar
su logical size, viewport y escala. No cambia la camara ni la superficie del juego.
Durante ese pase, mouse y layout se convierten juntos usando el tamano real de
ventana y salida; no aplicar tambien la division antigua por `renderScale`.

Los widgets actuales se conservaran como estado y enlaces a acciones/variables;
el adaptador usara estos metodos de dibujo y medida. La preview no introduce
otro formato de definiciones de pantallas ni reemplaza aun `Engine::processWidgets`.
