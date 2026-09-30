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

#include "player.h"

// Charged grenade throw (defined in bullets.cpp)
void resetGrenadeCharge();
bool handlePlayerGrenade(bool fireHeld);
void drawGrenadeCharge();

// Mouse aiming (defined in bullets.cpp)
void updatePlayerAim(bool keyboardFire);
bool isMouseAiming();
bool takePlayerReloadRequest();
void drawPlayerCrosshair();

// Player ammo / clip system
int playerAmmo = 0;
int playerAmmoMax = 0;
bool playerReloading = false;
int playerReloadTotal = 0;
static const int RELOAD_MULTIPLIER = 3;

// Limited-use weapons: a fixed number of shots per pickup, then back to the pistol
static const int WEAPON_PLAYER_ROCKET = 21; // player rocket launcher (data/weapons)
static const int GRENADE_SHOTS = 3;
static const int ROCKET_SHOTS = 1;

// Shots allowed by the current weapon, or 0 if it is not a limited-use weapon
static int getLimitedShots()
{
	if (player.currentWeapon == &weapon[WP_GRENADES])
		return GRENADE_SHOTS;
	if (player.currentWeapon == &weapon[WEAPON_PLAYER_ROCKET])
		return ROCKET_SHOTS;
	return 0;
}

void resetPlayerAmmo()
{
	playerAmmoMax = player.currentWeapon->clip;
	if (getLimitedShots() > 0)
		playerAmmoMax = getLimitedShots();
	if (playerAmmoMax > 0)
		playerAmmo = playerAmmoMax;
	else
		playerAmmo = 0;
	playerReloading = false;
	playerReloadTotal = 0;
}

void startPlayerReload()
{
	if (playerAmmoMax <= 0)
		return;
	// Limited-use weapons (grenades, rocket launcher) never reload: when the last shot is spent Bob goes back to the pistol
	if (getLimitedShots() > 0)
		return;
	playerReloadTotal = player.currentWeapon->reload * RELOAD_MULTIPLIER;
	if (playerReloadTotal < 30)
		playerReloadTotal = 30;
	player.reload = playerReloadTotal;
	playerReloading = true;
}

void updatePlayerReload()
{
	if (playerReloading && player.reload == 0)
	{
		playerReloading = false;
		playerAmmo = playerAmmoMax;
	}
}

void drawPlayerAmmo()
{
	if (playerAmmoMax <= 0)
		return;

	int x = (int)(player.x - engine.playerPosX);
	int y = (int)(player.y - engine.playerPosY) + 36;
	int w = 26;

	if (playerReloading)
	{
		int fill = (w * player.reload) / playerReloadTotal;
		graphics.drawRect(x - 1, y - 1, w + 2, 5, graphics.black, graphics.screen);
		graphics.drawRect(x, y, fill, 3, graphics.yellow, graphics.screen);
	}
	else
	{
		int fill = (w * playerAmmo) / playerAmmoMax;
		graphics.drawRect(x - 1, y - 1, w + 2, 5, graphics.black, graphics.screen);
		graphics.drawRect(x, y, fill, 3, (playerAmmo <= playerAmmoMax / 4) ? graphics.red : graphics.cyan, graphics.screen);
	}
}

int medalWorker(void *data)
{
	char *tname = (char*)data;
	
	SDL_mutexP(medalServer.lock);
	
	int type = medalServer.postMedal(tname);
	
	while (!graphics.canShowMedalMessage())
	{
		SDL_Delay(100);
	}
	
	SDL_Delay(100);
	
	if (type >= 1 && type <= 3)
	{
		audio.playSound(SND_ITEM, CH_ANY);
		graphics.showMedalMessage(type, medalServer.getMessage());
		
		if (medalServer.hasRuby())
		{
			while (!graphics.canShowMedalMessage())
			{
				SDL_Delay(100);
			}
			
			SDL_Delay(100);
			
			audio.playSound(SND_ITEM, CH_ANY);
			graphics.showMedalMessage(4, medalServer.getRubyMessage());
		}
	}
	
	SDL_mutexV(medalServer.lock);
	
	delete[] tname;
	
	return type;
}

void presentPlayerMedal(const char *tname)
{
	// Copy the input, so that threading
	// doesn't trip us up!
	char *data = new char[128];
	
	if (!engine.cheats)
	{
		strlcpy(data, tname, 128);
		
		SDL_Thread *thread = SDL_CreateThread(medalWorker, "MedalWorker", (void*)data);
		
		if (thread == NULL)
		{
			printf("Unable to create thread: %s\n", SDL_GetError());
			printf("Calling medal server directly\n");
			medalWorker((void*)data);
			return;
		}
	}
}

void addPlayerScore(int score)
{
	if (game.score < 100000 && game.score + score >= 100000)
	{
		presentPlayerMedal("Score_100000");
	}
	else if (game.score < 250000 && game.score + score >= 250000)
	{
		presentPlayerMedal("Score_250000");
	}
	else if (game.score < 500000 && game.score + score >= 500000)
	{
		presentPlayerMedal("Score_500000");
	}
	
	game.score += score;
}

void resetPlayer()
{
	game.getCheckPoint(&player.x, &player.y);
	
	player.dx = 0;
	player.dy = 0;
	player.immune = 120;
	player.environment = ENV_AIR;
	player.oxygen = 7;
	player.fuel = 7;
	resetGrenadeCharge();
	addTeleportParticles(player.x + 10, player.y + 10, 50, SND_TELEPORT2);
	
	Math::removeBit(&player.flags, ENT_FLIES);
	Math::removeBit(&player.flags, ENT_TELEPORTING);
	player.setSprites(graphics.getSprite("BobRight", true), graphics.getSprite("BobLeft", true), graphics.getSprite("BobSpin", true));
	
	map.windPower = 0;
	map.windChangeTime = 90;
}

void gibPlayer()
{
	float x, y, dx, dy;
	int amount = (game.gore) ? 25 : 150;

	for (int i = 0 ; i < amount ; i++)
	{
		x = player.x + Math::rrand(-3, 3);
		y = player.y + Math::rrand(-3, 3);
		
		if (game.gore)
		{
			dx = Math::rrand(-5, 5);
			dy = Math::rrand(-15, -5);
			addEffect(x, y, dx, dy, EFF_BLEEDS);
		}
		else
		{
			dx = Math::rrand(-5, 5);
			dy = Math::rrand(-5, 5);
			addColoredEffect(x, y, dx, dy, graphics.yellow, EFF_COLORED + EFF_WEIGHTLESS);
		}
	}

	(game.gore) ? audio.playSound(SND_SPLAT, CH_ANY) : audio.playSound(SND_POP, CH_ANY);
}

void checkPlayerBulletCollisions(Entity *bullet)
{
	if ((bullet->health < 1) || (player.health <= -60))
	{
		return;
	}

	if ((player.flags & ENT_TELEPORTING) || (game.missionOver > 0))
	{
		return;
	}

	if (bullet->owner != &player)
	{
		if (Collision::collision(&player, bullet))
		{
			if ((!player.immune) && (!(bullet->flags & ENT_EXPLODES)))
			{
				addBlood(&player, bullet->dx / 4, Math::rrand(-6, -3), 1);
				audio.playSound(SND_HIT, CH_ANY);
				if (game.missionOverReason == MIS_INPROGRESS)
				{
					player.health -= bullet->damage;
					player.immune = 120;
				}
			}

			Math::removeBit(&bullet->flags, ENT_SPARKS);
			Math::removeBit(&bullet->flags, ENT_PUFFS);

			if (player.health <= 0)
			{
				player.dx = bullet->dx;
				player.dy = -5;
				audio.playSound(SND_DEATH1 + Math::prand() % 3, CH_DEATH);
				player.health = 0;
			}

			if (bullet->id == WP_LASER)
			{
				debug(("Laser Hit Player"));
				throwAndDamageEntity(&player, 0, -3, 3, -8);
			}
			else
			{
				bullet->health = 0;
			}
		}
	}
}

void doPlayer()
{
	if (engine.cheatHealth)
	{
		player.health = MAX_HEALTH;
	}

	if (game.missionOverReason == MIS_PLAYERESCAPE)
	{
		return;
	}

	if (player.y > (map.limitDown + 500))
	{
		if (game.missionOver == 0)
		{
			player.health--;
		}

		if (player.health > 0)
		{
			game.setMissionOver(MIS_PLAYEROUT);
		}
		else
		{
			game.setMissionOver(MIS_PLAYERDEAD);
		}
	}
	
	if (player.flags & ENT_TELEPORTING)
	{
		moveEntity(&player);
		return;
	}

	if (game.missionOverReason == MIS_PLAYEROUT)
	{
		graphics.blit(player.getFaceImage(), (int)(player.x - engine.playerPosX), (int)(player.y - engine.playerPosY), graphics.screen, false);
		moveEntity(&player);
		return;
	}

	if ((player.health < 1) || (player.immune > 120))
	{
		if (player.health <= -60)
		{
			return;
		}

		player.think();
		
		if (game.hasAquaLung)
		{
			player.oxygen = 7;
		}

		moveEntity(&player);

		graphics.blit(player.getFaceImage(), (int)(player.x - engine.playerPosX), (int)(player.y - engine.playerPosY), graphics.screen, false);

		if (player.health < 1)
		{
			player.health--;
		}

		if (player.health <= -60)
		{
			gibPlayer();
		}

		return;
	}

	if ((!(player.flags & ENT_FLIES)) && (!map.isIceLevel))
	{
		player.dx = 0;
	}
	
	// toggles the Jetpack if available
	if (config.isControl(CONTROL::JETPACK))
	{
		if ((game.hasJetPack) || (engine.cheatExtras))
		{
			if (player.flags & ENT_FLIES)
			{
				Math::removeBit(&player.flags, ENT_FLIES);
				player.setSprites(graphics.getSprite("BobRight", true), graphics.getSprite("BobLeft", true), graphics.getSprite("BobSpin", true));
			}
			else
			{
				if ((player.fuel > 3) && (player.environment == ENV_AIR))
				{
					Math::addBit(&player.flags, ENT_FLIES);
					player.setSprites(graphics.getSprite("JPBobRight", true), graphics.getSprite("JPBobLeft", true), graphics.getSprite("BobSpin", true));
				}
				else if (player.environment == ENV_WATER)
				{
					engine.setInfoMessage("Jetpack cannot be used underwater", 0, INFO_NORMAL);
				}
				else
				{
					engine.setInfoMessage("Jetpack is recharging...", 0, INFO_NORMAL);
				}
			}
		}
		else
		{
			engine.setInfoMessage("Don't have jetpack!", 0, INFO_NORMAL);
		}
		
		config.resetControl(CONTROL::JETPACK);
	}
	
	if (map.isBlizzardLevel)
	{
		if ((!config.isControl(CONTROL::LEFT)) && (!config.isControl(CONTROL::RIGHT)))
		{
			if ((player.dx > -2) && (player.dx < 2))
			{
				player.dx += map.windPower * 0.1;
			}
		}
	}

	if (config.isControl(CONTROL::LEFT))
	{
		player.face = 1;
		
		if (player.flags & ENT_FLIES)
		{
			player.dx -= 0.1;
			Math::limitFloat(&player.dx, -PLAYER_FLY_SPEED, PLAYER_FLY_SPEED);
		}
		else if (map.isIceLevel)
		{
			player.dx -= 0.1;
			Math::limitFloat(&player.dx, -PLAYER_WALK_SPEED * 1.25, PLAYER_WALK_SPEED * 1.25);
			player.animate();
		}
		else
		{
			player.dx = -PLAYER_WALK_SPEED;
			player.animate();
		}

		if (engine.cheatSpeed)
		{
			player.dx *= 3;
		}
	}
	else if (config.isControl(CONTROL::RIGHT))
	{
		player.face = 0;

		if (player.flags & ENT_FLIES)
		{
			player.dx += 0.1;
			Math::limitFloat(&player.dx, -PLAYER_FLY_SPEED, PLAYER_FLY_SPEED);
		}
		else if (map.isIceLevel)
		{
			player.dx += 0.1;
			player.animate();
			Math::limitFloat(&player.dx, -PLAYER_WALK_SPEED * 1.25, PLAYER_WALK_SPEED * 1.25);
		}
		else
		{
			player.dx = PLAYER_WALK_SPEED;
			player.animate();
		}
		
		if (engine.cheatSpeed)
		{
			player.dx *= 3;
		}
	}
	else if (!(player.flags & ENT_FLIES))
	{
		// usual idle frame
		player.currentFrame = 0;
	}

	Math::limitFloat(&player.x, 10, map.limitRight + 608);
	Math::limitFloat(&player.y, map.limitUp + 5, (MAPHEIGHT * BRICKSIZE) + 64);

	// Keyboard break fix - Feb 09
	if ((config.isControl(CONTROL::UP)) || (config.isControl(CONTROL::JUMP)))
	{
		if (player.flags & ENT_FLIES)
		{
			player.dy -= 0.1;
			Math::limitFloat(&player.dy, -PLAYER_FLY_SPEED, PLAYER_FLY_SPEED);
			if (engine.cheatSpeed)
			{
				player.dy *= 2;
			}
		}
		else if ((player.environment == ENV_AIR) && (player.falling == false))
		{
			player.falling = true;
			player.dy = PLAYER_JUMP_SPEED;
			config.resetControl(CONTROL::UP);
			config.resetControl(CONTROL::JUMP);
		}
		else if (player.environment != ENV_AIR)
		{
			player.dy = -PLAYER_WALK_SPEED;
			player.animate();
			if (engine.cheatSpeed)
			{
				player.dy *= 3;
			}
		}
	}

	if (config.isControl(CONTROL::DOWN))
	{
		if (player.flags & ENT_FLIES)
		{
			player.dy += 0.1;
			Math::limitFloat(&player.dy, -PLAYER_FLY_SPEED, PLAYER_FLY_SPEED);
		}
		else if (player.environment != ENV_AIR)
		{
			player.dy = 2;
			player.animate();
		}

		if (engine.cheatSpeed)
		{
			player.dy *= 3;
		}
	}
	
	#if DEBUG
	if (engine.keyState[SDL_SCANCODE_1])
	{
		player.currentWeapon = &weapon[WP_PISTOL];
		resetPlayerAmmo();
	}
	else if (engine.keyState[SDL_SCANCODE_2])
	{
		player.currentWeapon = &weapon[WP_MACHINEGUN];
		resetPlayerAmmo();
	}
	else if (engine.keyState[SDL_SCANCODE_3])
	{
		player.currentWeapon = &weapon[WP_GRENADES];
		resetPlayerAmmo();
	}
	else if (engine.keyState[SDL_SCANCODE_4])
	{
		player.currentWeapon = &weapon[WP_LASER];
		resetPlayerAmmo();
	}
	else if (engine.keyState[SDL_SCANCODE_5])
	{
		player.currentWeapon = &weapon[WP_SPREAD];
		resetPlayerAmmo();
	}
	#endif
	
	moveEntity(&player);

	// Math::limitFloat(&player.x, 10, map.limitRight + 608);
	// moveEntity() limits x < 10
	if (player.x > map.limitRight + 608)
	{
		player.x = map.limitRight + 608;
		if (player.dx > 0)
		{
			player.dx = 0;
		}
	}
	// Math::limitFloat(&player.y, map.limitUp + 5, (MAPHEIGHT * BRICKSIZE) + 64);
	if (player.y < map.limitUp + 5)
	{
		player.y = map.limitUp + 5;
		if (player.dy < 0)
		{
			player.dy = 0;
		}
	}
	else if (player.y > (MAPHEIGHT * BRICKSIZE) + 64)
	{
		player.y = (MAPHEIGHT * BRICKSIZE) + 64;
	}

	// Mouse aiming: points the shot and Bob's face at the cursor, left click fires
	bool keyFire = config.isControl(CONTROL::FIRE);
	updatePlayerAim(keyFire);

	// Right click reloads before the clip is empty (only if there is a clip and it is not full; never for grenades or rockets)
	if ((takePlayerReloadRequest()) && (getLimitedShots() == 0) && (playerAmmoMax > 0) && (!playerReloading) && (playerAmmo < playerAmmoMax))
	{
		resetGrenadeCharge(); // drop a grenade that was being charged
		startPlayerReload();
	}

	bool fireHeld = keyFire || (isMouseAiming() && (engine.mouseLeft != 0));

	updatePlayerReload();

	// Limited-use weapons (grenades, rocket launcher): once the last shot is spent, back to the pistol
	if ((getLimitedShots() > 0) && (playerAmmo == 0))
	{
		player.currentWeapon = &weapon[WP_PISTOL];
		game.currentWeapon = WP_PISTOL;
		resetPlayerAmmo();
		player.reload = player.currentWeapon->reload;
	}

	// Grenades are charged while FIRE is held and thrown on release.
	// For every other weapon this returns false and firing works as before.
	if (!handlePlayerGrenade(fireHeld))
	{
		if (fireHeld)
		{
			if (player.reload == 0 && !playerReloading)
			{
				if (playerAmmoMax > 0 && playerAmmo == 0)
				{
					startPlayerReload();
				}
				else if (playerAmmoMax == 0 || playerAmmo > 0)
				{
					addBullet(&player, player.currentWeapon->getSpeed(player.face), 0);
					if (player.currentWeapon == &weapon[WP_SPREAD])
					{
						addBullet(&player, player.currentWeapon->getSpeed(player.face), 2);
						addBullet(&player, player.currentWeapon->getSpeed(player.face), -2);
					}
					// Only the pistol can fire underwater, so a limited-use weapon keeps its shot there
					if ((playerAmmoMax > 0) && (!((getLimitedShots() > 0) && (player.environment == ENV_WATER))))
					{
						playerAmmo--;
						if (playerAmmo == 0)
							startPlayerReload();
					}
				}
			}
		}
	}
	
	if (player.environment == ENV_WATER)
	{
		Math::removeBit(&player.flags, ENT_FLIES);

		addBubble(player.x, player.y);
		
		if ((player.thinktime == 30) && (player.oxygen == 0))
		{
			audio.playSound(SND_DROWNING, CH_ANY);
		}
	}

	player.think();
	
	if (player.fuel == 0)
	{
		player.setSprites(graphics.getSprite("BobRight", true), graphics.getSprite("BobLeft", true), graphics.getSprite("BobSpin", true));
	}
	
	if ((game.hasAquaLung) || (engine.cheatExtras))
	{
		player.oxygen = 7;
	}
		
	if (engine.cheatFuel)
	{
		player.fuel = 7;
	}

	if (((player.immune % 5) == 0) && (!(player.flags & ENT_TELEPORTING)))
	{
		if ((game.missionOverReason == MIS_COMPLETE) || (game.missionOverReason == MIS_INPROGRESS) || (game.missionOverReason == MIS_GAMECOMPLETE))
		{
			if (player.flags & ENT_FLIES)
			{
				player.animate();
				addFireTrailParticle(player.x + (player.face * 16) + Math::rrand(-1, 1), player.y + 10 + Math::rrand(-1, 1));
			}
			graphics.blit(player.getFaceImage(), (int)(player.x - engine.playerPosX), (int)(player.y - engine.playerPosY), graphics.screen, false);
		}
	}

	if (game.missionOverReason == MIS_INPROGRESS)
	{
		drawGrenadeCharge();
		drawPlayerAmmo();
		drawPlayerCrosshair();
	}
}
