# Game Over recovery

Game Over always offers restarting the whole current level and returning to the
title screen. Continue from the last checkpoint is disabled for boss missions,
before a checkpoint is available, after its three uses are exhausted, and after
a timed mission runs out of time. These are the existing checkpoint restrictions.

The native pause and Game Over overlays preserve the frozen world frame's aspect
ratio. On wide windows the 4:3 mission view remains centered with side bands.
Confirmation text is separated from the buttons by `MODAL_MESSAGE_GAP` in
`ui.cfg` (20 UI units by default).

Previously, accepting the Game Over screen without an available checkpoint
immediately returned to the title screen. It now opens the choices and focuses
Restart Level when checkpoint continuation is unavailable. Returning to the title
screen requires selecting that action explicitly.

Restart Level clears current mission progress using the same reset as the pause
menu, discards the current map's persistent snapshot and clears the checkpoint
continuation flag. Returning SECTION_GAME then follows the normal resource/map
reload path. Other stages and maps are preserved. Restarting does not save the
reset to disk until the game's normal save flow runs.

`./build.ps1 -UI -VerifyUI` exercises the real menu setup, engine button activation
and mission reset across all eight widget languages, boss/non-boss missions and
checkpoint counts -1, 0, 1 and 3. It runs without a playable game session.
