# UI Sprint 3

Sprint 3 connects the SDL-native UI renderer from Sprint 2 to the real title flow.

## Completed

- Added `TitleMenuView`, a native renderer for the title menu widgets.
- Reused the existing widget list, labels, enabled states, highlighted widget, and action variables.
- Kept the original title state machine intact: new game, load game, continue, options, help, quit, save slots, difficulty, and back still flow through `title.cpp`.
- Added mouse hover and click support for the title menu without replacing the keyboard/joystick widget navigation.
- Split `Graphics::updateScreen()` into `prepareScreenPresent()` and `finishScreenPresent()` so native UI can draw between the software framebuffer copy and `SDL_RenderPresent()`.
- Added `uiRenderer.o` and `titleMenuView.o` to the game builds.

## Scope Notes

The title menu now uses crisp SDL_ttf texture text and responsive button layout. The animated title background and logo remain the original game assets.

Options, in-game pause widgets, save/load screens, and HUD are still rendered by the legacy widget drawing path. Those screens can now move over incrementally using the same presentation hook and shared renderer.

## Verification

Ran:

```powershell
.\build.ps1 -UI -VerifyUI
```

This built `blobwars.exe`, `ui-preview.exe`, ran the layout tests, and generated the UI verification PNGs under `.build/ui`.
