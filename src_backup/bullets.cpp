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
#include <math.h>

extern void startPlayerReload();
extern void notifyPlayerShot(float originX, float originY, float dirX, float dirY);
extern bool deflectGrenadeAtDroids(Entity *bullet);
extern int playerAmmo;
extern int playerAmmoMax;

// Chispas de impacto: salen en sentido contrario al movimiento de la bala
static void addImpactSparks(Entity *bullet, int count)
{
	float dx, dy;
	float biasX = -bullet->dx * 0.12f;
	float biasY = -bullet->dy * 0.12f;

	Math::limitFloat(&biasX, -4, 4);
	Math::limitFloat(&biasY, -4, 4);

	for (int i = 0 ; i < count ; i++)
	{
		dx = Math::rrand(-30, 30); dx /= 12;
		dy = Math::rrand(-30, 30); dy /= 12;

		if (bullet->flags & ENT_SPARKS)
		{
			// mezcla de blanco y amarillo
			if ((i % 3) == 0)
			{
				map.addParticle(bullet->x, bullet->y, dx + biasX, dy + biasY, Math::rrand(5, 30), graphics.yellow, NULL, 0);
			}
			else
			{
				map.addParticle(bullet->x, bullet->y, dx + biasX, dy + biasY, Math::rrand(5, 30), graphics.white, NULL, 0);
			}
		}
		else
		{
			map.addParticle(bullet->x, bullet->y, dx + biasX, dy + biasY, Math::rrand(5, 30), graphics.red, NULL, 0);
		}
	}
}

// ---- MOUSE AIM BEGIN ----
/*
	Mouse aiming.
	The mouse position arrives in graphics.screen coordinates (SDL scales it through
	SDL_RenderSetLogicalSize), so the cursor in the world is (mouse + engine.playerPos).
	Aiming turns on when the mouse moves or is clicked and turns off when the
	keyboard FIRE control is used, so keyboard-only play works exactly as before.
*/
static const float GRENADE_MAX_VY = 8.0f; // keeps a full-charge lob straight up on screen

static bool mouseAimActive = false;
static float aimDirX = 1.0f;              // unit vector Bob -> cursor
static float aimDirY = 0.0f;
static int lastMouseX = 0;
static int lastMouseY = 0;
static bool lastMouseLeft = false;
static Uint32 lastAimTick = 0;
static bool lastMouseRight = false;
static bool reloadRequested = false;

// ---- CAMARA ADELANTADA BEGIN ----
/*
	La camara se corre hacia donde apunta el mouse. El corrimiento sale de la distancia
	entre el cursor y el centro de la pantalla (no depende de la posicion de Bob, asi que
	no se retroalimenta), se suaviza fotograma a fotograma y se suma en Engine::setPlayerPosition().
	Solo actua con la punteria por mouse activa; con teclado la camara queda como siempre.
*/
// Nivel de la camara: game.cameraLead (0 = apagada, 1 = suave, 2 = normal, 3 = fuerte)
static const float CAMERA_LEAD_FACTOR[4] = { 0.0f, 0.15f, 0.25f, 0.35f };
static const float CAMERA_LEAD_MAX[4] = { 0.0f, 60.0f, 100.0f, 150.0f };
static const float CAMERA_LEAD_SMOOTH = 0.10f; // fraccion del camino que se recorre por fotograma
static float camLeadX = 0.0f;
static float camLeadY = 0.0f;

static void updateCameraLead(bool aiming, int mx, int my)
{
	int level = game.cameraLead;
	Math::limitInt(&level, 0, 3);

	float targetX = 0.0f;
	float targetY = 0.0f;

	if ((aiming) && (level > 0))
	{
		float maxX = CAMERA_LEAD_MAX[level];
		float maxY = maxX * 0.75f;

		targetX = (mx - (graphics.logicalW() / 2)) * CAMERA_LEAD_FACTOR[level];
		targetY = (my - (graphics.logicalH() / 2)) * CAMERA_LEAD_FACTOR[level];

		Math::limitFloat(&targetX, -maxX, maxX);
		Math::limitFloat(&targetY, -maxY, maxY);
	}

	camLeadX += (targetX - camLeadX) * CAMERA_LEAD_SMOOTH;
	camLeadY += (targetY - camLeadY) * CAMERA_LEAD_SMOOTH;
}

// Corrimiento actual de la camara en pixeles. Lo usa Engine::setPlayerPosition().
void getCameraLead(int *leadX, int *leadY)
{
	// Si doPlayer() dejo de llamar (menu, muerte, otra pantalla) el corrimiento se apaga suave
	if ((SDL_GetTicks() - lastAimTick) > 250)
	{
		camLeadX *= 0.85f;
		camLeadY *= 0.85f;
	}

	*leadX = (int)camLeadX;
	*leadY = (int)camLeadY;
}
// ---- CAMARA ADELANTADA END ----

bool isMouseAiming()
{
	return (game.mouseAim && mouseAimActive);
}

// True once per right click (manual reload); reading it clears the request
bool takePlayerReloadRequest()
{
	bool request = reloadRequested;
	reloadRequested = false;
	return request;
}

// Called once per frame from doPlayer(), after the player has moved
void updatePlayerAim(bool keyboardFire)
{
	if (!game.mouseAim)
	{
		mouseAimActive = false;
		return;
	}

	int mx = engine.getMouseX();
	int my = engine.getMouseY();
	bool mouseLeft = (engine.mouseLeft != 0);

	// If doPlayer() was not called for a while (menu, pause) ignore what the mouse
	// did in the meantime, so clicking "resume" does not switch the aiming mode
	Uint32 now = SDL_GetTicks();
	bool resumed = ((now - lastAimTick) > 250);
	lastAimTick = now;

	if (resumed)
	{
		camLeadX = camLeadY = 0.0f;
	}

	// Right click = manual reload: one request per press, ignored on the first frame after a menu
	bool mouseRight = (engine.mouseRight != 0);
	if ((!resumed) && (mouseRight) && (!lastMouseRight))
	{
		reloadRequested = true;
	}
	lastMouseRight = mouseRight;

	if (!resumed)
	{
		if ((mx != lastMouseX) || (my != lastMouseY) || (mouseLeft && !lastMouseLeft))
		{
			mouseAimActive = true;
		}
	}

	lastMouseX = mx;
	lastMouseY = my;
	lastMouseLeft = mouseLeft;

	if (keyboardFire && !mouseLeft)
	{
		mouseAimActive = false;
	}

	updateCameraLead(mouseAimActive, mx, my);

	if (!mouseAimActive)
	{
		return;
	}

	// Bob's center: same point addBullet() uses as the muzzle
	float cx = (player.x + (player.width / 2)) - engine.playerPosX;
	float cy = (player.y + (player.height / 2)) - engine.playerPosY;

	float vx = mx - cx;
	float vy = my - cy;
	float len = sqrtf((vx * vx) + (vy * vy));

	if (len >= 6.0f)
	{
		aimDirX = vx / len;
		aimDirY = vy / len;
		player.face = (vx < 0) ? 1 : 0;
	}
	else
	{
		// cursor on top of Bob: shoot straight ahead
		aimDirX = (player.face) ? -1.0f : 1.0f;
		aimDirY = 0.0f;
	}
}

void drawPlayerCrosshair()
{
	if (!isMouseAiming())
	{
		return;
	}

	int x = engine.getMouseX();
	int y = engine.getMouseY();

	Math::limitInt(&x, 9, graphics.logicalW() - 10);
	Math::limitInt(&y, 9, graphics.logicalH() - 10);

	// black outline first, then the white cross with a gap in the middle
	graphics.drawRect(x - 9, y - 1, 7, 3, graphics.black, graphics.screen);
	graphics.drawRect(x + 3, y - 1, 7, 3, graphics.black, graphics.screen);
	graphics.drawRect(x - 1, y - 9, 3, 7, graphics.black, graphics.screen);
	graphics.drawRect(x - 1, y + 3, 3, 7, graphics.black, graphics.screen);

	graphics.drawRect(x - 8, y, 5, 1, graphics.white, graphics.screen);
	graphics.drawRect(x + 4, y, 5, 1, graphics.white, graphics.screen);
	graphics.drawRect(x, y - 8, 1, 5, graphics.white, graphics.screen);
	graphics.drawRect(x, y + 4, 1, 5, graphics.white, graphics.screen);

	graphics.drawRect(x - 1, y - 1, 3, 3, graphics.black, graphics.screen);
	graphics.drawRect(x, y, 1, 1, graphics.red, graphics.screen);
}
// Velocity of a shot fired by the player: the weapon's own values, or the Bob -> cursor
// direction when aiming with the mouse. Shared by addBullet() and the grenade preview,
// so the preview line always matches the real throw.
static void getPlayerShotVelocity(float dx, float dy, float *vx, float *vy)
{
	*vx = dx;
	*vy = player.currentWeapon->dy + dy;

	if (!isMouseAiming())
	{
		return;
	}

	float speed = fabsf(dx);

	if (player.currentWeapon->dy)
	{
		// Lobbed weapon (grenades): keeps its upward kick and gravity,
		// the cursor decides the direction of the throw
		*vx = aimDirX * speed;
		*vy = player.currentWeapon->dy + dy + (aimDirY * speed);
		Math::limitFloat(vy, -GRENADE_MAX_VY, GRENADE_MAX_VY);
	}
	else
	{
		// Straight shots. dy is the sideways offset of a pellet (spread: +-2),
		// applied perpendicular to the aim so the fan opens around the cursor
		*vx = (aimDirX * speed) - (aimDirY * dy);
		*vy = (aimDirY * speed) + (aimDirX * dy);
	}
}
// ---- MOUSE AIM END ----

// ---- ENEMY GRENADE BEGIN ----
/*
	Enemy grenade throw. The enemy works out the launch speed and angle that land on the player
	with the same physics as doBullets() (gravity 0.1 per frame). Far or high targets need more
	speed, which the AI pays for by holding the grenade longer before it throws.
	power 0 = plain lob (ENEMY_GRENADE_MIN_SPEED), power 1 = the strongest throw (ENEMY_GRENADE_MAX_SPEED).
*/
static const float BULLET_GRAVITY = 0.1f;      // same value doBullets() adds to dy each frame
static const float ENEMY_GRENADE_MIN_SPEED = 5.0f;  // plain lob: reaches about 250 px on level ground
static const float ENEMY_GRENADE_MAX_SPEED = 10.0f; // full power: reaches about 1000 px on level ground
static bool enemyThrowActive = false;
static float enemyThrowDX = 0.0f;
static float enemyThrowDY = 0.0f;
// ---- ENEMY GRENADE END ----

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

	// Mouse aiming: the player's shots leave along the Bob -> cursor vector
	if (owner == &player)
	{
		getPlayerShotVelocity(dx, dy, &bullet->dx, &bullet->dy);
	}

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

	// Enemy grenade thrown with addEnemyGrenade(): use the launch velocity it worked out
	if (enemyThrowActive)
	{
		bullet->dx = enemyThrowDX;
		bullet->dy = enemyThrowDY;
	}

	map.addBullet(bullet);

	// Fogonazo al disparar (las granadas no lo llevan)
	if (!(bullet->flags & ENT_EXPLODES))
	{
		for (int i = 0 ; i < 2 ; i++)
		{
			float fx = Math::rrand(-10, 10); fx /= 10;
			float fy = Math::rrand(-10, 10); fy /= 10;
			// enemy shots (Droid Laser included) get a red flash, same as their trail
			auto flashColor = (owner != &player) ? graphics.red : graphics.yellow;
			map.addParticle(bullet->x, bullet->y, (bullet->dx * 0.3f) + fx, fy, Math::rrand(3, 8), flashColor, NULL, 0);
		}
	}

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

		// straight shots (not lobbed ones) warn utility droids standing in their line of fire
		if (bullet->flags & ENT_WEIGHTLESS)
		{
			notifyPlayerShot(player.x + (player.width / 2.0f), player.y + (player.height / 2.0f), bullet->dx, bullet->dy);
		}
		
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

// ---- ENEMY GRENADE BEGIN ----
// Launch velocity that lands on a target X pixels away (X > 0) and h pixels above the launch point
// with speed v. vx is a magnitude (the caller picks the side), vy is negative when going up.
// Returns false if v cannot reach the target; then it gives the lob that gets closest.
static bool solveGrenadeThrow(float X, float h, float v, float *vx, float *vy, float *flightTime)
{
	if (X < 1.0f)
	{
		X = 1.0f;
	}

	// tan(angle) is the smaller root of A u^2 - X u + (h + A) = 0 (the flatter of the two arcs)
	float A = (BULLET_GRAVITY * X * X) / (2.0f * v * v);
	float disc = (X * X) - (4.0f * A * (h + A));
	bool reachable = (disc >= 0.0f);
	float u;

	if (reachable)
	{
		u = (X - sqrtf(disc)) / (2.0f * A);
	}
	else
	{
		u = X / (2.0f * A);
	}

	float c = 1.0f / sqrtf(1.0f + (u * u));

	*vx = v * c;
	*vy = -u * v * c;
	*flightTime = X / *vx;

	return reachable;
}

// Can this enemy hit the player with a grenade from where it stands? If so, power (0 to 1) is how
// much charge the throw needs: the AI holds the grenade for that long. False = out of reach.
bool planEnemyGrenade(Entity *owner, float *power)
{
	float sx = owner->x + (owner->width / 2);
	float sy = owner->y + (owner->height / 2);
	float tx = player.x + (player.width / 2);
	float ty = player.y + (player.height / 2);

	float X = fabsf(tx - sx);
	float h = sy - ty;

	float vMax = ENEMY_GRENADE_MAX_SPEED;

	// the grenade should not blow up in the air before it gets there: go faster (flatter) if the
	// flight is longer than its life. If even full speed is not enough it is thrown anyway.
	float fuse = 100000.0f;

	if (owner->currentWeapon->health > 0)
	{
		fuse = owner->currentWeapon->health * 0.85f;
	}

	// least speed that reaches the target (with some margin), never less than a plain lob
	float v = sqrtf(BULLET_GRAVITY * (h + sqrtf((h * h) + (X * X)))) * 1.12f;

	if (v < ENEMY_GRENADE_MIN_SPEED)
	{
		v = ENEMY_GRENADE_MIN_SPEED;
	}

	float vx, vy, t;
	bool ok = false;

	for (int i = 0 ; i < 40 ; i++)
	{
		if (v > vMax)
		{
			v = vMax;
		}

		ok = solveGrenadeThrow(X, h, v, &vx, &vy, &t);

		if ((t <= fuse) || (v >= vMax))
		{
			break;
		}

		v *= 1.08f;
	}

	if (!ok)
	{
		return false;
	}

	float p = (v - ENEMY_GRENADE_MIN_SPEED) / (ENEMY_GRENADE_MAX_SPEED - ENEMY_GRENADE_MIN_SPEED);

	if (p < 0.0f)
	{
		p = 0.0f;
	}

	if (p > 1.0f)
	{
		p = 1.0f;
	}

	*power = p;

	return true;
}

// Throws the grenade the enemy is holding at the player's current position
void addEnemyGrenade(Entity *owner, float power)
{
	if (power < 0.0f)
	{
		power = 0.0f;
	}

	if (power > 1.0f)
	{
		power = 1.0f;
	}

	float v = ENEMY_GRENADE_MIN_SPEED + ((ENEMY_GRENADE_MAX_SPEED - ENEMY_GRENADE_MIN_SPEED) * power);

	float sx = owner->x + (owner->width / 2);
	float sy = owner->y + (owner->height / 2);
	float tx = player.x + (player.width / 2);
	float ty = player.y + (player.height / 2);

	// aim error: smaller on higher difficulties
	int error = 24 - (game.skill * 6);

	if (error < 4)
	{
		error = 4;
	}

	tx += Math::rrand(-error, error);
	ty += Math::rrand(-error, error);

	float vx, vy, t;

	solveGrenadeThrow(fabsf(tx - sx), sy - ty, v, &vx, &vy, &t);

	enemyThrowDX = (tx < sx) ? -vx : vx;
	enemyThrowDY = vy;
	enemyThrowActive = true;

	addBullet(owner, 0, 0);

	enemyThrowActive = false;
}

// Aims one enemy shot at a target angle relative to the line to the player.
// Used by the Eye Droid sweep fan: the angle is the offset of that bullet inside the fan.
void addEnemyAimedShot(Entity *owner, float angleDegrees)
{
	float ax = (player.x + (player.width / 2.0f)) - (owner->x + (owner->width / 2.0f));
	float ay = (player.y + (player.height / 2.0f)) - (owner->y + (owner->height / 2.0f));
	float len = sqrtf((ax * ax) + (ay * ay));

	if (len < 1.0f)
		len = 1.0f;

	float dirX = ax / len;
	float dirY = ay / len;
	float rad = (angleDegrees * 3.14159265f) / 180.0f;
	float rotX = (dirX * cosf(rad)) - (dirY * sinf(rad));
	float rotY = (dirX * sinf(rad)) + (dirY * cosf(rad));
	float speed = (float)owner->currentWeapon->getSpeed(owner->face);

	addBullet(owner, rotX * speed, rotY * speed);
}
// ---- ENEMY GRENADE END ----

// ---- ENEMY BEAM BEGIN ----
// One hit of an Eye Droid's continuous beam. The beam itself is traced and drawn in enemies.cpp; the hurt goes through the
// same bullet collisions as any other enemy shot, so whatever happens to Bob when he is shot (damage, sound, death) happens
// here too. The bullet does not move and lives a single frame, placed inside Bob where the beam touches him.
void addDroidBeamHit(Entity *owner, float x, float y, int damage)
{
	Entity *bullet = new Entity();

	bullet->x = x;
	bullet->y = y;
	bullet->setName(owner->currentWeapon->name);
	bullet->id = owner->currentWeapon->id;
	bullet->dx = 0;
	bullet->dy = 0;
	bullet->next = NULL;
	bullet->health = 1;
	bullet->damage = damage;
	bullet->setSprites(owner->currentWeapon->sprite[0], owner->currentWeapon->sprite[1], owner->currentWeapon->sprite[1]);
	bullet->face = owner->face;
	bullet->owner = owner;
	bullet->flags = owner->currentWeapon->flags + ENT_BULLET + ((owner->flags & ENT_BOSS) ? ENT_BOSS : 0);
	bullet->deathSound = -1;

	map.addBullet(bullet);
}
// ---- ENEMY BEAM END ----

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

	// Mas chispas cuanto mas dano hace la bala (entre 4 y 9)
	int damage = bullet->damage;
	if (damage < 0) damage = 0;
	if (damage > 10) damage = 10;

	addImpactSparks(bullet, 4 + (damage / 2));
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
	// Chispas solo en rebotes rapidos (una granada casi quieta no las genera)
	if ((dx > 3) || (dx < -3) || (dy > 3) || (dy < -3))
	{
		addImpactSparks(bullet, 2);
	}

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

// ---- BULLET TRAIL BEGIN ----
/*
	Short tail of sparks behind fast, straight shots. On a diagonal the tail also shows
	which way they are going. Grenades, gravity-driven shots
	(rocks, stalactites) and bullets that already have their own trail are left alone.
*/
static const float TRAIL_MIN_SPEED = 4.0f;  // slower shots get no tail
static const int TRAIL_LIFE_MIN = 2;        // frames each spark lives (tail length = speed * life)
static const int TRAIL_LIFE_MAX = 5;

static void addBulletTrail(Entity *bullet, int screenX, int screenY)
{
	if (!game.bulletTrail)
	{
		return;
	}

	if (!(bullet->flags & ENT_WEIGHTLESS))
	{
		return;
	}

	if (bullet->flags & (ENT_EXPLODES | ENT_ONFIRE | ENT_FIRETRAIL | ENT_PARTICLETRAIL))
	{
		return;
	}

	// no sparks for bullets that are off screen
	if ((screenX < -20) || (screenY < -20) || (screenX > graphics.logicalW() + 20) || (screenY > graphics.logicalH() + 20))
	{
		return;
	}

	float speed = sqrtf((bullet->dx * bullet->dx) + (bullet->dy * bullet->dy));

	if (speed < TRAIL_MIN_SPEED)
	{
		return;
	}

	auto color = graphics.yellow;

	if (bullet->owner != &player)
	{
		color = graphics.red;
	}
	else if (bullet->id == WP_LASER)
	{
		color = graphics.cyan;
	}

	// sparks spread over the last step so the tail has no gaps at high speed
	int count = (speed < 8.0f) ? 2 : 3;

	for (int i = 0 ; i < count ; i++)
	{
		float f = (float)i / count;
		map.addParticle(bullet->x - (bullet->dx * f), bullet->y - (bullet->dy * f), 0, 0, Math::rrand(TRAIL_LIFE_MIN, TRAIL_LIFE_MAX), color, NULL, PAR_WEIGHTLESS);
	}
}
// ---- BULLET TRAIL END ----

// ---- BULLET ROTATION BEGIN ----
/*
	Shots that fly without gravity (mouse aiming, aimed enemy weapons, rockets, plasma...) are
	drawn rotated to match their direction. Each bullet only has a "right" and a "left" image,
	so the image is turned by the angle between that base direction and the velocity. Grenades,
	rocks and other gravity-driven shots are drawn as before. The rotated copy is built for the
	frame and freed right after drawing (bullets are tiny, so this is cheap).
*/
static const float BULLET_PI = 3.14159265f;
static const float BULLET_MIN_ANGLE = 0.09f; // about 5 degrees: smaller tilts keep the original image

static bool getBulletDrawAngle(Entity *bullet, float *angle)
{
	if (!(bullet->flags & ENT_WEIGHTLESS))
	{
		return false;
	}

	if (bullet->id == WP_STALAGTITE)
	{
		return false;
	}

	if (((bullet->dx * bullet->dx) + (bullet->dy * bullet->dy)) < 1.0f)
	{
		return false;
	}

	// the right image points right (face 0), the left image points left (face 1)
	float base = (bullet->face == 0) ? 0.0f : BULLET_PI;
	float a = atan2f(bullet->dy, bullet->dx) - base;

	while (a > BULLET_PI)
	{
		a -= 2.0f * BULLET_PI;
	}

	while (a < -BULLET_PI)
	{
		a += 2.0f * BULLET_PI;
	}

	if (fabsf(a) < BULLET_MIN_ANGLE)
	{
		return false;
	}

	*angle = a;
	return true;
}

// Returns a new surface with src turned by angle (radians, clockwise on screen), or NULL
// if it can't be done. The caller frees it. Only 32 bit images are handled.
static SDL_Surface *createRotatedBulletImage(SDL_Surface *src, float angle)
{
	if ((src == NULL) || (src->format->BytesPerPixel != 4))
	{
		return NULL;
	}

	float c = cosf(angle);
	float s = sinf(angle);

	// 2x images: work out the size in logical units, then multiply by the ratio
	// so the result keeps an exact logical size (src->w / lw is 1 for normal images)
	int lw = graphics.getLogicalWidth(src);
	int lh = graphics.getLogicalHeight(src);
	int k = (lw > 0) ? (src->w / lw) : 1;

	if (k < 1)
	{
		k = 1;
	}

	int w = ((int)ceilf((lw * fabsf(c)) + (lh * fabsf(s))) + 1) * k;
	int h = ((int)ceilf((lw * fabsf(s)) + (lh * fabsf(c))) + 1) * k;

	SDL_Surface *dest = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	if (dest == NULL)
	{
		return NULL;
	}

	SDL_SetSurfaceBlendMode(dest, SDL_BLENDMODE_BLEND);

	bool mustLock = SDL_MUSTLOCK(src);

	if (mustLock)
	{
		SDL_LockSurface(src);
	}

	Uint32 key = 0;
	bool hasKey = (SDL_GetColorKey(src, &key) == 0);
	bool hasAlpha = (src->format->Amask != 0);

	float srcCX = (src->w - 1) / 2.0f;
	float srcCY = (src->h - 1) / 2.0f;
	float destCX = (w - 1) / 2.0f;
	float destCY = (h - 1) / 2.0f;

	SDL_LockSurface(dest);

	for (int y = 0 ; y < h ; y++)
	{
		for (int x = 0 ; x < w ; x++)
		{
			float u = x - destCX;
			float v = y - destCY;

			// inverse rotation: where does this pixel come from in the original?
			int sx = (int)floorf((u * c) + (v * s) + srcCX + 0.5f);
			int sy = (int)floorf((-u * s) + (v * c) + srcCY + 0.5f);

			if ((sx < 0) || (sy < 0) || (sx >= src->w) || (sy >= src->h))
			{
				continue;
			}

			Uint32 pixel = *(Uint32*)((Uint8*)src->pixels + (sy * src->pitch) + (sx * 4));

			if (hasKey && (pixel == key))
			{
				continue;
			}

			Uint8 r, g, b, a;
			SDL_GetRGBA(pixel, src->format, &r, &g, &b, &a);

			if (!hasAlpha)
			{
				a = 255;

				// no color key on the image: the game treats black as transparent
				if ((!hasKey) && (r == 0) && (g == 0) && (b == 0))
				{
					continue;
				}
			}

			if (a == 0)
			{
				continue;
			}

			*(Uint32*)((Uint8*)dest->pixels + (y * dest->pitch) + (x * 4)) = SDL_MapRGBA(dest->format, r, g, b, a);
		}
	}

	SDL_UnlockSurface(dest);

	if (mustLock)
	{
		SDL_UnlockSurface(src);
	}

	// keep the logical size so a rotated 2x image is still drawn at the right scale
	graphics.setLogicalSize(dest, w / k, h / k);

	return dest;
}

static void drawBullet(Entity *bullet, int x, int y)
{
	SDL_Surface *image = bullet->getFaceImage();
	float angle;

	if (getBulletDrawAngle(bullet, &angle))
	{
		SDL_Surface *rotated = createRotatedBulletImage(image, angle);

		if (rotated != NULL)
		{
			graphics.blit(rotated, x, y, graphics.screen, true);
			SDL_FreeSurface(rotated);
			return;
		}
	}

	graphics.blit(image, x, y, graphics.screen, true);
}
// ---- BULLET ROTATION END ----

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

		drawBullet(bullet, x, y);
		bullet->animate();

		addBulletTrail(bullet, x, y);

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
			addColorParticles(bullet->x, bullet->y, 3, -1);
		}

		if (bullet->owner == &player)
		{
			if ((x < -160) || (y < -120) || (x > graphics.logicalW() + 160) || (y > graphics.logicalH() + 120))
			{
				removeBullet(bullet);
			}
		}

		// utility droids' repulsor field bounces the player's grenades (before they move into the droid)
		deflectGrenadeAtDroids(bullet);

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

// Launch values for a given charge (0..1): used by the real throw and by the preview
static void getGrenadeThrow(float t, float *dx, float *dyExtra)
{
	*dx = player.currentWeapon->getSpeed(player.face) * (1.0f + (GRENADE_SPEED_BOOST * t));
	*dyExtra = GRENADE_ARC_BOOST * t;
}

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

	// Grenades cannot be thrown underwater (addBullet ignores them), so do not spend one
	if (player.environment == ENV_WATER)
	{
		grenadeCharge = 0;
		return true;
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

		float dx, dyExtra;
		getGrenadeThrow(t, &dx, &dyExtra);

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

// ---- GRENADE PREVIEW BEGIN ----
/*
	Trajectory preview while charging a grenade: a line of soft dots that follows the same
	physics as doBullets() (move x, check brick, move y, check brick, gravity) and stops at
	the first bounce, where a small ring is drawn. It only looks at bricks: enemies, moving
	platforms and obstacles are ignored.
*/
static const int PREVIEW_MAX_DOTS = 36;
static const float PREVIEW_DOT_SPACING = 11.0f; // pixels between dots
static const float PREVIEW_START_GAP = 16.0f;   // keep Bob's body clear
static const int PREVIEW_ALPHA_START = 200;     // nearest dot
static const int PREVIEW_ALPHA_END = 40;        // farthest dot
static const int PREVIEW_ALPHA_RING = 210;

static SDL_Surface *previewDot = NULL;
static SDL_Surface *previewRing = NULL;

static void createPreviewSprites()
{
	static bool tried = false;

	if (tried)
	{
		return;
	}

	tried = true;

	previewDot = SDL_CreateRGBSurface(0, 4, 4, 32, 0xff0000, 0xff00, 0xff, 0xff000000);
	previewRing = SDL_CreateRGBSurface(0, 10, 10, 32, 0xff0000, 0xff00, 0xff, 0xff000000);

	if ((previewDot == NULL) || (previewRing == NULL))
	{
		if (previewDot != NULL) SDL_FreeSurface(previewDot);
		if (previewRing != NULL) SDL_FreeSurface(previewRing);
		previewDot = previewRing = NULL;
		return;
	}

	SDL_SetSurfaceBlendMode(previewDot, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceBlendMode(previewRing, SDL_BLENDMODE_BLEND);

	// Dot: solid 2x2 core with a soft one-pixel edge (round look, no corners)
	SDL_FillRect(previewDot, NULL, 0x00ffffff);
	SDL_Rect core = {1, 1, 2, 2};
	SDL_FillRect(previewDot, &core, 0xffffffff);
	SDL_Rect edge[8] = {{1, 0, 1, 1}, {2, 0, 1, 1}, {1, 3, 1, 1}, {2, 3, 1, 1}, {0, 1, 1, 1}, {0, 2, 1, 1}, {3, 1, 1, 1}, {3, 2, 1, 1}};
	for (int i = 0 ; i < 8 ; i++)
	{
		SDL_FillRect(previewDot, &edge[i], 0x70ffffff);
	}

	// Ring: anti-aliased circle, one pixel thick
	SDL_FillRect(previewRing, NULL, 0x00ffffff);
	for (int py = 0 ; py < 10 ; py++)
	{
		for (int px = 0 ; px < 10 ; px++)
		{
			float ddx = px - 4.5f;
			float ddy = py - 4.5f;
			float a = 1.0f - fabsf(sqrtf((ddx * ddx) + (ddy * ddy)) - 4.0f);

			if (a > 0)
			{
				SDL_Rect pixel = {px, py, 1, 1};
				Uint32 alpha = (Uint32)(a * 255);
				SDL_FillRect(previewRing, &pixel, (alpha << 24) | 0x00ffffff);
			}
		}
	}
}

// Same test bulletHasCollided() does, plus the edges of the map
static bool previewSolid(float x, float y)
{
	if ((x < 0) || (y < 0))
	{
		return true;
	}

	int bx = (int)x >> BRICKSHIFT;
	int by = (int)y >> BRICKSHIFT;

	if ((bx >= MAPWIDTH) || (by >= MAPHEIGHT))
	{
		return true;
	}

	return map.isSolid(bx, by);
}

static void drawGrenadeTrajectory(float t)
{
	if (!game.grenadePreview)
	{
		return;
	}

	createPreviewSprites();

	if ((previewDot == NULL) || (previewRing == NULL))
	{
		return;
	}

	// Launch velocity, exactly as addBullet() computes it
	float dx, dyExtra;
	getGrenadeThrow(t, &dx, &dyExtra);

	float vx, vy;
	getPlayerShotVelocity(dx, dyExtra, &vx, &vy);

	int tdx, tdy;
	getTrainMotion(&player, tdx, tdy);
	vx += player.dx - tdx;
	vy += -tdy;

	float gravity = (player.currentWeapon->flags & ENT_WEIGHTLESS) ? 0.0f : 0.1f;

	int maxSteps = player.currentWeapon->health; // lifetime in frames
	if (maxSteps > 300) maxSteps = 300;
	if (maxSteps < 1) maxSteps = 1;

	float x = player.x + (player.width / 2);
	float y = player.y + (player.height / 2);
	float lastX = x;
	float lastY = y;

	float dotX[PREVIEW_MAX_DOTS];
	float dotY[PREVIEW_MAX_DOTS];
	int dots = 0;
	float travelled = 0.0f;
	float nextDot = PREVIEW_START_GAP;
	bool bounced = false;

	for (int step = 0 ; (step < maxSteps) && (dots < PREVIEW_MAX_DOTS) ; step++)
	{
		x += vx;
		if (previewSolid(x, y))
		{
			bounced = true;
			break;
		}

		y += vy;
		if (previewSolid(x, y))
		{
			bounced = true;
			break;
		}

		vy += gravity;

		float sx = x - lastX;
		float sy = y - lastY;
		float segLen = sqrtf((sx * sx) + (sy * sy));
		float before = travelled;
		travelled += segLen;

		// dots are placed along the segment so the spacing stays even at any speed
		while ((travelled >= nextDot) && (dots < PREVIEW_MAX_DOTS))
		{
			float f = (segLen > 0) ? ((nextDot - before) / segLen) : 1.0f;
			dotX[dots] = lastX + (sx * f);
			dotY[dots] = lastY + (sy * f);
			dots++;
			nextDot += PREVIEW_DOT_SPACING;
		}

		lastX = x;
		lastY = y;
	}

	for (int i = 0 ; i < dots ; i++)
	{
		int px = (int)(dotX[i] - engine.playerPosX);
		int py = (int)(dotY[i] - engine.playerPosY);

		if ((px < 4) || (py < 4) || (px > graphics.logicalW() - 4) || (py > graphics.logicalH() - 4))
		{
			continue;
		}

		int alpha = PREVIEW_ALPHA_START - (((PREVIEW_ALPHA_START - PREVIEW_ALPHA_END) * i) / ((dots > 1) ? (dots - 1) : 1));
		SDL_SetSurfaceAlphaMod(previewDot, (Uint8)alpha);
		graphics.blit(previewDot, px - 2, py - 2, graphics.screen, false);
	}

	// Ring where the grenade first hits something
	if ((bounced) && (travelled > PREVIEW_START_GAP))
	{
		int px = (int)(lastX - engine.playerPosX);
		int py = (int)(lastY - engine.playerPosY);

		if ((px >= 5) && (py >= 5) && (px <= graphics.logicalW() - 5) && (py <= graphics.logicalH() - 5))
		{
			SDL_SetSurfaceAlphaMod(previewRing, PREVIEW_ALPHA_RING);
			graphics.blit(previewRing, px - 5, py - 5, graphics.screen, false);
		}
	}
}
// ---- GRENADE PREVIEW END ----

void drawGrenadeCharge()
{
	if (grenadeCharge <= 0)
		return;

	drawGrenadeTrajectory((float)grenadeCharge / GRENADE_CHARGE_MAX);

	int x = (int)(player.x - engine.playerPosX);
	int y = (int)(player.y - engine.playerPosY) - 10;
	int w = 24;
	int fill = (w * grenadeCharge) / GRENADE_CHARGE_MAX;

	graphics.drawRect(x - 1, y - 1, w + 2, 5, graphics.black, graphics.screen);
	graphics.drawRect(x, y, fill, 3, (grenadeCharge >= GRENADE_CHARGE_MAX) ? graphics.red : graphics.yellow, graphics.screen);
}
