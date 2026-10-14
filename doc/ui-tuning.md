# UI tuning

Edit `ui.cfg` next to the game, in its working directory. It is read from the
filesystem, even with USEPAK enabled. No compilation or resource repack is needed.
Restart the game or return to the title screen to create a new UI renderer and
load changes. The standalone UI preview reads the same file when started.

The initial values preserve the Sprint 3 title menu appearance. This configuration
applies to the modern renderer, title menu, difficulty, save slots and Options,
Keyboard, Joystick, Gameplay and Cheats screens. The gameplay HUD has not been
migrated yet.

## Common adjustments

| Setting | Meaning |
| --- | --- |
| PANEL_WIDTH | Maximum title panel width; shrinks to fit the screen |
| PANEL_MARGIN | Minimum horizontal screen margin |
| PANEL_PADDING | Space on either side inside the panel |
| PANEL_PADDING_BOTTOM | Space below the last button |
| BUTTON_HEIGHT / BUTTON_GAP | Title button height and separation |
| HEADING_TOP / HEADING_HEIGHT / HEADING_GAP | Header spacing and height |
| LABEL_HEIGHT | Height of save-slot information rows |
| PANEL_MIN_TOP | Preferred minimum vertical position |
| PANEL_POSITION | Fraction of free vertical space, from 0 to 1 |
| PANEL_BOTTOM_MARGIN | Bottom screen clearance, overriding preferred position |
| FONT_TITLE / FONT_BODY / FONT_SMALL | Integer font sizes |
| CONTROL_PADDING / CONTROL_HEIGHT | Shared renderer controls |
| MARGIN / GAP | Shared preview and options layout spacing |
| OPTIONS_WIDTH | Maximum settings width; shrinks to fit the screen |
| OPTIONS_ROW_HEIGHT / OPTIONS_ROW_GAP | Minimum settings control height and row spacing |
| MODAL_MESSAGE_GAP | Space between confirmation text and buttons |
| *_COLOR | Four integers: red green blue alpha, each 0..255 |

Dimensions use UI units (pixels at scale 1). Lines use `KEY = value`; `#` starts
a comment. Partial files are supported: omitted settings retain defaults.
Unknown keys, malformed values and out-of-range values are reported on stderr
with their line number and ignored. Missing files use the complete default theme.
If a key appears more than once, its last valid value wins.

Defaults and accepted ranges live in `src/uiTheme.def`, shared by the default
theme and parser. Increase button or heading heights when increasing font sizes;
individually valid settings can still produce clipped content if combined poorly.

Run `./build.ps1 -UI -VerifyUI` to check configuration loading, defaults, invalid
input handling, layout and preview rendering across resolutions and UI scales.
