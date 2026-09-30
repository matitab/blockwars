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

#include "explosions.h"

// Damage Bob takes from an explosion: MAX at the center, falling to MIN at the edge of the radius
// (Bob has MAX_HEALTH = 10 points)
static const int EXPLOSION_PLAYER_MAX_DAMAGE = 5;
static const int EXPLOSION_PLAYER_MIN_DAMAGE = 1;

// Se define en traps.cpp: avisa a las minas que hubo una explosion
void requestTrapBlast(float x, float y, int radius);

// ---- TEMBLOR DE PANTALLA BEGIN ----
/*
	Cada explosion sacude la camara segun lo cerca que este Bob. La fuerza baja de forma
	lineal hasta cero durante la duracion. Se mide con el reloj (SDL_GetTicks), asi que no
	necesita actualizarse por fotograma. Engine::setPlayerPosition() suma el resultado.
*/
// Nivel: game.screenShake (0 = apagado, 1 = suave, 2 = normal, 3 = fuerte)
static const float SHAKE_LEVEL_SCALE[4] = { 0.0f, 0.5f, 1.0f, 1.6f };
static const float SHAKE_MIN_POWER = 2.0f;   // pixeles en el borde del alcance
static const float SHAKE_MAX_POWER = 12.0f;  // pixeles pegado a la explosion
static const float SHAKE_RANGE_PER_RADIUS = 5.0f;
static const float SHAKE_MIN_RANGE = 250.0f;

static Uint32 shakeStart = 0;
static Uint32 shakeDuration = 0;
static float shakePower = 0.0f;

static void addScreenShake(float power, Uint32 ms)
{
	Uint32 now = SDL_GetTicks();

	// una sacudida nueva solo reemplaza a la actual si es mas fuerte que lo que le queda
	if ((shakeDuration > 0) && ((now - shakeStart) < shakeDuration))
	{
		float remaining = shakePower * (1.0f - ((float)(now - shakeStart) / shakeDuration));

		if (remaining >= power)
		{
			return;
		}
	}

	shakePower = power;
	shakeStart = now;
	shakeDuration = ms;
}

static void addExplosionShake(float x, float y, int radius)
{
	float dx = (player.x + (player.width / 2)) - x;
	float dy = (player.y + (player.height / 2)) - y;
	float dist = sqrtf((dx * dx) + (dy * dy));

	float range = radius * SHAKE_RANGE_PER_RADIUS;

	if (range < SHAKE_MIN_RANGE)
	{
		range = SHAKE_MIN_RANGE;
	}

	if (dist >= range)
	{
		return;
	}

	float closeness = 1.0f - (dist / range);

	addScreenShake(SHAKE_MIN_POWER + ((SHAKE_MAX_POWER - SHAKE_MIN_POWER) * closeness), (Uint32)(180 + (320 * closeness)));
}

// Desvio actual de la camara en pixeles. Lo usa Engine::setPlayerPosition().
void getScreenShake(int *shakeX, int *shakeY)
{
	*shakeX = *shakeY = 0;

	int level = game.screenShake;
	Math::limitInt(&level, 0, 3);

	if ((level == 0) || (shakeDuration == 0))
	{
		return;
	}

	Uint32 elapsed = SDL_GetTicks() - shakeStart;

	if (elapsed >= shakeDuration)
	{
		return;
	}

	int amount = (int)(shakePower * SHAKE_LEVEL_SCALE[level] * (1.0f - ((float)elapsed / shakeDuration)));

	if (amount < 1)
	{
		return;
	}

	*shakeX = Math::rrand(-amount, amount);
	*shakeY = Math::rrand(-amount, amount);
}
// ---- TEMBLOR DE PANTALLA END ----

void addExplosion(float x, float y, int radius, Entity *owner)
{
	// las minas cercanas detonan en cadena y la camara se sacude segun la distancia a Bob
	requestTrapBlast(x, y, radius);
	addExplosionShake(x, y, radius);

	audio.playSound(SND_GRENADE, CH_EXPLODE, x);

	float dx, dy;
	int distX, distY;
	int distance;

	Sprite *explosion = graphics.getSprite("Explosion", true);

	for (int i = 0 ; i < radius ; i++)
	{
		dx = Math::rrand(-radius, radius); dx /= 10;
		dy = Math::rrand(-radius, radius); dy /= 10;
		map.addParticle(x, y, dx, dy, Math::rrand(5, 30), graphics.white, explosion, PAR_WEIGHTLESS);
	}

	Entity *enemy = (Entity*)map.enemyList.getHead();

	while (enemy->next != NULL)
	{
		enemy = (Entity*)enemy->next;
		
		if ((enemy->flags & ENT_IMMUNE) || (enemy->flags & ENT_IMMUNEEXPLODE))
		{
			continue;
		}
		
		if (enemy->dead == DEAD_DYING)
		{
			continue;
		}

		distX = (int)fabs(enemy->x + (enemy->width / 2) - x);
		distY = (int)fabs(enemy->y + (enemy->height / 2) - y);

		distX *= distX;
		distY *= distY;

		distance = (int)sqrt(distX + distY);

		if (radius - distance > 0)
		{	
			if (enemy->health > 0)
			{
				enemy->health -= radius - distance;
				
				if (enemy->health <= 0)
				{	
					checkObjectives("Enemy", false);
					checkObjectives(enemy->name, false);
				}
				
				if (!(enemy->flags & ENT_STATIC))
				{
					if (enemy->flags & ENT_EXPLODES)
					{
						audio.playSound(SND_ELECDEATH1 + Math::prand() % 3, CH_DEATH, enemy->x);
					}
					else
					{
						if (game.gore)
						{
							audio.playSound(SND_DEATH1 + Math::prand() % 3, CH_DEATH, enemy->x);
						}
					}
				}
				
				if (owner == &player)
				{
					addPlayerScore(enemy->value);
				}
			}
			
			for (int i = 0 ; i < 4 ; i++)
			{
				(enemy->flags & ENT_EXPLODES) ? addSmokeAndFire(enemy, Math::rrand(-15, 5), Math::rrand(-15, 5), 2) : addBlood(enemy, Math::rrand(-5, 5), Math::rrand(-5, 5), 1);
			}
				
			if (!(enemy->flags & ENT_STATIC))
			{
				enemy->dx = Math::rrand(-5, 5);
				enemy->dy = Math::rrand(-5, 0);
			}
		}
	}

	// Bob's own grenades hurt him too (owner == &player is no longer excluded)
	if ((player.immune) || (player.health <= -60) || (game.missionOver > 0))
	{
		return;
	}

	distX = (int)fabs(player.x + (player.width / 2) - x);
	distY = (int)fabs(player.y + (player.height / 2) - y);

	distX *= distX;
	distY *= distY;

	distance = (int)sqrt(distX + distY);

	if (radius - distance >= 0)
	{
		// Same idea as the enemies above: the closer to the center, the more damage
		int damage = EXPLOSION_PLAYER_MIN_DAMAGE;

		if (radius > 0)
		{
			// 1.0 at the center of the blast, 0.0 at the edge (float + rounding, so every point in between counts)
			float closeness = (float)(radius - distance) / radius;
			damage += (int)(((EXPLOSION_PLAYER_MAX_DAMAGE - EXPLOSION_PLAYER_MIN_DAMAGE) * closeness) + 0.5f);
		}

		// Throw Bob away from the blast. throwAndDamageEntity() picks dx = -minDX or maxDX at random,
		// so passing the same horizontal speed in both makes the direction fixed
		int push = ((player.x + (player.width / 2)) < x) ? -3 : 3;
		throwAndDamageEntity(&player, damage, -push, push, -8);
	}
}
