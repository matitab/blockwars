# Modern options screens

`showOptions()`, `showKeyConfig()`, `showJoystickConfig()`, `showGameplayConfig()`
and `showCheatConfig()` retain their existing widget files, variable bindings and
configuration saving. `OptionsScreen` in `src/options.cpp` adapts those widgets to
the shared `UI::OptionsView` and native SDL renderer.

- Binary settings use checkboxes; multiple choices use segmented controls.
- Sliders display the existing numeric value and support mouse dragging.
- Keyboard and joystick bindings use the engine's existing capture path,
  including cancellation, clearing joystick bindings and restoring key defaults.
- Keyboard/joystick navigation automatically scrolls to the focused control;
  the mouse wheel scrolls only overflowing lists. Resizing resets the offset
  when the complete list fits. Pointer input is suspended during capture.
- Menus show the system cursor and restore its previous visibility on exit,
  including when returning from a submenu or opening Options during gameplay.
- Disabled audio/joystick controls and cheat visibility follow the existing
  engine conditions. Audio, fullscreen and brightness effects retain their
  original handlers.
- The layout wraps labels and stacks controls in narrow views. Legacy keyboard
  instruction labels are omitted from the new presentation.

`ui.cfg` supplies colors, fonts and spacing, with `OPTIONS_WIDTH`,
`OPTIONS_ROW_HEIGHT` and `OPTIONS_ROW_GAP` controlling the settings layout.

`./build.ps1 -UI -VerifyUI` builds the game and exercises the view with real
English and Spanish widget definitions at 1024x768, 1280x720, 1360x768 and
1920x1080, at scales 1 and 1.5. It checks text fit, focus visibility, mouse
activation, disabled controls and slider limits, and writes screenshots to
`.build/ui/options-*.png`. Hardware joystick capture and OS gamma behavior
require an interactive run; the rendering tests do not emulate those devices.
