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

#include <map>
#include <algorithm>
#include "enemies.h"

// Enemy grenade throws (bullets.cpp): plan says how much charge a throw needs, add makes the throw
extern bool planEnemyGrenade(Entity *owner, float *power);
extern void addEnemyGrenade(Entity *owner, float power);

// Extra per-enemy AI state (kept here so no other header has to change)
struct EnemyAIState
{
	int telegraph;   // frames left of the "about to shoot" warning
	bool fireNow;    // the warning is over: fire as soon as possible
	int fireTimeout; // ...but give up if it can't fire within this many frames
	float awareness; // 0 = calm, AWARE_SUSPICIOUS = "?", AWARE_ALERT = "!"
	int lostSight;   // frames since the player was last noticed
	bool alerted;    // reached full alert (holds the alert longer)
	bool canSee;     // clear line of sight to the player this frame
	int lastX, lastY; // where the player was last seen
	int prevX;       // position last frame (to detect getting stuck)
	int stuck;       // frames spent unable to move towards the target
	int maxHealth;   // health the enemy spawned with (for the health bar)
	int burstLeft;   // shots still to fire in the current burst
	int burstTimeout; // safety: a burst can never last longer than this
	unsigned int attackStamp; // last frame this enemy was busy attacking (attack turns)
	int rank;        // RANK_SOLDIER / RANK_VETERAN / RANK_SERGEANT
	Entity *squadLeader; // the sergeant this soldier follows (NULL = none)
	int confused;    // frames left of disorientation after the sergeant died
	int panicDir;    // direction (-1 / 1) it stumbles in while confused
	int grenadeX, grenadeY; // position of last grenade thrown (for avoidance)
	int grenadeTimer; // frames since grenade was thrown (to know when it's safe)
	int telegraphTotal; // length of the current warning (for the grenade charge bar)
	bool charging;   // the current warning is a grenade being held

	EnemyAIState() : telegraph(0), fireNow(false), fireTimeout(0), awareness(0), lostSight(0), alerted(false), canSee(false), lastX(0), lastY(0), prevX(0), stuck(0), maxHealth(0), burstLeft(0), burstTimeout(0), attackStamp(0), rank(0), squadLeader(NULL), confused(0), panicDir(1), grenadeX(0), grenadeY(0), grenadeTimer(0), telegraphTotal(0), charging(false) {}
};

static std::map<Entity*, EnemyAIState> aiState;

// Awareness (Metal Gear style): builds up while an enemy can see you, faster the
// closer you are. "?" = suspicious, goes to investigate. "!" = alert, attacks.
// After losing you it stays alert for a while, then calms down slowly.
static const float AWARE_MAX = 100.0f;
static const float AWARE_SUSPICIOUS = 30.0f;
static const float AWARE_ALERT = 100.0f;
static const float AWARE_DECAY = 0.16f;  // lost per frame once the hold time is over
static const int ALERT_HOLD = 360;       // frames an alerted enemy keeps looking before calming down
static const int SUSPICIOUS_HOLD = 120;  // same for an enemy that was only suspicious
static const int VISION_RANGE = 720;     // farthest they can notice you
static const int BEHIND_RANGE = 96;      // they only notice you behind their back this close
static const float ALLY_ALERT_LEVEL = 65.0f;  // what allies get told when someone spots you / is shot
static const float HEAR_LEVEL = 45.0f;        // what a nearby gunshot does
static const int ALERT_RANGE_X = 300;   // chain alert radius
static const int ALERT_RANGE_Y = 150;
static const int HEAR_RANGE_X = 420;    // how far a gunshot is heard
static const int HEAR_RANGE_Y = 220;
static const int BURST_PAUSE = 50;      // extra wait after a burst

// Preferred distance from player, by weapon type (pixels)
static const int DIST_CLOSE = 150;      // pistol, machine gun
static const int DIST_MEDIUM = 300;     // laser, spread
static const int DIST_FAR = 450;        // grenades, rockets

// Separation between enemies (pixels) to avoid stacking
static const int ENEMY_SEPARATION = 64;

// Grenade safety: frames to wait after throwing before throwing another
static const int GRENADE_COOLDOWN = 120; // about 2 seconds
static const int GRENADE_SAFE_DISTANCE = 150; // stay this far from own grenade
static const int GRENADE_CHARGE_FRAMES = 45;  // extra hold time of a full-power throw (about 0.75 s)

static unsigned int aiFrame = 0;        // frame counter, used for the attack turns
static int attackCooldown = 0;          // frames until another enemy may start attacking
static int lastPlayerReload = 0;        // used to notice when the player fires

static EnemyAIState &getAIState(Entity *enemy)
{
	return aiState[enemy];
}

// Get preferred distance from player based on weapon type
static int getPreferredDistance(Entity *enemy)
{
	Weapon *w = enemy->currentWeapon;

	// Grenades and rockets: stay far
	if ((w == &weapon[WP_GRENADES]) || (w == &weapon[WP_ROCKETS]) ||
	    (w == &weapon[WP_ALIENGRENADE]))
		return DIST_FAR;

	// Laser and spread: medium distance
	if ((w == &weapon[WP_LASER]) || (w == &weapon[WP_SPREAD]) ||
	    (w == &weapon[WP_ALIENSPREAD]) || (w == &weapon[WP_ALIENLASER]))
		return DIST_MEDIUM;

	// Pistol and machine gun: close
	return DIST_CLOSE;
}

// Check if another enemy is too close (for separation)
static bool isEnemyTooClose(Entity *enemy, Entity *other)
{
	if (other == enemy)
		return false;

	if (other->health <= 0)
		return false;

	float dx = fabs(enemy->x - other->x);
	float dy = fabs(enemy->y - other->y);

	return ((dx < ENEMY_SEPARATION) && (dy < ENEMY_SEPARATION));
}

static bool isGrenadeWeapon(Entity *enemy)
{
	Weapon *w = enemy->currentWeapon;

	return ((w == &weapon[WP_GRENADES]) || (w == &weapon[WP_ALIENGRENADE]));
}

// Check if it's safe to throw a grenade (won't hurt self or nearby allies)
static bool isGrenadeSafe(Entity *enemy)
{
	EnemyAIState &st = getAIState(enemy);
	Weapon *w = enemy->currentWeapon;
	int grenadeRadius = 50; // standard grenade explosion radius

	// Check if cooldown hasn't expired (strategic: wait for previous grenade to explode)
	if (st.grenadeTimer > 0)
		return false;

	// Check if enemy is too close to player (would hurt self)
	float distToPlayer = fabs(enemy->x - player.x);
	if (distToPlayer < grenadeRadius)
		return false;

	// Check if enemy would be hurt by its own grenade
	// Grenades land near the player, so if enemy is too close to player, it's unsafe
	if (distToPlayer < (grenadeRadius + 100))
		return false;

	// Check if any nearby ally would be hurt
	Entity *other = (Entity*)map.enemyList.getHead();
	while (other->next != NULL)
	{
		other = (Entity*)other->next;

		if (other == enemy)
			continue;

		if (other->health <= 0)
			continue;

		// Don't care about enemies that are already far away
		float dx = fabs(enemy->x - other->x);
		float dy = fabs(enemy->y - other->y);

		if ((dx < grenadeRadius) && (dy < grenadeRadius))
			return false; // ally would be hurt
	}

	return true;
}

// Check if there's a player grenade nearby and move away from it
static bool avoidPlayerGrenade(Entity *enemy)
{
	Entity *bullet = (Entity*)map.bulletList.getHead();
	int grenadeRadius = 50;
	int safeDistance = 150;

	while (bullet->next != NULL)
	{
		bullet = (Entity*)bullet->next;

		// Only care about player's grenades
		if (bullet->owner != &player)
			continue;

		if (!(bullet->flags & ENT_EXPLODES))
			continue;

		// Check distance to this grenade
		float dx = fabs(enemy->x - bullet->x);
		float dy = fabs(enemy->y - bullet->y);

		if ((dx < safeDistance) && (dy < safeDistance))
		{
			// Move away from the grenade
			if (bullet->x < enemy->x)
				enemy->tx = (int)(enemy->x + 100);
			else
				enemy->tx = (int)(enemy->x - 100);

			return true; // avoiding a grenade
		}
	}

	return false; // no grenades nearby
}

// ---------------------------------------------------------------------------
// Ranks & squads. Only ground blobs with a low base health can be promoted.
//   Soldier : base hp (from defEnemies)
//   Veteran : base + 2 hp, longer bursts, shorter warning, always shows a health bar
//   Sergeant: base + 4 hp, leads a squad. When it spots you (or is shot) the whole squad
//             goes to full alert. Kill it and the squad panics for a few seconds.
// ---------------------------------------------------------------------------
static const int RANK_SOLDIER = 0;
static const int RANK_VETERAN = 1;
static const int RANK_SERGEANT = 2;

static const int VETERAN_BONUS_HEALTH = 2;   // extra hp on top of the base health
static const int SERGEANT_BONUS_HEALTH = 4;
static const int RANK_MAX_BASE_HEALTH = 3;   // tougher enemies (spiders...) are never promoted
static const int MAX_SERGEANTS = 5;        // alive at the same time on a map
static const int SQUAD_PANIC_FRAMES = 150; // how long the squad is lost without its leader

// Chances (in %) are re-rolled for every enemy that is placed. Tune to taste.
static int getVeteranChance()
{
	int chance = 10 + (game.stagesCleared * 3) + (game.skill * 4);

	return (chance > 45) ? 45 : chance;
}

static int getSergeantChance()
{
	// no sergeants in the very first missions
	if (game.stagesCleared < 2)
		return 0;

	int chance = 6 + ((game.stagesCleared - 2) * 2) + (game.skill * 2);

	return (chance > 25) ? 25 : chance;
}

// Forget everything about an enemy that is being removed (and whoever followed it)
static void forgetEnemy(Entity *enemy)
{
	aiState.erase(enemy);

	for (std::map<Entity*, EnemyAIState>::iterator it = aiState.begin() ; it != aiState.end() ; ++it)
	{
		if (it->second.squadLeader == enemy)
			it->second.squadLeader = NULL;
	}
}

// True when (part of) the enemy is inside the visible area. Enemies that are
// off screen never notice the player: no more unseen red alerts.
static bool isEnemyOnScreen(Entity *enemy)
{
	const int margin = 32;
	int sx = (int)(enemy->x - engine.playerPosX);
	int sy = (int)(enemy->y - engine.playerPosY);

	return (sx > -(enemy->width + margin)) && (sx < (graphics.screen->w + margin)) &&
	       (sy > -(enemy->height + margin)) && (sy < (graphics.screen->h + margin));
}

// Tiny pixel-art glyphs for the awareness icons
static const char * const GLYPH_ALERT[7] = {".#.", ".#.", ".#.", ".#.", ".#.", "...", ".#."};
static const char * const GLYPH_SUSPECT[7] = {".###.", "#...#", "....#", "..##.", "..#..", ".....", "..#.."};

static void drawGlyph(const char * const *rows, int w, int px, int py, Uint32 color)
{
	for (int r = 0 ; r < 7 ; r++)
	{
		for (int c = 0 ; c < w ; c++)
		{
			if (rows[r][c] == '#')
			{
				graphics.drawRect(px + (c * 2), py + (r * 2), 2, 2, color, graphics.screen);
			}
		}
	}
}

// "?" while suspicious, "!" while alert, plus a small gauge that fills as the
// enemy notices you and drains as it calms down
static void drawAwareness(Entity *enemy, int x, int y)
{
	if (enemy->health <= 0)
		return;

	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if (it == aiState.end())
		return;

	const EnemyAIState &st = it->second;

	if (st.awareness <= 0)
		return;

	int cx = x + (enemy->width / 2);
	int top = y - 26;

	if (st.awareness >= AWARE_ALERT)
	{
		Uint32 color = SDL_MapRGB(graphics.screen->format, 230, 30, 30);

		// blink while winding up a shot
		if ((st.telegraph > 0) && (((st.telegraph / 4) % 2) == 0))
			color = graphics.yellow;

		drawGlyph(GLYPH_ALERT, 3, cx - 3 + 1, top + 1, graphics.black);
		drawGlyph(GLYPH_ALERT, 3, cx - 3, top, color);

		// holding a grenade: bar that fills up until the throw
		if ((st.charging) && (st.telegraph > 0) && (st.telegraphTotal > 0))
		{
			int cw = 16;
			int cfill = (cw * (st.telegraphTotal - st.telegraph)) / st.telegraphTotal;

			if (cfill < 1)
				cfill = 1;

			graphics.drawRect(cx - (cw / 2) - 1, y - 12, cw + 2, 4, graphics.black, graphics.screen);
			graphics.drawRect(cx - (cw / 2), y - 11, cfill, 2, SDL_MapRGB(graphics.screen->format, 255, 140, 0), graphics.screen);
		}

		return;
	}

	if (st.awareness >= AWARE_SUSPICIOUS)
	{
		drawGlyph(GLYPH_SUSPECT, 5, cx - 5 + 1, top + 1, graphics.black);
		drawGlyph(GLYPH_SUSPECT, 5, cx - 5, top, graphics.yellow);
	}

	int w = 16;
	int fill = (int)((w * st.awareness) / AWARE_ALERT);

	if (fill < 1)
		fill = 1;

	Uint32 color = (st.awareness >= (AWARE_ALERT / 2)) ? SDL_MapRGB(graphics.screen->format, 255, 140, 0) : graphics.yellow;

	graphics.drawRect(cx - (w / 2) - 1, y - 12, w + 2, 4, graphics.black, graphics.screen);
	graphics.drawRect(cx - (w / 2), y - 11, fill, 2, color, graphics.screen);
}

// Small health bar over enemies that can take several hits. Only shown once
// the enemy has been damaged (ranked enemies always show it).
static void drawHealthBar(Entity *enemy, int x, int y)
{
	if (enemy->health <= 0)
		return;

	if (enemy->flags & (ENT_STATIC|ENT_IMMUNE|ENT_BOSS|ENT_GALDOV|ENT_INANIMATE))
		return;

	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if ((it == aiState.end()) || (it->second.maxHealth <= 0))
		return;

	int maxHealth = it->second.maxHealth;

	if ((enemy->health >= maxHealth) && (it->second.rank == RANK_SOLDIER))
		return;

	int w = enemy->width;

	if (w < 16) w = 16;
	if (w > 40) w = 40;

	int fill = (w * enemy->health) / maxHealth;

	if (fill < 1)
		fill = 1;

	int bx = x + (enemy->width / 2) - (w / 2);
	int by = y - 6;

	Uint32 color;

	if ((enemy->health * 2) > maxHealth)
		color = SDL_MapRGB(graphics.screen->format, 0, 200, 0);
	else if ((enemy->health * 4) > maxHealth)
		color = graphics.yellow;
	else
		color = SDL_MapRGB(graphics.screen->format, 220, 0, 0);

	graphics.drawRect(bx - 1, by - 1, w + 2, 5, graphics.black, graphics.screen);
	graphics.drawRect(bx, by, fill, 3, color, graphics.screen);
}

// Small chevrons next to the head: one = veteran, two = sergeant
static void drawChevron(int px, int py, Uint32 color)
{
	static const char * const rows[3] = {"..#..", ".#.#.", "#...#"};

	for (int r = 0 ; r < 3 ; r++)
	{
		for (int c = 0 ; c < 5 ; c++)
		{
			if (rows[r][c] == '#')
				graphics.drawRect(px + (c * 2), py + (r * 2), 2, 2, color, graphics.screen);
		}
	}
}

static void drawRankBadge(Entity *enemy, int x, int y)
{
	if (enemy->health <= 0)
		return;

	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if ((it == aiState.end()) || (it->second.rank == RANK_SOLDIER))
		return;

	int px = x - 13;
	int py = y - 8;
	int count = (it->second.rank == RANK_SERGEANT) ? 2 : 1;
	Uint32 color = (count == 2) ? SDL_MapRGB(graphics.screen->format, 255, 140, 0) : graphics.yellow;

	for (int i = 0 ; i < count ; i++)
	{
		drawChevron(px + 1, py + 1 - (i * 5), graphics.black);
		drawChevron(px, py - (i * 5), color);
	}
}

// Higher difficulty = shorter warning. Higher rank = shorter warning too.
static int getWindupFrames(int rank)
{
	int frames = 30 - (game.skill * 4) - (rank * 4);

	if (frames < 10)
		frames = 10;

	return frames;
}

Entity *getDefinedEnemy(const char *name)
{
	for (int i = 0 ; i < MAX_ENEMIES ; i++)
	{
		if (strcmp(name, defEnemy[i].name) == 0)
		{
			return &defEnemy[i];
		}
	}

	debug(("No Such Enemy '%s'\n", name));

	return NULL;
}

Entity *getEnemy(const char *name)
{
	Entity *enemy = (Entity*)map.enemyList.getHead();

	while (enemy->next != NULL)
	{
		enemy = (Entity*)enemy->next;

		if (strcmp(name, enemy->name) == 0)
		{
			return enemy;
		}
	}
	
	debug(("No Such Enemy '%s'\n", name));

	return NULL;
}

static Entity *spawnEnemyEntity(const char *name, int x, int y, int flags)
{
	Entity *defEnemy = getDefinedEnemy(name);

	if (defEnemy == NULL)
	{
		debug(("ERROR : COULDN'T FIND ENEMY '%s'!\n", name));
		return NULL;
	}

	Entity *enemy = new Entity();
	forgetEnemy(enemy);
	enemy->setName(defEnemy->name);
	enemy->setSprites(defEnemy->sprite[0], defEnemy->sprite[1], defEnemy->sprite[2]);
	enemy->currentWeapon = defEnemy->currentWeapon;
	enemy->value = defEnemy->value;
	enemy->health = defEnemy->health;
	enemy->flags = defEnemy->flags;

	enemy->place(x, y);
	enemy->setVelocity(0, 0);
	enemy->baseThink = 60;

	enemy->flags |= flags;

	getAIState(enemy).maxHealth = enemy->health;
	
	enemy->reload = 120; // Wait about seconds seconds before attacking

	if (map.data[(int)(enemy->x) >> BRICKSHIFT][(int)(enemy->y) >> BRICKSHIFT] == MAP_WATER)
	{
		enemy->environment = ENV_WATER;
	}

	map.addEnemy(enemy);

	return enemy;
}

static int countLiveRank(int rank)
{
	int count = 0;
	Entity *e = (Entity*)map.enemyList.getHead();

	while (e->next != NULL)
	{
		e = (Entity*)e->next;

		if (e->health <= 0)
			continue;

		std::map<Entity*, EnemyAIState>::iterator it = aiState.find(e);

		if ((it != aiState.end()) && (it->second.rank == rank))
			count++;
	}

	return count;
}

// Solid ground close below, and nothing solid or liquid where the soldier would stand
static bool isGoodEscortSpot(int px, int py)
{
	int tx = (px + 10) >> BRICKSHIFT;
	int ty = (py + 10) >> BRICKSHIFT;

	if ((tx < 1) || (ty < 1) || (tx >= (MAPWIDTH - 1)) || (ty >= (MAPHEIGHT - 6)))
		return false;

	if (map.isSolid(tx, ty) || map.isLiquid(tx, ty))
		return false;

	for (int i = 1 ; i <= 4 ; i++)
	{
		if (map.isLiquid(tx, ty + i))
			return false;

		if (map.isSolid(tx, ty + i))
			return true;
	}

	return false;
}

// Surround a new sergeant with a few soldiers
static void spawnEscorts(Entity *leader)
{
	static const char * const names[3] = {"Pistol Blob", "Pistol Blob", "Machine Gun Blob"};
	static const int offsets[6] = {-40, 40, -72, 72, -104, 104};

	int wanted = 2 + ((game.skill >= 2) ? 1 : 0);
	int placed = 0;

	for (int i = 0 ; (i < 6) && (placed < wanted) ; i++)
	{
		int nx = (int)leader->x + offsets[i];
		int ny = (int)leader->y;

		if (!isGoodEscortSpot(nx, ny))
			continue;

		Entity *soldier = spawnEnemyEntity(names[Math::prand() % 3], nx, ny, 0);

		if (soldier != NULL)
		{
			getAIState(soldier).squadLeader = leader;
			placed++;
		}
	}
}

// Higher ranks may carry a different weapon than their base type. The weapon is always
// taken from the same aiming style (ENT_AIMS blobs keep aimed weapons, the others keep
// straight ones), so the AI, aim and burst code keep working as before.
static const int VETERAN_WEAPON_CHANCE = 30;  // % of veterans that change weapon
static const int SERGEANT_WEAPON_CHANCE = 60; // % of sergeants that change weapon
static const int SERGEANT_ROCKET_CHANCE = 25; // % of the sergeants' aimed pools that also include rockets

static void giveRankWeapon(Entity *enemy, int rank)
{
	int chance = (rank == RANK_SERGEANT) ? SERGEANT_WEAPON_CHANCE : VETERAN_WEAPON_CHANCE;

	if ((int)(Math::prand() % 100) >= chance)
		return;

	Weapon *pool[6];
	int count = 0;

	if (enemy->flags & ENT_AIMS)
	{
		pool[count++] = &weapon[WP_AIMEDPISTOL];
		pool[count++] = &weapon[WP_AIMEDMACHINE];
		pool[count++] = &weapon[WP_ALIENSPREAD];
		pool[count++] = &weapon[WP_ALIENGRENADE];

		if ((rank == RANK_SERGEANT) && ((int)(Math::prand() % 100) < SERGEANT_ROCKET_CHANCE))
			pool[count++] = &weapon[WP_ROCKETS];
	}
	else
	{
		pool[count++] = &weapon[WP_MACHINEGUN];
		pool[count++] = &weapon[WP_ALIENLASER];
	}

	// never pick the weapon the enemy already has
	Weapon *candidates[6];
	int total = 0;

	for (int i = 0 ; i < count ; i++)
	{
		if (pool[i] != enemy->currentWeapon)
			candidates[total++] = pool[i];
	}

	if (total == 0)
		return;

	Weapon *chosen = candidates[Math::prand() % total];
	enemy->currentWeapon = chosen;

	// ENT_RAPIDFIRE belongs to the machine gun: a laser must not inherit it (or the reverse)
	if (!(enemy->flags & ENT_AIMS))
	{
		if (chosen == &weapon[WP_MACHINEGUN])
			Math::addBit(&enemy->flags, ENT_RAPIDFIRE);
		else
			Math::removeBit(&enemy->flags, ENT_RAPIDFIRE);
	}
}

// Decide the rank of a newly placed enemy (the name never changes, so objectives still count it)
static void assignRank(Entity *enemy, int flags)
{
	if (map.isBossMission)
		return;

	if (enemy->health > RANK_MAX_BASE_HEALTH)
		return;

	if (enemy->flags & (ENT_FLIES|ENT_SWIMS|ENT_STATIC|ENT_NOMOVE|ENT_BOSS|ENT_GALDOV|ENT_INANIMATE|ENT_IMMUNE))
		return;

	EnemyAIState &st = getAIState(enemy);

	// randomly appearing enemies can be veterans, but never lead a squad
	bool spawned = ((flags & ENT_SPAWNED) != 0);
	int sergeantChance = spawned ? 0 : getSergeantChance();
	int roll = Math::prand() % 100;

	if (roll < sergeantChance)
	{
		if (countLiveRank(RANK_SERGEANT) < MAX_SERGEANTS)
		{
			st.rank = RANK_SERGEANT;
			enemy->health += SERGEANT_BONUS_HEALTH;
			enemy->value *= 4;
			st.maxHealth = enemy->health;
			giveRankWeapon(enemy, RANK_SERGEANT);
			spawnEscorts(enemy);
			return;
		}

		roll = sergeantChance; // too many sergeants already: make it a veteran
	}

	if (roll < (sergeantChance + getVeteranChance()))
	{
		st.rank = RANK_VETERAN;
		enemy->health += VETERAN_BONUS_HEALTH;
		enemy->value *= 2;
		st.maxHealth = enemy->health;
		giveRankWeapon(enemy, RANK_VETERAN);
	}
}

void addEnemy(const char *name, int x, int y, int flags)
{
	Entity *enemy = spawnEnemyEntity(name, x, y, flags);

	if (enemy != NULL)
	{
		assignRank(enemy, flags);
	}
}

bool hasClearShot(Entity *enemy)
{
	int mx, my;
	float x = enemy->x + (enemy->width / 2);
	float y = enemy->y + (enemy->height / 2);
	float dx, dy;

	Math::calculateSlope(player.x + (player.width / 2), player.y + (player.height / 2), x, y, &dx, &dy);

	if ((dx == 0) && (dy == 0))
		return true;

	int steps = 0;

	while (true)
	{
		x += dx;
		y += dy;

		// safety: never loop forever if the ray misses the player
		if (++steps > 2000)
			return false;

		//graphics.blit(graphics.getSprite("AimedShot", true)->getCurrentFrame(), (int)(x - engine.playerPosX), (int)(y - engine.playerPosY), graphics.screen, true);

		mx = (int)(x) >> BRICKSHIFT;
		my = (int)(y) >> BRICKSHIFT;

		if ((mx < 0) || (my < 0) || (mx >= MAPWIDTH) || (my >= MAPHEIGHT))
			return false;

		if (map.isSolid(mx, my))
			return false;

		if (Collision::collision(x, y, 3, 3, (int)player.x, (int)player.y, player.width, player.height))
			break;
	}

	return true;
}

// Vertical velocity to add to a straight shot so it heads towards the player.
// Only used for weapons that fly straight (weightless, no built-in dy) and for
// enemies that don't already aim by themselves (ENT_AIMS).
static float getAimDY(Entity *enemy)
{
	if (enemy->flags & ENT_AIMS)
		return 0;

	if (enemy->currentWeapon->dy != 0)
		return 0;

	if (!(enemy->currentWeapon->flags & ENT_WEIGHTLESS))
		return 0;

	float ex = enemy->x + (enemy->width / 2);
	float ey = enemy->y + (enemy->height / 2);
	float px = player.x + (player.width / 2);
	float py = player.y + (player.height / 2);

	float speed = fabs((float)enemy->currentWeapon->getSpeed(enemy->face));

	// Lead the target: aim at where the player will be when the shot arrives.
	// Easy = no prediction, Extreme = full prediction.
	if (speed >= 1)
	{
		float lead = game.skill / 3.0f;

		if (lead > 1)
			lead = 1;

		float t = fabs(px - ex) / speed;

		if (t > 40)
			t = 40;

		px += player.dx * t * lead;

		t = fabs(px - ex) / speed;

		if (t > 40)
			t = 40;

		// gravity slows a jump down, so only lead about half of the vertical speed
		float offset = player.dy * t * 0.5f * lead;

		Math::limitFloat(&offset, -48, 48);

		py += offset;
	}

	float dist = fabs(px - ex);

	if (dist < 32)
		dist = 32;

	float dy = ((py - ey) / dist) * speed;

	Math::limitFloat(&dy, -3, 3);

	return dy;
}

// Tells every free enemy inside the given area where the player is
static void alertEnemiesNear(int cx, int cy, int rangeX, int rangeY, Entity *skip, float awareLevel)
{
	Entity *other = (Entity*)map.enemyList.getHead();

	while (other->next != NULL)
	{
		other = (Entity*)other->next;

		if (other == skip)
			continue;

		if ((other->health <= 0) || (other->owner != other))
			continue;

		if (other->flags & (ENT_BOSS|ENT_STATIC|ENT_GALDOV|ENT_NOMOVE|ENT_INANIMATE))
			continue;

		if ((fabs(other->x - cx) > rangeX) || (fabs(other->y - cy) > rangeY))
			continue;

		EnemyAIState &st = getAIState(other);

		st.lastX = (int)player.x;
		st.lastY = (int)player.y;

		if (st.awareness < awareLevel)
			st.awareness = awareLevel;

		st.lostSight = 0;

		other->tx = (int)player.x;
		other->ty = (int)player.y;
	}
}

// When an enemy spots the player (or gets shot), nearby enemies hear about it too
static void alertNearbyEnemies(Entity *source)
{
	// a sergeant shouts further
	float scale = (getAIState(source).rank == RANK_SERGEANT) ? 1.5f : 1.0f;

	alertEnemiesNear((int)source->x, (int)source->y, (int)(ALERT_RANGE_X * scale), (int)(ALERT_RANGE_Y * scale), source, ALLY_ALERT_LEVEL);
}

// A sergeant that spots the player (or is shot) puts its whole squad on full alert
static void alertSquad(Entity *leader)
{
	Entity *other = (Entity*)map.enemyList.getHead();

	while (other->next != NULL)
	{
		other = (Entity*)other->next;

		if ((other == leader) || (other->health <= 0))
			continue;

		std::map<Entity*, EnemyAIState>::iterator it = aiState.find(other);

		if ((it == aiState.end()) || (it->second.squadLeader != leader) || (it->second.confused > 0))
			continue;

		EnemyAIState &st = it->second;

		st.awareness = AWARE_MAX;
		st.alerted = true;
		st.lostSight = 0;
		st.lastX = (int)player.x;
		st.lastY = (int)player.y;

		other->tx = (int)player.x;
		other->ty = (int)player.y;
	}
}

// Attack turns: only a few enemies may be attacking at the same time, and
// they can't all start on the same frame. Harder difficulty = more at once.
static int getMaxAttackers()
{
	return 1 + game.skill;
}

static int countActiveAttackers()
{
	int count = 0;

	for (std::map<Entity*, EnemyAIState>::iterator it = aiState.begin() ; it != aiState.end() ; ++it)
	{
		if ((it->second.attackStamp != 0) && ((aiFrame - it->second.attackStamp) <= 1))
			count++;
	}

	return count;
}

static bool canStartAttack()
{
	return (attackCooldown <= 0) && (countActiveAttackers() < getMaxAttackers());
}

static void beginAttack(EnemyAIState &st)
{
	st.attackStamp = aiFrame;
	attackCooldown = 20 - (game.skill * 4);
}

// Fast, weak weapons fire a short burst. Slow, explosive or hard-hitting ones
// (grenades, rockets, alien lasers...) stay single shot.
static int getBurstSize(Entity *enemy)
{
	Weapon *w = enemy->currentWeapon;

	if ((w->reload > 25) || (w->flags & ENT_EXPLODES))
		return 1;

	int size = 2;

	if ((game.skill >= 2) && ((Math::prand() % 2) == 0))
		size = 3;

	if (getAIState(enemy).rank == RANK_VETERAN)
		size++;

	// never let a whole burst hit for more than about 4 damage
	int damage = (w->damage < 1) ? 1 : w->damage;
	int maxSize = 4 / damage;

	if (maxSize < 1)
		maxSize = 1;

	// the spread gun already fires 3 shots at once
	if ((w == &weapon[WP_ALIENSPREAD]) && (maxSize > 2))
		maxSize = 2;

	if (size > maxSize)
		size = maxSize;

	return size;
}

// Called once per frame for every free enemy: notices the player gradually,
// remembers where they were last seen and heads there.
static void senseSurroundings(Entity *enemy, EnemyAIState &ai)
{
	ai.canSee = false;

	bool playerActive = (player.health > -60) && (game.missionOverReason != MIS_COMPLETE);
	float dist = fabs(enemy->x - player.x);
	bool inRange = playerActive && isEnemyOnScreen(enemy) && (dist <= VISION_RANGE) && (fabs(enemy->y - player.y) <= 100);

	if (inRange)
		ai.canSee = hasClearShot(enemy);

	float before = ai.awareness;
	bool sensed = false;

	if (enemy->flags & ENT_ALWAYSCHASE)
	{
		// these always know where the player is
		if (inRange)
		{
			ai.awareness = AWARE_MAX;
			sensed = true;
		}
	}
	else if (ai.canSee)
	{
		bool behind = ((enemy->face == 0) && (player.x < enemy->x)) || ((enemy->face == 1) && (player.x > enemy->x));

		// an unaware enemy doesn't notice you behind its back, unless you're very close
		if ((!behind) || (dist < BEHIND_RANGE) || (ai.awareness >= AWARE_SUSPICIOUS))
		{
			float closeness = 1.0f - (dist / VISION_RANGE);

			if (closeness < 0)
				closeness = 0;

			// close = fast, far = slow. Harder difficulty = quicker to notice.
			float gain = (0.6f + (2.4f * closeness)) * (0.7f + (0.2f * game.skill));

			if (behind)
				gain *= 0.5f;

			ai.awareness += gain;

			if (ai.awareness > AWARE_MAX)
				ai.awareness = AWARE_MAX;

			sensed = true;
		}
	}

	if (sensed)
	{
		ai.lostSight = 0;
		ai.lastX = (int)player.x;
		ai.lastY = (int)player.y;
	}
	else
	{
		if (ai.lostSight < 100000)
			ai.lostSight++;

		// keep looking for a while before calming down, then calm down slowly
		int hold = ai.alerted ? ALERT_HOLD : SUSPICIOUS_HOLD;

		if ((ai.lostSight > hold) && (ai.awareness > 0))
		{
			ai.awareness -= AWARE_DECAY;

			if (ai.awareness <= 0)
			{
				ai.awareness = 0;
				ai.alerted = false;
			}
		}
	}

	// just became fully alert: warn the others
	if ((before < AWARE_ALERT) && (ai.awareness >= AWARE_ALERT))
	{
		ai.alerted = true;
		alertNearbyEnemies(enemy);

		if (ai.rank == RANK_SERGEANT)
			alertSquad(enemy);
	}

	// suspicious or alert: go to where the player was last seen
	if (ai.awareness >= AWARE_SUSPICIOUS)
	{
		enemy->tx = ai.lastX;
		enemy->ty = ai.lastY;
	}
}

void lookForPlayer(Entity *enemy)
{
	// player is dead
	if (player.health <= -60)
		return;

	if (game.missionOverReason == MIS_COMPLETE)
		return;

	int x = (int)fabs(enemy->x - player.x);
	int y = (int)fabs(enemy->y - player.y);

	// out of range (or off screen)
	if ((x > 720) || (!isEnemyOnScreen(enemy)))
		return;

	// can't even jump that high!
	if (y > 100)
		return;

	EnemyAIState &st = getAIState(enemy);
	bool isLeader = (enemy->owner == enemy);

	// winding up a shot: stand still, doAI() tells us when to fire
	if (st.telegraph > 0)
		return;

	// the sergeant died: too confused to fight
	if (st.confused > 0)
		return;

	// leaders already checked their line of sight (and awareness) in senseSurroundings()
	bool canSee = isLeader ? st.canSee : hasClearShot(enemy);

	// followers have no awareness of their own: they always act alert
	bool alert = (!isLeader) || (st.awareness >= AWARE_ALERT);

	// lost sight of the player: give up the burst
	if (!canSee)
		st.burstLeft = 0;

	if (!isLeader)
	{
		// Player is in range... go for them!
		bool spotted = (enemy->flags & ENT_ALWAYSCHASE) || ((Math::prand() % (35 - game.skill)) == 0);

		if (spotted)
		{
			enemy->owner->tx = (int)(player.x);
			enemy->owner->ty = (int)(player.y);
		}
	}

	// an alert enemy always turns to face the player it is fighting
	if (isLeader && alert && canSee)
		enemy->face = (player.x < enemy->x) ? 1 : 0;

	// facing the wrong way
	if ((enemy->face == 0) && (player.x < enemy->x))
	{
		st.burstLeft = 0;
		return;
	}

	// still facing the wrong way
	if ((enemy->face == 1) && (player.x > enemy->x))
	{
		st.burstLeft = 0;
		return;
	}

	// can't fire while reloading, but keep chasing/turning. Only alert enemies attack.
	if ((enemy->reload <= 0) && (alert || (enemy->flags & ENT_ALWAYSFIRES)))
	{
		float aimDY = getAimDY(enemy);

		if (canSee)
		{
			bool shoot = false;
			bool fromWindup = st.fireNow;
			bool continuing = (st.burstLeft > 0);

			if (enemy->flags & ENT_ALWAYSFIRES)
			{
				shoot = true;
			}
			else if (fromWindup)
			{
				shoot = true;
			}
			else if (continuing)
			{
				shoot = true;
			}
			else if (isLeader ? ((Math::prand() % 100) < (4 + (game.skill * 3))) : ((Math::prand() % 850) <= (game.skill * 5)))
			{
				// wait for our turn to attack
				if (canStartAttack())
				{
					float power = 0;
					bool grenade = isGrenadeWeapon(enemy);

					// a grenade thrower only starts when it can hit the player and its own blast is safe
					if ((!grenade) || (isGrenadeSafe(enemy) && planEnemyGrenade(enemy, &power)))
					{
						beginAttack(st);

						if (isLeader)
						{
							// warn the player before shooting
							st.telegraph = getWindupFrames(st.rank);
							st.charging = false;

							// a grenade is held for as long as the throw needs (far or high targets)
							if (grenade)
							{
								int hold = (int)(power * GRENADE_CHARGE_FRAMES);

								if (hold > 0)
								{
									st.telegraph += hold;
									st.charging = true;
								}
							}

							st.telegraphTotal = st.telegraph;
						}
						else
						{
							shoot = true;
						}
					}
				}
			}

			if (shoot)
			{
				st.fireNow = false;

				// Don't fire grenades if it would hurt self or allies
				bool grenadeThrown = false;
				if (isGrenadeWeapon(enemy))
				{
					if (!isGrenadeSafe(enemy))
					{
						// Skip this shot, but keep the burst timing
						if (fromWindup || continuing)
						{
							if (fromWindup)
							{
								st.burstLeft = getBurstSize(enemy) - 1;
								st.burstTimeout = 120;
							}
							else
							{
								st.burstLeft--;
							}
						}
						return;
					}

					// The player may have moved out of reach while the grenade was being held
					float power = 0;
					if (!planEnemyGrenade(enemy, &power))
					{
						st.burstLeft = 0;
						enemy->reload = 30;
						return;
					}

					// Record grenade position and start cooldown
					st.grenadeX = (int)player.x;
					st.grenadeY = (int)player.y;
					st.grenadeTimer = GRENADE_COOLDOWN;

					// Throw it with the speed and angle that land on the player
					addEnemyGrenade(enemy, power);
					grenadeThrown = true;
				}

				if (!grenadeThrown)
				{
					addBullet(enemy, enemy->currentWeapon->getSpeed(enemy->face), aimDY);
					if (enemy->currentWeapon == &weapon[WP_ALIENSPREAD])
					{
						addBullet(enemy, enemy->currentWeapon->getSpeed(enemy->face), aimDY + 2);
						addBullet(enemy, enemy->currentWeapon->getSpeed(enemy->face), aimDY - 2);
					}
				}

				if (enemy->flags & ENT_ALWAYSFIRES)
				{
					st.burstLeft = 0;
				}
				else if (fromWindup || continuing)
				{
					bool inBurst = continuing;

					if (fromWindup)
					{
						st.burstLeft = getBurstSize(enemy) - 1;
						st.burstTimeout = 120;
						inBurst = (st.burstLeft > 0);
					}
					else
					{
						st.burstLeft--;
					}

					// space the shots out, and rest once the burst is over
					int gap = enemy->currentWeapon->reload;

					if (gap < 6)
						gap = 6;

					if (enemy->reload < gap)
						enemy->reload = gap;

					if (inBurst && (st.burstLeft <= 0))
						enemy->reload += BURST_PAUSE;
				}
			}

			if ((enemy->flags & ENT_RAPIDFIRE) && (st.telegraph == 0))
			{
				if (enemy->flags & ENT_ALWAYSFIRES)
				{
					if ((Math::prand() % 25) > game.skill * 3)
						Math::removeBit(&enemy->flags, ENT_ALWAYSFIRES);
				}
				else
				{
					if ((Math::prand() % 50) < game.skill * 2)
						Math::addBit(&enemy->flags, ENT_ALWAYSFIRES);
				}
			}
		}
		else
		{
			if (enemy->flags & ENT_RAPIDFIRE)
				Math::removeBit(&enemy->flags, ENT_ALWAYSFIRES);
		}
	}

	// just started the warning (or in the middle of a burst): don't jump away
	if ((st.telegraph > 0) || (st.burstLeft > 0))
		return;

	if ((enemy->flags & ENT_FLIES) || (enemy->flags & ENT_SWIMS) || (enemy->flags & ENT_NOJUMP))
		return;
		
	if (enemy->flags & ENT_JUMPS)
	{
		if (!enemy->falling)
		{
			if ((Math::prand() % 25) == 0)
			{
				int distance = Math::rrand(1, 4);
				enemy->setVelocity(distance - ((distance * 2) * enemy->face), Math::rrand(-12, -10));
			}
		}
		
		return;
	}

	// calm enemies don't try to get to the player
	if (isLeader && (st.awareness < AWARE_SUSPICIOUS))
		return;

	// Jump to try and reach player (even if they are approximately level with you!)
	if (player.y - 5 < enemy->y)
	{
		if (!enemy->falling)
		{
			if ((Math::prand() % 100) == 0)
			{
				enemy->dy = -12;
			}
		}
	}
}

void doAI(Entity *enemy)
{
	EnemyAIState &ai = getAIState(enemy);

	if (ai.telegraph > 0)
	{
		if (--ai.telegraph == 0)
		{
			ai.fireNow = true;
			ai.fireTimeout = 45;
			ai.charging = false;
		}
	}
	else if (ai.fireNow)
	{
		// the shot wasn't possible yet (still reloading...): keep trying for a moment
		if (--ai.fireTimeout <= 0)
			ai.fireNow = false;
	}

	if (ai.burstLeft > 0)
	{
		if (--ai.burstTimeout <= 0)
			ai.burstLeft = 0;
	}

	// Update grenade timer
	if (ai.grenadeTimer > 0)
		ai.grenadeTimer--;

	// tell the attack turns system this enemy is busy
	if ((ai.telegraph > 0) || (ai.burstLeft > 0))
		ai.attackStamp = aiFrame;

	if (enemy->flags & ENT_GALDOV)
	{
		doGaldovAI(enemy);
	}
	else if (enemy->flags & ENT_BOSS)
	{
		return;
	}

	int x = (int)enemy->x;
	int y = (int)enemy->y + enemy->height;
	
	if (enemy->dx > 0)
		x += enemy->width;

	x = x >> BRICKSHIFT;
	y = y >> BRICKSHIFT;

	// notice the player (gradually) and head for where they were last seen
	senseSurroundings(enemy, ai);

	bool aware = (ai.awareness >= AWARE_SUSPICIOUS);

	// a squad soldier whose sergeant just died is lost for a few seconds
	if ((ai.squadLeader != NULL) && (ai.squadLeader->health <= 0))
	{
		ai.squadLeader = NULL;
		ai.confused = SQUAD_PANIC_FRAMES;
		ai.panicDir = (player.x < enemy->x) ? 1 : -1;
		ai.telegraph = 0;
		ai.fireNow = false;
		ai.burstLeft = 0;
	}

	if (ai.confused > 0)
	{
		ai.confused--;

		if (((ai.confused % 45) == 0) && ((Math::prand() % 2) == 0))
			ai.panicDir = -ai.panicDir;

		enemy->tx = (int)enemy->x + (ai.panicDir * 100);
		enemy->ty = (int)enemy->y;
		aware = true; // keep the destination instead of forgetting it
	}

	// An aware enemy that can't get any closer (wall in the way) tries to jump
	// it, and gives up after a while
	int curX = (int)enemy->x;

	if (aware && (enemy->tx != curX) && (!enemy->falling) && (ai.telegraph == 0) && (ai.burstLeft == 0) && (curX == ai.prevX))
		ai.stuck++;
	else
		ai.stuck = 0;

	ai.prevX = curX;

	if (ai.stuck == 20)
	{
		if ((!(enemy->flags & (ENT_FLIES|ENT_SWIMS|ENT_NOJUMP))) && map.isSolid(x, y))
			enemy->dy = -12;
	}
	else if (ai.stuck > 60)
	{
		enemy->tx = curX;
	}

	// Calm enemies that stopped moving forget their destination. Aware ones
	// keep it: otherwise a standing enemy could never start chasing.
	if ((enemy->dx == 0) && (!aware))
		enemy->tx = (int)enemy->x;

	// AVOID PLAYER'S GRENADES (highest priority - survival)
	if (avoidPlayerGrenade(enemy))
	{
		// Grenade detected and moved away, skip distance preference
		// Continue to ally grenade avoidance and separation
	}
	else
	{
		// Distance preference: aware enemies stop at their preferred distance
		if (aware && (ai.canSee))
		{
			int prefDist = getPreferredDistance(enemy);
			float currentDist = fabs(enemy->x - player.x);

			// If too close, back away. If too far, approach.
			if (currentDist < (prefDist - 50))
			{
				// Too close: move away from player
				if (player.x < enemy->x)
					enemy->tx = (int)(enemy->x + 100);
				else
					enemy->tx = (int)(enemy->x - 100);
			}
			else if (currentDist > (prefDist + 50))
			{
				// Too far: move toward player (already handled by ai.lastX/lastY)
				enemy->tx = ai.lastX;
			}
			else
			{
				// At preferred distance: hold position
				enemy->tx = (int)enemy->x;
			}
		}
	}

	// Grenade avoidance: if enemy threw a grenade recently, stay away from it
	if (ai.grenadeTimer > 0)
	{
		float distToGrenade = fabs(enemy->x - ai.grenadeX);

		// If too close to grenade position, move away
		if (distToGrenade < GRENADE_SAFE_DISTANCE)
		{
			// Move away from grenade position
			if (ai.grenadeX < enemy->x)
				enemy->tx = (int)(enemy->x + 100);
			else
				enemy->tx = (int)(enemy->x - 100);
		}
	}

	// Also avoid grenades thrown by nearby allies
	Entity *other = (Entity*)map.enemyList.getHead();
	while (other->next != NULL)
	{
		other = (Entity*)other->next;

		if (other == enemy)
			continue;

		if (other->health <= 0)
			continue;

		EnemyAIState &otherSt = getAIState(other);

		// If this ally has an active grenade, check if we're too close
		if (otherSt.grenadeTimer > 0)
		{
			float distToAllyGrenade = fabs(enemy->x - otherSt.grenadeX);

			if (distToAllyGrenade < GRENADE_SAFE_DISTANCE)
			{
				// Move away from ally's grenade position
				if (otherSt.grenadeX < enemy->x)
					enemy->tx = (int)(enemy->x + 100);
				else
					enemy->tx = (int)(enemy->x - 100);
			}
		}
	}

	// Separation: avoid stacking with other enemies
	if (aware)
	{
		Entity *other = (Entity*)map.enemyList.getHead();
		while (other->next != NULL)
		{
			other = (Entity*)other->next;
			if (isEnemyTooClose(enemy, other))
			{
				// Move away from the other enemy
				if (other->x < enemy->x)
					enemy->tx = (int)(enemy->x + 50);
				else
					enemy->tx = (int)(enemy->x - 50);
			}
		}
	}

	// Don't enter areas you're not supposed to
	if (enemy->tx != (int)enemy->x)
	{
		if (!(enemy->flags & (ENT_FLIES|ENT_SWIMS)))
		{
			if (!map.isSolid(x, y))
			{
				enemy->tx = (int)enemy->x;
			}
		}
	}

	if ((int)enemy->x == enemy->tx)
	{
		// while searching for the player, look around instead of wandering off
		if (aware && (!ai.canSee) && (ai.lostSight > 0) && ((ai.lostSight % 60) == 0))
		{
			enemy->face = 1 - enemy->face;
		}

		if ((!aware) && ((Math::prand() % 100) == 0))
		{
			if ((ai.squadLeader != NULL) && (ai.squadLeader->health > 0))
				enemy->tx = (int)(ai.squadLeader->x + Math::rrand(-96, 96)); // stay with the squad
			else
				enemy->tx = (int)(enemy->x + Math::rrand(-640, 640));
			enemy->ty = (int)(enemy->y);
			if ((enemy->flags & ENT_FLIES) || (enemy->flags & ENT_SWIMS))
			{
				enemy->ty = (int)(enemy->y + Math::rrand(-320, 320));
			}
		}

		Math::limitInt(&enemy->tx, 15, (MAPWIDTH * BRICKSIZE)- 20);
		Math::limitInt(&enemy->ty, 15, (MAPHEIGHT * BRICKSIZE)- 20);

		if (map.isSolid((enemy->tx >> BRICKSHIFT), (enemy->ty >> BRICKSHIFT)))
		{
			enemy->tx = (int)enemy->x;
			enemy->ty = (int)enemy->y;
		}
	}

	// Don't enter areas you're not supposed to
	if (enemy->ty != (int)enemy->y)
	{
		if (enemy->flags & ENT_FLIES)
		{
			if (map.isLiquid(x, y + 1))
			{
				enemy->ty = (int)enemy->y;
			}
		}
	}

	if ((int)enemy->y == enemy->ty)
	{
		enemy->y = enemy->ty;
		enemy->dx = 0;
	}

	if (!enemy->falling)
		enemy->dx = 0;

	if ((enemy->flags & ENT_FLIES) || (enemy->flags & ENT_SWIMS))
	{
		enemy->dx = enemy->dy = 0;

		if ((int)enemy->y < enemy->ty) enemy->dy = 1;
		if ((int)enemy->y > enemy->ty) enemy->dy = -1;
	}

	if ((int)enemy->x == enemy->tx) {enemy->dx = 0;}
	if ((int)enemy->x < enemy->tx) {enemy->dx = 1; enemy->face = 0;}
	if ((int)enemy->x > enemy->tx) {enemy->dx = -1; enemy->face = 1;}

	if ((enemy->flags & ENT_SWIMS) && (enemy->environment == ENV_WATER))
	{
		enemy->dy = 0;

		if ((int)enemy->y < enemy->ty) enemy->dy = 1;
		if ((int)enemy->y > enemy->ty) enemy->dy = -1;
	}

	// winding up a shot or firing a burst: hold still
	if ((ai.telegraph > 0) || (ai.burstLeft > 0))
	{
		enemy->dx = 0;

		if ((enemy->flags & ENT_FLIES) || (enemy->flags & ENT_SWIMS))
			enemy->dy = 0;
	}

	lookForPlayer(enemy);
}

void checkCombo()
{
	int old = game.currentComboHits;
	
	game.doCombo();
	
	if (old == 24 && game.currentComboHits == 25)
	{
		presentPlayerMedal("25_Hit_Combo");
	}
}

void enemyBulletCollisions(Entity *bullet)
{
	if (bullet->health < 1)
	{
		return;
	}

	Entity *enemy = (Entity*)map.enemyList.getHead();

	while (enemy->next != NULL)
	{
		enemy = (Entity*)enemy->next;

		if ((enemy->flags & ENT_TELEPORTING) || (enemy->dead == DEAD_DYING))
		{
			continue;
		}

		char comboString[100];

		if ((bullet->owner == &player) || (bullet->owner == &engine.world) || (bullet->flags & ENT_BOSS))
		{
			if (Collision::collision(enemy, bullet))
			{
				if (bullet->id != WP_LASER)
				{
					bullet->health = 0;
				}
				
				Math::removeBit(&bullet->flags, ENT_SPARKS);
				Math::removeBit(&bullet->flags, ENT_PUFFS);

				if ((enemy->flags & ENT_IMMUNE) && (!(enemy->flags & ENT_STATIC)))
				{
					bullet->health = 0; // include the Laser for this one!
					enemy->owner->tx = (int)bullet->owner->x;
					enemy->owner->ty = (int)bullet->owner->y;
					if (enemy->x < enemy->tx) {enemy->owner->dx = 1; enemy->owner->face = 0;}
					if (enemy->x > enemy->tx) {enemy->owner->dx = -1; enemy->owner->face = 1;}
					return;
				}

				/*
					Increment the bullet hits counter. The laser can only do this
				 	if the target has more than 0 health. Overwise the stats screen
				 	can show an accurracy of 800%. Which is just plain silly.
				*/
				if (bullet->owner == &player)
				{
					enemy->tx = (int)player.x;
					enemy->ty = (int)player.y;

					// the enemy that was hit, and its neighbours, now know where the player is
					if (!(enemy->flags & ENT_STATIC))
					{
						EnemyAIState &hitState = getAIState(enemy);

						hitState.lastX = (int)player.x;
						hitState.lastY = (int)player.y;
						hitState.awareness = AWARE_MAX;
						hitState.alerted = true;
						hitState.lostSight = 0;

						alertNearbyEnemies(enemy);

						if (hitState.rank == RANK_SERGEANT)
							alertSquad(enemy);
					}
					enemy->face = 0;
					
					if (player.x < enemy->x)
					{
						enemy->face = 1;
					}

					if ((bullet->id != WP_LASER) || (enemy->health > 0))
					{
						game.incBulletsHit();
					}
				}

				if (!(enemy->flags & ENT_EXPLODES))
				{
					audio.playSound(SND_HIT, CH_ANY, enemy->x);
					if (game.gore)
					{
						addBlood(enemy, bullet->dx / 4, Math::rrand(-6, -3), 1);
					}
					else
					{
						addColorParticles(bullet->x, bullet->y, Math::rrand(25, 75), -1);
					}
				}
				else
				{
					audio.playSound(SND_CLANG, CH_ANY, enemy->x);
					addColorParticles(bullet->x, bullet->y, Math::rrand(25, 75), -1);
				}

				if (enemy->health > 0)
				{
					if (!(enemy->flags & ENT_IMMUNE))
					{
						enemy->health -= bullet->damage;
					}
					
					if (enemy->health <= 0)
					{	
						if (bullet->owner == &player)
						{
							addPlayerScore(enemy->value);
							game.currentMissionEnemiesDefeated++;
	
							if (player.currentWeapon != &weapon[WP_LASER])
							{
								checkCombo();
							}
							
							snprintf(comboString, sizeof comboString, "Combo-%s", bullet->name);
							checkObjectives(comboString, false);
							checkObjectives("Enemy", false);
							checkObjectives(enemy->name, false);
						}
	
						if (!(enemy->flags & ENT_STATIC))
						{
							enemy->dx = (bullet->dx / 4);
							enemy->dy = -10;
							
							if (enemy->flags & ENT_EXPLODES)
							{
								audio.playSound(SND_ELECDEATH1 + Math::prand() % 3, CH_DEATH, enemy->x);
							}
							else if (game.gore)
							{
								audio.playSound(SND_DEATH1 + Math::prand() % 3, CH_DEATH, enemy->x);
							}
						}
					}
					
				}
				
				if (enemy->flags & ENT_STATIC)
				{
					return;
				}

				if (enemy->health < 0)
				{
					enemy->dx = Math::rrand(-3, 3);
					enemy->dy = 5 - Math::prand() % 15;
					enemy->health = -1;
					
					if (enemy->flags & ENT_EXPLODES)
					{
						audio.playSound(SND_ELECDEATH1 + Math::prand() % 3, CH_DEATH, enemy->x);
					}
					else if (game.gore)
					{
						audio.playSound(SND_DEATH1 + Math::prand() % 3, CH_DEATH, enemy->x);
					}

					if (bullet->owner == &player)
					{
						if (player.currentWeapon != &weapon[WP_LASER])
						{
							snprintf(comboString, sizeof comboString, "Combo-%s", bullet->name);
							checkCombo();
							checkObjectives(comboString, false);
						}
					}
				}

				if (game.currentComboHits >= 3)
				{
					char message[50];
					snprintf(message, sizeof message, _("%d Hit Combo!"), game.currentComboHits);
					engine.setInfoMessage(message, 0, INFO_NORMAL);
				}

				return;
			}
		}
	}
}

int getNonGoreParticleColor(const char *name)
{
	int rtn = graphics.yellow;
	
	if (strcmp(name, "Pistol Blob") == 0)
	{
		rtn = graphics.green;
	}
	else if (strcmp(name, "Grenade Blob") == 0)
	{
		rtn = graphics.skyBlue;
	}
	else if (strcmp(name, "Aqua Blob") == 0)
	{
		rtn = graphics.cyan;
	}
	else if (strcmp(name, "Laser Blob") == 0)
	{
		rtn = SDL_MapRGB(graphics.screen->format, 255, 0, 255);
	}
	else if (strcmp(name, "Machine Gun Blob") == 0)
	{
		rtn = SDL_MapRGB(graphics.screen->format, 200, 64, 24);
	}
	
	return rtn;
}

void gibEnemy(Entity *enemy)
{
	if (enemy->flags & ENT_GALDOV)
	{
		addTeleportParticles(enemy->x, enemy->y, 75, SND_TELEPORT3);
		checkObjectives("Galdov", true);
	}

	if (enemy->flags & ENT_EXPLODES)
	{
		addExplosion(enemy->x + (enemy->width / 2), enemy->y + (enemy->height / 2), 10 + (20 * game.skill), enemy);
		addSmokeAndFire(enemy, Math::rrand(-5, 5), Math::rrand(-5, 5), 2);
		return;
	}

	float x, y, dx, dy;
	int amount = (game.gore) ? 25 : 150;
	int color = getNonGoreParticleColor(enemy->name);

	for (int i = 0 ; i < amount ; i++)
	{
		x = enemy->x + Math::rrand(-3, 3);
		y = enemy->y + Math::rrand(-3, 3);
		
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
			addColoredEffect(x, y, dx, dy, color, EFF_COLORED + EFF_WEIGHTLESS);
		}
	}
	
	(game.gore) ? audio.playSound(SND_SPLAT, CH_ANY) : audio.playSound(SND_POP, CH_ANY, enemy->x);
}

void doEnemies()
{
	Entity *enemy = (Entity*)map.enemyList.getHead();
	Entity *previous = enemy;
	
	map.fightingGaldov = false;

	aiFrame++;

	if (attackCooldown > 0)
		attackCooldown--;

	// Hearing: a shot from the player is heard by enemies nearby, even through walls
	if ((int)player.reload > lastPlayerReload)
	{
		if ((player.health > -60) && (game.missionOverReason != MIS_COMPLETE))
		{
			alertEnemiesNear((int)player.x, (int)player.y, HEAR_RANGE_X + (game.skill * 40), HEAR_RANGE_Y, NULL, HEAR_LEVEL);
		}
	}

	lastPlayerReload = (int)player.reload;

	int x, y, absX, absY;

	while (enemy->next != NULL)
	{
		enemy = (Entity*)enemy->next;
		
		if (!engine.cheatBlood)
		{
			if (enemy->dead == DEAD_DYING)
			{
				if (!enemy->referenced)
				{
					debug(("Removing unreferenced enemy '%s'\n", enemy->name));
					forgetEnemy(enemy);
					map.enemyList.remove(previous, enemy);
					enemy = previous;
				}
				else
				{
					previous = enemy;
				}
				
				enemy->referenced = false;
				continue;
			}
		}

		x = (int)(enemy->x - engine.playerPosX);
		y = (int)(enemy->y - engine.playerPosY);

		absX = abs(x);
		absY = abs(y);

		if ((absX < ACTIVE_W) && (absY < ACTIVE_H))
		{
			// Fly forever
			if (enemy->flags & ENT_FLIES)
			{
				enemy->fuel = 7;
			}

			if (enemy->owner->flags & ENT_TELEPORTING)
			{
				moveEntity(enemy);
			}
			else
			{
				if ((enemy->health > 0) && (!(enemy->flags & ENT_STATIC)))
				{
					enemy->think();

					if (enemy->owner == enemy)
					{
						doAI(enemy);
					}
					else
					{
						lookForPlayer(enemy);
					}
				}
				
				if (map.isBlizzardLevel)
				{
					enemy->dx += map.windPower * 0.1;
				}

				if (enemy->flags & ENT_NOMOVE)
				{
					enemy->dx = 0;
				}

				moveEntity(enemy);

				if ((absX < DRAW_W) && (absY < DRAW_H))
				{
					if (enemy->flags & ENT_FIRETRAIL)
					{
						addFireTrailParticle(enemy->x + (enemy->face * 16) + Math::rrand(-1, 1), enemy->y + Math::rrand(-1, 1));
					}
					
					graphics.blit(enemy->getFaceImage(), x, y, graphics.screen, false);
					
					drawAwareness(enemy, x, y);
					drawHealthBar(enemy, x, y);
					drawRankBadge(enemy, x, y);

					if ((enemy->dx != 0) || (enemy->flags & ENT_FLIES) || (enemy->flags & ENT_STATIC))
					{
						enemy->animate();
					}
				}
			}
		}
		else
		{
			if (enemy->flags & ENT_SPAWNED)
			{
				if ((absX > 1920) || (absY > 1440))
				{
					enemy->health = -100;
				}
			}
		}

		if (enemy->health > 0)
		{
			previous = enemy;
			
			if ((enemy->environment == ENV_SLIME) || (enemy->environment == ENV_LAVA))
			{
				checkObjectives(enemy->name, false);
				enemy->health = -1;
			}
		}
		else
		{
			if (enemy->flags & ENT_GALDOV)
			{
				enemy->health = -99;
			}

			if (enemy->health == 0)
			{
				Math::removeBit(&enemy->flags, ENT_WEIGHTLESS);
				Math::removeBit(&enemy->flags, ENT_SWIMS);
				Math::removeBit(&enemy->flags, ENT_FLIES);
				Math::addBit(&enemy->flags, ENT_INANIMATE);
				Math::addBit(&enemy->flags, ENT_BOUNCES);
				enemy->health = -1 - Math::prand() % 25;
			}
			
			if (engine.cheatBlood)
			{
				if (!(enemy->flags & ENT_EXPLODES))
				{
					if ((enemy->health % 4) == 0)
					{
						addBlood(enemy, Math::rrand(-2, 2), Math::rrand(-6, -3), 1);
					}
					else if ((enemy->health % 10) == 0)
					{
						if (game.gore)
						{
							audio.playSound(SND_DEATH1 + Math::prand() % 3, CH_DEATH, enemy->x);
						}
					}
				}
			}

			enemy->health--;

			if (enemy->flags & ENT_MULTIEXPLODE)
			{
				if (enemy->health < -30)
				{
					if ((enemy->health % 3) == 0)
					{
						addExplosion(enemy->x + Math::prand() % 25, enemy->y + Math::prand() % 25,  10 + (20 * game.skill), enemy);
						addSmokeAndFire(enemy, Math::rrand(-5, 5), Math::rrand(-5, 5), 2);
					}
				}
			}

			if (enemy->health > -50)
			{
				previous = enemy;
			}
			else
			{
				if (enemy->flags & ENT_GALDOVFINAL)
				{
					enemy->health = -30;
					enemy->dx = Math::rrand(-10, 10);
					enemy->dy = Math::rrand(-10, 10);
				}
				else
				{
					if (enemy->dead == DEAD_ALIVE)
					{
						if ((absX < ACTIVE_W) && (absY < ACTIVE_H))
						{
							gibEnemy(enemy);
							
							if (enemy->value)
							{
								dropRandomItems((int)enemy->x, (int)enemy->y);
							}
						}
						
						enemy->dead = DEAD_DYING;
					}
					
					if (enemy->dead == DEAD_DYING)
					{
						if (!enemy->referenced)
						{
							if ((absX < ACTIVE_W) && (absY < ACTIVE_H))
							{
								gibEnemy(enemy);
								
								if (enemy->value)
								{
									dropRandomItems((int)enemy->x, (int)enemy->y);
								}
							}
							
							debug(("Removing unreferenced enemy '%s'\n", enemy->name));
							forgetEnemy(enemy);
							map.enemyList.remove(previous, enemy);
							enemy = previous;
						}
					}
				}
			}
		}
		
		// default the enemy to not referenced.
		// doBullets() will change this if required.
		enemy->referenced = false;
	}
}

void loadEnemy(const char *token)
{
	int enemy = -1;

	for (int i = MAX_ENEMIES - 1; i >= 0; i--)
		if (strcmp(defEnemy[i].name, "") == 0)
			enemy = i;

	if (enemy == -1)
	{
		printf("Out of enemy define space!\n");
		exit(1);
	}

	char name[50], sprite[3][100], weapon[100], flags[1024];
	int health, value;

	sscanf(token, "%*c %[^\"] %*c %s %s %s %*c %[^\"] %*c %d %d %s", name, sprite[0], sprite[1], sprite[2], weapon, &health, &value, flags);

	defEnemy[enemy].setName(name);
	defEnemy[enemy].setSprites(graphics.getSprite(sprite[0], true), graphics.getSprite(sprite[1], true), graphics.getSprite(sprite[2], true));
	defEnemy[enemy].currentWeapon = getWeaponByName(weapon);
	defEnemy[enemy].health = health;
	defEnemy[enemy].value = value;

	defEnemy[enemy].flags = engine.getValueOfFlagTokens(flags);
}

void loadDefEnemies()
{
	for (int i = 0 ; i < MAX_ENEMIES ; i++)
	{
		defEnemy[i].name[0] = 0;
	}

	int enemy = 0;

	if (!engine.loadData("data/defEnemies"))
	{
		return graphics.showErrorAndExit("Couldn't load enemy definitions file (%s)", "data/defEnemies");
	}

	char *token = strtok((char*)engine.dataBuffer, "\n");

	char name[50], sprite[3][100], weapon[100], flags[1024];
	int health, value;

	while (true)
	{
		if (strcmp(token, "@EOF@") == 0)
		{
			break;
		}

		sscanf(token, "%*c %[^\"] %*c %s %s %s %*c %[^\"] %*c %d %d %s", name, sprite[0], sprite[1], sprite[2], weapon, &health, &value, flags);

		defEnemy[enemy].setName(name);
		defEnemy[enemy].setSprites(graphics.getSprite(sprite[0], true), graphics.getSprite(sprite[1], true), graphics.getSprite(sprite[2], true));
		defEnemy[enemy].currentWeapon = getWeaponByName(weapon);
		defEnemy[enemy].health = health;
		defEnemy[enemy].value = value;
		defEnemy[enemy].flags = engine.getValueOfFlagTokens(flags);

		enemy++;

		token = strtok(NULL, "\n");
	}
}
