/*
Copyright (C) 2004-2011 Parallel Realities
Copyright (C) 2011-2015 Perpendicular Dimensions

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "options.h"

#include "uiOptionsView.h"
#include "uiMenuCursor.h"
#include <sstream>

namespace {
std::string optionsFont() {
#if USEPAK
    return std::string(engine.userHomeDirectory) + "font.ttf";
#else
    return "data/vera.ttf";
#endif
}

// The engine continues to own navigation, variable bindings and input capture.
class OptionsScreen {
    UI::MenuCursor menuCursor;
    UI::Renderer ui;
    UI::OptionsView view;
    std::string title;
    std::vector<Widget*> widgets;
public:
    explicit OptionsScreen(const char *title) : ui(graphics.renderer, optionsFont().c_str()), title(title) {
        resetPointer();
    }
    void resetPointer() {
        view = UI::OptionsView();
        view.suspendPointer((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) != 0);
    }
    void present() {
        widgets.clear();
        std::vector<UI::OptionItem> items;
        for (Widget *w = (Widget*)engine.widgetList.getHead()->next; w; w = (Widget*)w->next) {
            w->changed = false;
            if (!w->visible || !w->value || w->type == WG_LABEL) continue;
            UI::OptionItem item;
            item.label = w->label;
            item.value = *w->value;
            item.minimum = w->min;
            item.maximum = w->max;
            item.enabled = w->enabled;
            item.focused = w == engine.highlightedWidget;
            if (w->type == WG_RADIO) {
                item.type = UI::OptionRadio;
                std::istringstream choices(w->options);
                std::string choice;
                while (std::getline(choices, choice, '|')) item.choices.push_back(choice);
            } else if (w->type == WG_SLIDER || w->type == WG_SMOOTH_SLIDER) {
                item.type = UI::OptionSlider;
            } else if (w->type == WG_KEYBOARD || w->type == WG_JOYPAD) {
                item.type = UI::OptionBinding;
                item.capturing = item.focused && (engine.waitForKey || engine.waitForButton);
                if (w->type == WG_KEYBOARD) item.binding = Keyboard::translateKey(item.value);
                else if (item.value < -2) item.binding = "...";
                else if (item.value == -2) item.binding = _("N/A");
                else {
                    char button[64];
                    std::snprintf(button, sizeof button, _("Button #%d"), item.value);
                    item.binding = button;
                }
            }
            widgets.push_back(w);
            items.push_back(item);
        }
        int width, height;
        SDL_GetRendererOutputSize(graphics.renderer, &width, &height);
        ui.begin(width, height);
        view.draw(ui, title, items);
        graphics.finishScreenPresent();
    }
    int process() {
        int x, y, width, height;
        bool left = (SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) != 0;
        SDL_GetWindowSize(graphics.window, &width, &height);
        if (engine.waitForKey || engine.waitForButton) {
            view.suspendPointer(left);
            engine.mouseWheel = 0;
            return 0;
        }
        UI::OptionAction action = view.pointer(ui.layout.fromWindow(UI::Point(x, y), width, height), left, engine.mouseWheel);
        engine.mouseWheel = 0;
        int sound = 0;
        if (action.index >= 0 && action.index < int(widgets.size())) {
            Widget *w = widgets[action.index];
            if (w->visible && w->enabled) {
                engine.highlightWidget(w->name);
                if (action.activate) {
                    // Reuse binding capture and button activation in processWidgets.
                    engine.keyState[SDL_SCANCODE_RETURN] = 1;
                } else if (*w->value != action.value) {
                    *w->value = action.value;
                    w->changed = true;
                    sound = 1;
                }
            }
        }
        int keyboardSound = engine.processWidgets();
        return keyboardSound ? keyboardSound : sound;
    }
};
}


void showCheatConfig()
{
	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	OptionsScreen screen(_("Cheats"));

	if (!engine.loadWidgets(_("data/cheatWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/cheatWidgets"));
	}
		
	int done = 0;

	engine.setWidgetVariable("health", &engine.cheatHealth);
	engine.setWidgetVariable("extras", &engine.cheatExtras);
	engine.setWidgetVariable("fuel", &engine.cheatFuel);
	engine.setWidgetVariable("rate", &engine.cheatReload);
	engine.setWidgetVariable("blood", &engine.cheatBlood);
	engine.setWidgetVariable("invulnerable", &engine.cheatInvulnerable);
	engine.setWidgetVariable("speed", &engine.cheatSpeed);
	engine.setWidgetVariable("levels", &engine.cheatLevels);
	engine.setWidgetVariable("skip", &engine.cheatSkipLevel);
	engine.setWidgetVariable("confirm", &done);

	engine.flushInput();
	engine.clearInput();
	
	int menuSound = -1;

	while (!done)
	{
		screen.present();
		
		if (menuSound)
			audio.playMenuSound(menuSound);

		engine.getInput();
		// Capturing keyboard bindings temporarily stores negative scancodes.
		if (!engine.waitForKey && !engine.waitForButton)
			config.populate();

		menuSound = screen.process();


		if (engine.keyState[SDL_SCANCODE_ESCAPE])
		{
			engine.clearInput();
			engine.flushInput();
			done = 1;
		}

		SDL_Delay(16);
	}
	
	audio.playMenuSound(2);

	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/optionWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/optionWidgets"));
	}
	
	engine.highlightWidget("cheats");
}

void showKeyConfig()
{
	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/keyboardWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/keyboardWidgets"));
	}
		
	OptionsScreen screen(_("Keyboard"));

	int done = 0;
	int defaults = 0;

	engine.setWidgetVariable("left", &config.keyboard.control[CONTROL::LEFT]);
	engine.setWidgetVariable("right", &config.keyboard.control[CONTROL::RIGHT]);
	engine.setWidgetVariable("down", &config.keyboard.control[CONTROL::DOWN]);
	engine.setWidgetVariable("fire", &config.keyboard.control[CONTROL::FIRE]);
	engine.setWidgetVariable("jump", &config.keyboard.control[CONTROL::JUMP]);
	engine.setWidgetVariable("pause", &config.keyboard.control[CONTROL::PAUSE]);

	engine.setWidgetVariable("jetpack", &config.keyboard.control[CONTROL::JETPACK]);
	engine.setWidgetVariable("map", &config.keyboard.control[CONTROL::MAP]);

	engine.setWidgetVariable("defaults", &defaults);
	engine.setWidgetVariable("confirm", &done);

	engine.flushInput();
	engine.clearInput();
	
	int menuSound = -1;
	
	engine.allowJoypad = false;

	while (!done)
	{
		screen.present();
		
		if (menuSound)
		{
			audio.playMenuSound(menuSound);
		}

		engine.getInput();
		// Capturing keyboard bindings temporarily stores negative scancodes.
		if (!engine.waitForKey && !engine.waitForButton)
			config.populate();

		menuSound = screen.process();
		
		if (defaults)
		{
			config.restoreKeyDefaults();
			defaults = 0;
		}

		if (engine.keyState[SDL_SCANCODE_ESCAPE])
		{
			engine.clearInput();
			engine.flushInput();
			done = 1;
		}

		SDL_Delay(16);
	}
	
	engine.allowJoypad = true;
	
	config.saveKeyConfig();

	audio.playMenuSound(2);

	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/optionWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/optionWidgets"));
	}
	
	engine.highlightWidget("keys");
}

void showJoystickConfig()
{
	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/joystickWidgets")))
		graphics.showErrorAndExit(ERR_FILE, _("data/joystickWidgets"));
		
	OptionsScreen screen(_("Joystick"));

	int done = 0;
	int sensitivity = (config.joystick.sensitivity / 100);

	engine.setWidgetVariable("left", &config.joystick.control[CONTROL::LEFT]);
	engine.setWidgetVariable("right", &config.joystick.control[CONTROL::RIGHT]);
	engine.setWidgetVariable("up", &config.joystick.control[CONTROL::UP]);
	engine.setWidgetVariable("down", &config.joystick.control[CONTROL::DOWN]);
	engine.setWidgetVariable("fire", &config.joystick.control[CONTROL::FIRE]);
	engine.setWidgetVariable("jump", &config.joystick.control[CONTROL::JUMP]);
	engine.setWidgetVariable("pause", &config.joystick.control[CONTROL::PAUSE]);

	engine.setWidgetVariable("jetpack", &config.joystick.control[CONTROL::JETPACK]);
	engine.setWidgetVariable("map", &config.joystick.control[CONTROL::MAP]);
	
	engine.setWidgetVariable("sensitivity", &sensitivity);

	engine.setWidgetVariable("confirm", &done);

	engine.flushInput();
	engine.clearInput();
	
	int menuSound = -1;
	
	engine.allowJoypad = false;

	while (!done)
	{
		screen.present();
		
		if (menuSound)
			audio.playMenuSound(menuSound);

		engine.getInput();
		// Capturing keyboard bindings temporarily stores negative scancodes.
		if (!engine.waitForKey && !engine.waitForButton)
			config.populate();

		menuSound = screen.process();

		if (engine.keyState[SDL_SCANCODE_ESCAPE])
		{
			engine.clearInput();
			engine.flushInput();
			done = 1;
		}

		SDL_Delay(16);
	}
	
	config.joystick.sensitivity = (sensitivity * 100);
	
	engine.allowJoypad = true;
	
	config.saveJoystickConfig();

	audio.playMenuSound(2);

	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/optionWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/optionWidgets"));
	}
	
	engine.highlightWidget("joysticks");
}

void showGameplayConfig()
{
	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/gameplayWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/gameplayWidgets"));
	}

	OptionsScreen screen(_("Gameplay"));

	int done = 0;

	engine.setWidgetVariable("mouseaim", &game.mouseAim);
	engine.setWidgetVariable("trail", &game.bulletTrail);
	engine.setWidgetVariable("preview", &game.grenadePreview);
	engine.setWidgetVariable("camlead", &game.cameraLead);
	engine.setWidgetVariable("shake", &game.screenShake);
	engine.setWidgetVariable("confirm", &done);

	engine.flushInput();
	engine.clearInput();

	int menuSound = -1;

	while (!done)
	{
		screen.present();

		if (menuSound)
			audio.playMenuSound(menuSound);

		engine.getInput();
		// Capturing keyboard bindings temporarily stores negative scancodes.
		if (!engine.waitForKey && !engine.waitForButton)
			config.populate();

		menuSound = screen.process();


		if (engine.keyState[SDL_SCANCODE_ESCAPE])
		{
			engine.clearInput();
			engine.flushInput();
			done = 1;
		}

		SDL_Delay(16);
	}

	audio.playMenuSound(2);

	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/optionWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/optionWidgets"));
	}

	engine.highlightWidget("gameplay");
}

void showOptions()
{
	float brightness;

	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);

	if (!engine.loadWidgets(_("data/optionWidgets")))
	{
		graphics.showErrorAndExit(ERR_FILE, _("data/optionWidgets"));
	}
		
	OptionsScreen screen(_("Options"));

	int done = 0;
	int joysticks = 0;
	int cheats = 0;
	int keys = 0;
	int gameplay = 0;

	engine.setWidgetVariable("fullscreen", &engine.fullScreen);
	engine.setWidgetVariable("soundvol", &game.soundVol);
	engine.setWidgetVariable("musicvol", &game.musicVol);
	engine.setWidgetVariable("output", &game.output);
	engine.setWidgetVariable("autosave", &game.autoSave);
	engine.setWidgetVariable("gamma", &game.brightness);
	engine.setWidgetVariable("gore", &game.gore);
	engine.setWidgetVariable("keys", &keys);
	engine.setWidgetVariable("joysticks", &joysticks);
	engine.setWidgetVariable("cheats", &cheats);
	engine.setWidgetVariable("gameplay", &gameplay);
	engine.setWidgetVariable("confirm", &done);

	if (!engine.useAudio)
	{
		engine.enableWidget("soundvol", false);
		engine.enableWidget("musicvol", false);
		engine.enableWidget("output", false);
	}

	if (SDL_NumJoysticks() == 0)
	{
		engine.enableWidget("joysticks", false);
	}

	engine.showWidget("cheats", engine.cheats);

	engine.flushInput();
	engine.clearInput();
	
	int menuSound = -1;

	while (!done)
	{
		screen.present();
		
		if (menuSound)
			audio.playMenuSound(menuSound);

		engine.getInput();
		// Capturing keyboard bindings temporarily stores negative scancodes.
		if (!engine.waitForKey && !engine.waitForButton)
			config.populate();

		if (engine.compareLastKeyInputs())
		{
			if (engine.cheats)
			{
				audio.playSound(SND_CHEAT, CH_ANY);
				engine.clearCheatVars();
			}

			engine.showWidget("cheats", engine.cheats);
			screen.resetPointer();
			screen.present();
		}
		
		menuSound = screen.process();

		if (menuSound)
		{
			if (engine.widgetChanged("soundvol"))
				audio.setSoundVolume(game.soundVol);

			if (engine.widgetChanged("musicvol"))
				audio.setMusicVolume(game.musicVol);

			if (engine.widgetChanged("fullscreen"))
				SDL_SetWindowFullscreen(graphics.window, engine.fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

			if (engine.widgetChanged("gamma"))
			{
				brightness = game.brightness;
				if (brightness > 0) {
					brightness /= 10;
					uint16_t ramp[256];
					SDL_CalculateGammaRamp(brightness, ramp);
					SDL_SetWindowGammaRamp(graphics.window, ramp, ramp, ramp);
				}
			}
			
			if ((joysticks) || (cheats) || (keys) || (gameplay))
			{
				audio.playMenuSound(2);
				menuSound = 0;

				if (joysticks)
				{
					showJoystickConfig();
				}
				else if (cheats)
				{
					showCheatConfig();
				}
				else if (keys)
				{
					showKeyConfig();
				}
				else if (gameplay)
				{
					showGameplayConfig();
				}
				
				joysticks = keys = cheats = gameplay = 0;

				engine.setWidgetVariable("fullscreen", &engine.fullScreen);
				engine.setWidgetVariable("soundvol", &game.soundVol);
				engine.setWidgetVariable("musicvol", &game.musicVol);
				engine.setWidgetVariable("output", &game.output);
				engine.setWidgetVariable("autosave", &game.autoSave);
				engine.setWidgetVariable("gamma", &game.brightness);
				engine.setWidgetVariable("gore", &game.gore);
				engine.setWidgetVariable("keys", &keys);
				engine.setWidgetVariable("joysticks", &joysticks);
				engine.setWidgetVariable("cheats", &cheats);
				engine.setWidgetVariable("gameplay", &gameplay);
				engine.setWidgetVariable("confirm", &done);

				if (!engine.useAudio)
				{
					engine.enableWidget("soundvol", false);
					engine.enableWidget("musicvol", false);
					engine.enableWidget("output", false);
				}
				
				if (SDL_NumJoysticks() == 0)
				{
					engine.enableWidget("joysticks", false);
				}
				
				engine.showWidget("cheats", engine.cheats);
				screen.resetPointer();
			}
		}

		if (engine.keyState[SDL_SCANCODE_ESCAPE])
		{
			engine.clearInput();
			engine.flushInput();
			done = 1;
		}

		SDL_Delay(16);
	}
	
	
	if (audio.output  != game.output)
	{
		audio.output = game.output;
		
		if (!audio.output)
		{
			audio.stopMusic();
			audio.stopAmbiance();
		}
		else
		{
			audio.playMusic();
			audio.playAmbiance();
		}
	}

	audio.playMenuSound(2);

	SDL_FillRect(graphics.screen, NULL, graphics.black);
	graphics.delay(500);
}
