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

#include "bullets.h"

extern void startPlayerReload();
extern int playerAmmo;
extern int playerAmmoMax;

void addBullet(Entity *owner, float dx, float dy)
{
	if (!(owner->flags & ENT_BOSS))
	{
		if (owner->environment == ENV_WATER)
		{
			if ((owner->currentWeapon != &weapon[WP_PISTOL]) && (owner->currentWeapon != &weapon[WP_AIMEDPISTOL]))
			{
				return;
			}
		}
	}

	Entity *bullet = new Entity();

	bullet->x = owner->x;
	bullet->y = owner->y;// + owner->dy;

	if (owner != &engine.world)
	{
		bullet->x += (owner->width / 2);
		bullet->y += (owner->height / 2);
	}

	bullet->setName(owner->currentWeapon->name);
	bullet->id = owner->currentWeapon->id;
	bullet->dx = dx;
	bullet->dy = owner->currentWeapon->dy + dy;

	// Add motion of the player and any platform he/she is on to grenades
	if (owner->currentWeapon->dy)
	{
		int tdx, tdy;
		getTrainMotion(owner, tdx, tdy);
		bullet->dx += owner->dx - tdx;
		bullet->dy += -tdy;
	}

	bullet->next = NULL;
	bullet->health = owner->currentWeapon->health;
	bullet->damage = owner->currentWeapon->damage;
	bullet->setSprites(owner->currentWeapon->sprite[0], owner->currentWeapon->sprite[1], owner->currentWeapon->sprite[1]);
	bullet->face = owner->face;
	bullet->owner = owner;
	bullet->flags = owner->currentWeapon->flags + ENT_SPARKS + ENT_BULLET + ((owner->flags & ENT_BOSS) ? ENT_BOSS : 0);

	if (bullet->flags & ENT_EXPLODES)
	{
		bullet->deathSound = SND_GRENADE;
	}
	else if (owner->currentWeapon->fireSound > -1)
	{
		if ((Math::prand() % 2) == 0)
		{
			bullet->deathSound = SND_RICO1;
		}
		else
		{
			bullet->deathSound = SND_RICO2;
		}
	}
	
	// cheating here!
	if (owner->currentWeapon->id == WP_STALAGTITE)
	{
		bullet->deathSound = SND_STONEBREAK;
	}

	if (owner->currentWeapon->fireSound > -1)
	{
		audio.playSound(owner->currentWeapon->fireSound, CH_WEAPON, owner->x);
	}

	if (owner->flags & ENT_AIMS)
	{
		Math::calculateSlope(player.x + Math::rrand(-20, 20), player.y + Math::rrand(-20, 20), bullet->x, bullet->y, &bullet->dx, &bullet->dy);
		bullet->dx *= owner->currentWeapon->dx;
		bullet->dy *= owner->currentWeapon->dy;
	}

	map.addBullet(bullet);

	// Adjust the reload time of enemies according to difficulty level
	owner->reload = owner->currentWeapon->reload;
	
	if ((owner != &player) && (game.skill < 3))
	{
		owner->reload *= (3 - game.skill);
	}

	if (owner->flags & ENT_ALWAYSFIRES)
	{
		owner->reload = 10;
	}

	if (owner == &player)
	{
		game.incBulletsFired();
		
		if (engine.cheatReload)
		{
			owner->reload = 4;
		}
		
		if (game.bulletsFired[game.currentWeapon] == 10000)
		{
			presentPlayerMedal("10000_Bullets");
		}
	}
}

void destroyBullet(Entity *bullet)
{
	if (bullet->deathSound == -1)
	{
		return;
	}

	bullet->health = 0;

	if (bullet->flags & ENT_SPARKS)
	{
		audio.playSound(bullet->deathSound, CH_TOUCH, bullet->x);
	}

	if (bullet->flags & ENT_EXPLODES)
	{
		addExplosion(bullet->x + (bullet->width / 2), bullet->y + (bullet->height / 2), bullet->damage, bullet->owner);
	}
	
	if (bullet->id == WP_STALAGTITE)
	{
		throwStalagParticles(bullet->x, bullet->y);
	}

	float dx, dy;

	for (int i = 0 ; i < 3 ; i++)
	{
		dx = Math::rrand(-30, 30); dx /= 12;
		dy = Math::rrand(-30, 30); dy /= 12;
		
		if (bullet->flags & ENT_SPARKS)
		{
			map.addParticle(bullet->x, bullet->y, dx, dy, Math::rrand(5, 30), graphics.white, NULL, 0);
		}
		else
		{
			map.addParticle(bullet->x, bullet->y, dx, dy, Math::rrand(5, 30), graphics.red, NULL, 0);
		}
	}
}

// Just a little convinence function!
void removeBullet(Entity *bullet)
{
	bullet->health = 0;
	bullet->deathSound = -1;
	Math::removeBit(&bullet->flags, ENT_SPARKS);
	Math::removeBit(&bullet->flags, ENT_PUFFS);
	Math::removeBit(&bullet->flags, ENT_EXPLODES);
}

void bounceBullet(Entity *bullet, float dx, float dy)
{
	if (dx)
	{
		bullet->dx = -bullet->dx;
		bullet->x += bullet->dx;
		if (bullet->id != WP_LASER)
		{
			bullet->dx *= 0.75;
			audio.playSound(SND_GRBOUNCE, CH_TOUCH, bullet->x);
		}
		bullet->face = !bullet->face;
	}

	if (dy)
	{
		bullet->dy = -bullet->dy;
		bullet->y += bullet->dy;
		
		Math::limitFloat(&bullet->dy, -4, 4);

		if ((bullet->dy > -2) && (bullet->dy <= 0)) bullet->dy = -2;
		if ((bullet->dy > 0) && (bullet->dy < 2)) bullet->dy = 2;

		if (bullet->id != WP_LASER)
		{
			bullet->dy *= 0.75;
			audio.playSound(SND_GRBOUNCE, CH_TOUCH, bullet->x);
		}

		if ((bullet->dy > -2) && (bullet->dy <= 0)) bullet->dy = -2;
		if ((bullet->dy > 0) && (bullet->dy < 2)) bullet->dy = 2;

		bullet->face = !bullet->face;
	}
}

bool bulletHasCollided(Entity *bullet, float dx, float dy)
{
	bullet->x += dx;
	bullet->y += dy;

	int x = (int)bullet->x >> BRICKSHIFT;
	int y = (int)bullet->y >> BRICKSHIFT;

	if ((x < 0) || (y < 0))
	{
		removeBullet(bullet);
	}
	else
	{
		if (map.isSolid(x, y))
		{
			if (map.isBreakable(x, y))
			{
				if (bullet->flags & ENT_EXPLODES)
				{
					Math::removeBit(&bullet->flags, ENT_BOUNCES);
					map.data[x][y] = MAP_AIR;
					audio.playSound(SND_STONEBREAK, CH_EXPLODE, bullet->x);
					throwBrickParticles(x << BRICKSHIFT, y << BRICKSHIFT);
				}
				else
				{
					if ((Math::prand() % 2) == 0)
					{
						map.data[x][y] = MAP_AIR;
						audio.playSound(SND_STONEBREAK, CH_EXPLODE, bullet->x);
						throwBrickParticles(x << BRICKSHIFT, y << BRICKSHIFT);
					}
				}
			}

			if (bullet->flags & ENT_BOUNCES)
			{
				bounceBullet(bullet, dx, dy);
			}

			return true;
		}
	}

	enemyBulletCollisions(bullet);

	checkPlayerBulletCollisions(bullet);
	
	checkBossBulletCollisions(bullet);
	
	checkSwitchContact(bullet);

	if ((checkTrainContact(bullet, DIR_XY)) || (checkObstacleContact(bullet, DIR_XY)))
	{
		if (bullet->flags & ENT_BOUNCES)
			bounceBullet(bullet, dx, dy);
		return true;
	}

	return false;
}

void doBullets()
{
	Entity *bullet = (Entity*)map.bulletList.getHead();
	Entity *previous = bullet;

	int x, y;

	while (bullet->next != NULL)
	{
		bullet = (Entity*)bullet->next;
		
		bullet->owner->referenced = true;

		x = (int)(bullet->x - engine.playerPosX);
		y = (int)(bullet->y - engine.playerPosY);

		graphics.blit(bullet->getFaceImage(), x, y, graphics.screen, true);
		bullet->animate();

		if (bullet->flags & ENT_ONFIRE)
		{
			addFireParticles(bullet->x + Math::rrand(-8, 8), bullet->y + Math::rrand(-8, 8), 1);
		}

		if (bullet->flags & ENT_FIRETRAIL)
		{
			addFireTrailParticle(bullet->x, bullet->y);
		}
		
		if (bullet->flags & ENT_PARTICLETRAIL)
		{
			addColorParticles(x, y, -1, 3);
		}

		if (bullet->owner == &player)
		{
			if ((x < -160) || (y < -120) || (x > graphics.screen->w + 160) || (y > graphics.screen->h + 120))
			{
				removeBullet(bullet);
			}
		}

		if (bulletHasCollided(bullet, bullet->dx, 0))
		{
			if (!(bullet->flags & ENT_BOUNCES))
			{
				bullet->health = 0;
			}
		}

		if (bulletHasCollided(bullet, 0, bullet->dy))
		{
			if (!(bullet->flags & ENT_BOUNCES))
			{
				bullet->health = 0;
			}
		}

		bullet->health--;
		
		if (bullet->health == 0)
		{
			Math::removeBit(&bullet->flags, ENT_SPARKS);
			Math::removeBit(&bullet->flags, ENT_PUFFS);
		}

		if (!(bullet->flags & ENT_WEIGHTLESS))
		{
			bullet->dy += 0.1;
		}

		if (bullet->health > 0)
		{
			previous = bullet;
		}
		else
		{
			destroyBullet(bullet);
			map.bulletList.remove(previous, bullet);
			bullet = previous;
		}
	}
}

/*
	Charged grenade throw: hold FIRE to charge, release to throw.
	A quick tap throws exactly like the normal grenade.
*/
static int grenadeCharge = 0;
static const int GRENADE_CHARGE_MAX = 45;      // about 0.75 s at 60 fps
static const float GRENADE_SPEED_BOOST = 3.0f; // dx = base * (1 + 3) at full charge
static const float GRENADE_ARC_BOOST = -2.5f;  // extra upward dy at full charge

void resetGrenadeCharge()
{
	grenadeCharge = 0;
}

// Returns true if the current weapon is the grenade (the button was handled here)
bool handlePlayerGrenade(bool fireHeld)
{
	if ((player.currentWeapon != &weapon[WP_GRENADES]) || (player.health <= 0))
	{
		grenadeCharge = 0;
		return false;
	}

	if (fireHeld)
	{
		if ((player.reload <= 0) && (grenadeCharge < GRENADE_CHARGE_MAX))
		{
			grenadeCharge++;
		}

		return true;
	}

	if ((grenadeCharge > 0) && (player.reload <= 0))
	{
		if (playerAmmoMax > 0 && playerAmmo == 0)
		{
			startPlayerReload();
			grenadeCharge = 0;
			return true;
		}

		float t = (float)grenadeCharge / GRENADE_CHARGE_MAX;

		float dx = player.currentWeapon->getSpeed(player.face) * (1.0f + (GRENADE_SPEED_BOOST * t));
		float dyExtra = GRENADE_ARC_BOOST * t;

		addBullet(&player, dx, dyExtra);

		if (playerAmmoMax > 0)
		{
			playerAmmo--;
			if (playerAmmo == 0)
				startPlayerReload();
		}
	}

	grenadeCharge = 0;
	return true;
}

void drawGrenadeCharge()
{
	if (grenadeCharge <= 0)
		return;

	int x = (int)(player.x - engine.playerPosX);
	int y = (int)(player.y - engine.playerPosY) - 10;
	int w = 24;
	int fill = (w * grenadeCharge) / GRENADE_CHARGE_MAX;

	graphics.drawRect(x - 1, y - 1, w + 2, 5, graphics.black, graphics.screen);
	graphics.drawRect(x, y, fill, 3, (grenadeCharge >= GRENADE_CHARGE_MAX) ? graphics.red : graphics.yellow, graphics.screen);
}
