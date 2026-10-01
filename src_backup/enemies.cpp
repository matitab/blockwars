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
extern void addEnemyAimedShot(Entity *owner, float angleDegrees);
// Eye Droid beam (bullets.cpp): one damage tick, delivered through the normal bullet collisions
extern void addDroidBeamHit(Entity *owner, float x, float y, int damage);

// Item drops (items.cpp): the rank (0 soldier, 1 veteran, 2 sergeant) makes the special weapons drop more often
extern void dropRandomItemsByRank(int x, int y, int rank);

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
	bool utilityDroid; // this enemy is the utility droid (shield, dashes, safe mode)
	int shield;      // electromagnetic shield points left (utility droid only)
	int maxShield;   // shield points it started with (for the glow and the recharge)
	int dashes;      // teleport dashes left in the pool
	int rechargeTimer; // frames left of the safe mode recharge
	int stagger;     // frames left of the stun caused by hits while the shield is down
	bool safeMode;   // shield broken or dash pool empty: weapons off, crawling, dodging with the dashes it has left, hiding until the recharge ends
	int dashCooldown; // frames until the next dash is allowed
	bool dashReact;  // the player just hit it: may answer with a dash
	int staggerImmune; // frames after a stagger in which it can't be staggered again
	int lastDir;     // which way the player was moving when last noticed (-1 / 1, 0 = unknown)
	bool searching;  // sweeping the area around the last known position
	int searchX;     // current point of the sweep
	int searchDir;   // side of the last known position the next sweep point goes to (-1 / 1)
	int searchWait;  // frames left looking around before moving to the next sweep point
	float energy;    // Droid Laser: energy left in the charge bar (float to allow beam fallback when the other modes do not fit)
	float maxEnergy; // Droid Laser: full charge (0 = not set up yet)
	int laserMode;   // Droid Laser: preferred firing mode (LASER_MODE_*), re-rolled after every reload
	int volleyMode;  // Droid Laser: mode of the volley being fired
	int volleyTotal; // Droid Laser: shots in the current volley
	int volleyIndex; // Droid Laser: shots already fired in the current volley
	int sweepDir;    // Droid Laser: direction the sweep crosses the player (-1 / 1)
	int beamDir;     // Droid Laser: side the beam starts from, and the way it sweeps across the player (-1 / 1)
	int beamTimer;   // Droid Laser: frames left of the beam being fired (0 = no beam)
	int beamTotal;   // Droid Laser: length of the current beam in frames
	int beamTick;    // Droid Laser: frames until the beam may hurt Bob again
	float beamAngle; // Droid Laser: direction of the beam (radians, y grows downward)
	float beamStep;  // Droid Laser: how much the beam turns each frame (radians)
	float beamLen;   // Droid Laser: length of the beam this frame (pixels, up to a wall or Bob)
	bool hasCover;   // safe mode: a hiding spot has been chosen (coverX / coverY)
	int coverX, coverY; // top-left position of that spot
	int coverRetry;  // frames until the next cover search after a failed one
	int coverCheck;  // frames until the chosen spot is checked against Bob's position again
	int coverTimer;  // frames until the next check that it is getting closer to the cover
	int snapX, snapY; // where the droid was at the last of those checks
	bool hasBadCover; // a spot it could not reach (stuck on the way): not chosen again
	int badCoverX, badCoverY; // that spot

	EnemyAIState() : telegraph(0), fireNow(false), fireTimeout(0), awareness(0), lostSight(0), alerted(false), canSee(false), lastX(0), lastY(0), prevX(0), stuck(0), maxHealth(0), burstLeft(0), burstTimeout(0), attackStamp(0), rank(0), squadLeader(NULL), confused(0), panicDir(1), grenadeX(0), grenadeY(0), grenadeTimer(0), telegraphTotal(0), charging(false), utilityDroid(false), shield(0), maxShield(0), dashes(0), rechargeTimer(0), stagger(0), safeMode(false), dashCooldown(0), dashReact(false), staggerImmune(0), lastDir(0), searching(false), searchX(0), searchDir(0), searchWait(0), energy(0.0f), maxEnergy(0.0f), laserMode(0), volleyMode(0), volleyTotal(0), volleyIndex(0), sweepDir(1), beamDir(1), beamTimer(0), beamTotal(0), beamTick(0), beamAngle(0.0f), beamStep(0.0f), beamLen(0.0f), hasCover(false), coverX(0), coverY(0), coverRetry(0), coverCheck(0), coverTimer(0), snapX(0), snapY(0), hasBadCover(false), badCoverX(0), badCoverY(0) {}
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

// Droid Laser (weapon 23 in data/weapons): magazine, reload and firing modes of the Eye Droids
static const int WEAPON_DROID_LASER = 23;
static const float LASER_CLIP_SIZE = 12.0f;    // energy before it has to reload
static const int LASER_RELOAD_FRAMES = 150;     // reload time on the easiest difficulty
static const int LASER_RELOAD_SKILL_CUT = 20;   // frames shaved off per difficulty level
static const int LASER_MODE_SINGLE = 0;
static const int LASER_MODE_BURST = 1;
static const int LASER_MODE_SWEEP = 2;          // fan-shaped volley crossing the player
static const int LASER_MODE_BEAM = 3;           // continuous beam
static const int LASER_BURST_CHANCE = 35;       // % of volleys that are bursts
static const int LASER_SWEEP_BASE_CHANCE = 10;  // % of sweeps on the easiest difficulty...
static const int LASER_SWEEP_SKILL_CHANCE = 10; // ...plus this per difficulty level
static const int LASER_SWEEP_SHOTS = 4;         // bullets in a sweep, plus 1 per difficulty level
static const int LASER_SWEEP_GAP = 5;           // frames between the bullets of a sweep
static const int LASER_SWEEP_WARNING = 12;      // extra warning before a sweep
static const float LASER_SWEEP_HALF_WIDTH = 55.0f; // pixels around the player the sweep covers
static const float LASER_SWEEP_MIN_ANGLE = 4.0f;   // degrees, half width of the fan
static const float LASER_SWEEP_MAX_ANGLE = 25.0f;
static const int LASER_BEAM_BASE_CHANCE = 10;   // % of beams on the easiest difficulty...
static const int LASER_BEAM_SKILL_CHANCE = 10;  // ...plus this per difficulty level
static const int LASER_BEAM_FRAMES = 90;        // how long the beam stays on (1.5 s)
static const float LASER_BEAM_CLIP_COST = 3.0f; // energy one whole beam uses; it can still fire when other modes do not fit
static const int LASER_BEAM_WARNING = 12;       // extra warning before a beam
static const int LASER_BEAM_TICK = 15;          // frames between two hits while the beam touches Bob
static const int LASER_BEAM_DAMAGE = 1;         // health Bob loses per hit (health is a whole number: the beam is weaker through its rate, not through each hit)
static const int LASER_BEAM_RANGE = 720;        // longest the beam can be (pixels)
static const int LASER_BEAM_STEP = 4;           // pixels between the points tested along the beam
static const float LASER_BEAM_HALF_WIDTH = 55.0f; // pixels around the player the beam sweeps across
static const float LASER_BEAM_MIN_ANGLE = 4.0f;   // degrees, half width of the sweep
static const float LASER_BEAM_MAX_ANGLE = 25.0f;

// Utility droid (the common Eye Droid): electromagnetic shield, dash pool and recharge
static const char *UTILITY_DROID_NAME = "Eye Droid V1.0";
static const int DROID_SHIELD_POINTS = 5;       // shield points at spawn
static const int DROID_MAX_DASHES = 3;          // teleport dashes in the pool
static const int DASH_SHIELD_COST = 1;          // shield points one dash consumes (the shield has DROID_SHIELD_POINTS)
static const int DROID_RECHARGE_FRAMES = MAX_FPS * 10; // once the shield breaks (or the dashes run out) it is back, full, after 10 s (not before)
static const int DROID_RECHARGE_CAP = MAX_FPS * 10;    // the aggression penalty can never push the timer past this
static const int DROID_STAGGER_FRAMES = 4;      // a direct hit with the shield down freezes it this long
static const int DROID_STAGGER_IMMUNITY = 20;   // ...and it can't be frozen again for this long (no stun-lock)
static const int DROID_DEFLECT_COST = 2;        // shield points one grenade deflection consumes
static const int DROID_DEFLECT_MARGIN = 40;     // the repulsor field reaches this far (pixels) beyond the droid's half size
static const int DASH_MIN_RADIUS = 64;          // a dash jumps at least this far (pixels)
static const int DASH_MAX_RADIUS = 160;         // ...and at most this far
static const int DASH_TRIES = 8;                // candidate spots tested per dash
static const int DASH_COOLDOWN = 40;            // frames between two dashes
static const int DASH_HIT_CHANCE = 0;           // % chance to dash after being hit (0: it no longer reacts to hits, it dodges the shot itself)
static const int DASH_DODGE_CHANCE = 100;       // % chance that a droid in the line of fire dashes the instant the player shoots
static const int DASH_DODGE_RANGE = 1000;       // farthest a droid reacts to a shot (pixels along the line of fire)
static const int DASH_DODGE_CLEARANCE = 40;     // extra pixels a dodge spot keeps from the line of fire (on top of the droid's half size)
static const int DASH_CLOSE_RANGE = 110;        // the player this close (pixels) makes it dash away
static const int DASH_CLOSE_CHANCE = 8;         // % per frame while the player is that close
static const int DASH_IDLE_PERMILLE = 4;        // per-mille per frame of a random dash while alert and seeing the player

// Safe mode: starts when the shield breaks and lasts DROID_RECHARGE_FRAMES from that moment
static const int DROID_SAFE_MOVE_PERIOD = 3;    // it crawls: it only moves on one frame in this many
static const int DASH_SAFE_COOLDOWN = 20;       // frames between dashes in safe mode (dodging is what it concentrates on)
static const int DASH_SAFE_CLOSE_CHANCE = 30;   // % per frame to dash away while Bob is within DASH_CLOSE_RANGE
static const int DROID_COVER_STEP = 48;         // cover points are looked for in rings this far apart (pixels)...
static const int DROID_COVER_RADIUS = 288;      // ...up to this far from the droid
static const int DROID_COVER_DIRS = 12;         // points tested per ring
static const int DROID_COVER_RECHECK = 15;      // frames between checks that the chosen cover is still hidden from Bob
static const int DROID_COVER_RETRY = 30;        // frames to wait before searching again after finding nothing
static const int DROID_COVER_STUCK_FRAMES = 45; // frames between the checks that it is getting closer to its cover
static const int DROID_COVER_STUCK_MOVE = 3;    // moving less than this (pixels) between two checks = stuck, the spot is dropped
static const int DROID_COVER_ARRIVE_DIST = 8;   // this close (pixels, x plus y) to the spot counts as arrived
static const int DASH_TRIES_HIDDEN = 16;        // candidate spots tested per dash in safe mode (it looks for one Bob can't see)
static const int DROID_AGGRO_BOB_HEALTH = 2;    // Bob with this much health or less (and in its sights) tempts it to shoot
static const int DROID_AGGRO_CHANCE = 10;       // % per frame that it gives in to that temptation
static const int DROID_AGGRO_BURST_RANGE = 200; // closer than this (pixels) it prefers a burst, farther a beam
static const int DROID_AGGRO_PENALTY_FRAMES = MAX_FPS * 5; // a single shot from safe mode adds this to the recharge (up to DROID_RECHARGE_CAP)

static unsigned int aiFrame = 0;        // frame counter, used for the attack turns
static int attackCooldown = 0;          // frames until another enemy may start attacking
static int lastPlayerReload = 0;        // used to notice when the player fires

static EnemyAIState &getAIState(Entity *enemy)
{
	return aiState[enemy];
}

static bool isUtilityDroid(Entity *enemy)
{
	return (strcmp(enemy->name, UTILITY_DROID_NAME) == 0);
}

// Gives a freshly spawned utility droid its full shield and dash pool
static void initUtilityDroid(Entity *enemy, EnemyAIState &st)
{
	if (!isUtilityDroid(enemy))
		return;

	st.utilityDroid = true;
	st.shield = st.maxShield = DROID_SHIELD_POINTS;
	st.dashes = DROID_MAX_DASHES;
	st.rechargeTimer = 0;
	st.stagger = 0;
	st.safeMode = false;
	st.dashCooldown = 0;
	st.dashReact = false;
	st.staggerImmune = 0;
}

// The shield broke or the dash pool ran dry: safe mode. Weapons off (droidSafeAggression is the one exception),
// crawling, dodging with whatever is left of the dash pool, looking for cover. The 10 s recharge counts from here.
static void enterDroidSafeMode(EnemyAIState &ai)
{
	if (ai.safeMode)
		return;

	ai.safeMode = true;
	ai.rechargeTimer = DROID_RECHARGE_FRAMES;

	// whatever it was winding up or firing is cancelled
	ai.telegraph = 0;
	ai.fireNow = false;
	ai.charging = false;
	ai.burstLeft = 0;
	ai.volleyTotal = 0;
	ai.volleyIndex = 0;
	ai.beamTimer = 0;

	ai.hasCover = false;
	ai.coverRetry = 0;
	ai.coverCheck = 0;
	ai.coverTimer = 0;
	ai.hasBadCover = false;
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
// Movement priorities. When several rules want to move an enemy in the same frame
// the highest priority wins; on a tie the rule asked last wins. The base destinations
// (last seen point, confusion, getting unstuck) are written straight to tx earlier in
// doAI(), so any proposal below beats them.
static const int MOVE_PRIO_SEARCH = 0;     // sweep the area around the last known position
static const int MOVE_PRIO_DISTANCE = 1;   // hold, approach or back off to the preferred distance
static const int MOVE_PRIO_SQUAD = 2;      // reserved for the squad post (not used yet)
static const int MOVE_PRIO_COVER = 2;      // safe mode: go to cover (shares the slot of the unused squad post)
static const int MOVE_PRIO_SEPARATION = 3; // don't stack on other enemies
static const int MOVE_PRIO_GRENADE = 4;    // get away from a grenade (survival)

// Active search: after losing sight of the player, an enemy that reaches the last known
// position sweeps the area around it (one side, then the other) until it calms down.
static const int SEARCH_ARRIVE_DIST = 40;     // close enough to the last known position to start sweeping
static const int SEARCH_POINT_TOLERANCE = 8;  // close enough to a sweep point to count as arrived
static const int SEARCH_MIN_OFFSET = 50;      // sweep points are this far from the last known position...
static const int SEARCH_MAX_OFFSET = 150;     // ...up to this far
static const int SEARCH_WAIT_MIN = 40;        // frames spent looking around at each point
static const int SEARCH_WAIT_MAX = 80;

struct MoveProposal
{
	bool has;
	int prio;
	int tx;

	MoveProposal() : has(false), prio(0), tx(0) {}

	void propose(int p, int x)
	{
		if ((!has) || (p >= prio))
		{
			has = true;
			prio = p;
			tx = x;
		}
	}
};

static bool avoidPlayerGrenade(Entity *enemy, MoveProposal &mv)
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
				mv.propose(MOVE_PRIO_GRENADE, (int)(enemy->x + 100));
			else
				mv.propose(MOVE_PRIO_GRENADE, (int)(enemy->x - 100));

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

	EnemyAIState &spawnState = getAIState(enemy);
	spawnState.maxHealth = enemy->health;
	initUtilityDroid(enemy, spawnState);
	
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

// Straight ray from the point (x, y) to Bob: false when a solid brick is in the way
static bool clearShotFromPoint(float x, float y)
{
	int mx, my;
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

bool hasClearShot(Entity *enemy)
{
	return clearShotFromPoint(enemy->x + (enemy->width / 2), enemy->y + (enemy->height / 2));
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

// ---------------------------------------------------------------------------
// Noise. Something loud happened at (x, y): enemies that can hear it go and
// investigate. The range grows with the volume, shrinks for every solid tile in
// the way, and the spot they head for is only approximate (worse when far away).
// It never raises anyone above "?", and enemies that are already alert ignore it
// (they know where the player is). Called from player.cpp and explosions.cpp.
// ---------------------------------------------------------------------------
static const float NOISE_VERTICAL_RATIO = 0.55f; // sound carries less across floors: vertical reach = volume * this
static const float NOISE_WALL_PENALTY = 0.35f;   // each solid tile in the way adds this fraction to the distance
static const int NOISE_MAX_WALLS = 6;
static const int NOISE_ERROR_BASE = 24;          // the heard position is off by this many pixels...
static const int NOISE_ERROR_DIVISOR = 6;        // ...plus distance / this

static int countSolidTilesBetween(float x0, float y0, float x1, float y1)
{
	float dx = x1 - x0;
	float dy = y1 - y0;
	float length = sqrtf((dx * dx) + (dy * dy));
	int steps = (int)(length / (BRICKSIZE / 2));

	if (steps < 2)
		return 0;

	int count = 0;
	int lastTX = -1000;
	int lastTY = -1000;

	for (int i = 1 ; i < steps ; i++)
	{
		float t = (float)i / steps;
		int tileX = ((int)(x0 + (dx * t))) >> BRICKSHIFT;
		int tileY = ((int)(y0 + (dy * t))) >> BRICKSHIFT;

		if ((tileX == lastTX) && (tileY == lastTY))
			continue;

		lastTX = tileX;
		lastTY = tileY;

		if (map.isSolid(tileX, tileY))
		{
			if (++count >= NOISE_MAX_WALLS)
				break;
		}
	}

	return count;
}

void emitNoise(float x, float y, int volume)
{
	if ((volume <= 0) || (player.health <= -60) || (game.missionOverReason == MIS_COMPLETE))
		return;

	Entity *enemy = (Entity*)map.enemyList.getHead();

	while (enemy->next != NULL)
	{
		enemy = (Entity*)enemy->next;

		if ((enemy->health <= 0) || (enemy->owner != enemy))
			continue;

		if (enemy->flags & (ENT_BOSS|ENT_STATIC|ENT_GALDOV|ENT_NOMOVE|ENT_INANIMATE))
			continue;

		float ex = enemy->x + (enemy->width / 2);
		float ey = enemy->y + (enemy->height / 2);
		float dx = ex - x;
		float dy = ey - y;
		float dist = sqrtf((dx * dx) + (dy * dy));

		if ((dist > volume) || (fabsf(dy) > (volume * NOISE_VERTICAL_RATIO)))
			continue;

		float effective = dist * (1.0f + (NOISE_WALL_PENALTY * countSolidTilesBetween(x, y, ex, ey)));

		if (effective > volume)
			continue;

		EnemyAIState &st = getAIState(enemy);

		if (st.awareness >= AWARE_ALERT)
			continue;

		// louder and closer = more suspicious, between "?" and what a gunshot does
		float closeness = 1.0f - (effective / volume);
		float level = AWARE_SUSPICIOUS + ((HEAR_LEVEL - AWARE_SUSPICIOUS) * closeness);

		if (st.awareness < level)
			st.awareness = level;

		int error = NOISE_ERROR_BASE + ((int)dist / NOISE_ERROR_DIVISOR);

		st.lastX = (int)x + Math::rrand(-error, error);
		st.lastY = (int)y;
		st.lostSight = 0;

		enemy->tx = st.lastX;
		enemy->ty = st.lastY;
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

// ---- DROID LASER BEGIN ----
static bool isDroidLaser(Entity *enemy)
{
	return (enemy->currentWeapon == &weapon[WEAPON_DROID_LASER]);
}

static int pickLaserMode()
{
	int roll = Math::prand() % 100;
	int beamChance = LASER_BEAM_BASE_CHANCE + (game.skill * LASER_BEAM_SKILL_CHANCE);
	int sweepChance = LASER_SWEEP_BASE_CHANCE + (game.skill * LASER_SWEEP_SKILL_CHANCE);

	if (roll < beamChance)
		return LASER_MODE_BEAM;

	if (roll < (beamChance + sweepChance))
		return LASER_MODE_SWEEP;

	if (roll < (beamChance + sweepChance + LASER_BURST_CHANCE))
		return LASER_MODE_BURST;

	return LASER_MODE_SINGLE;
}

// Chooses a mode with a bit of tactical sense: near = burst, medium = sweep, far = beam if it has enough energy.
static int pickLaserModeForEnemy(Entity *enemy, EnemyAIState &st)
{
	if (st.energy < 2.0f)
		return LASER_MODE_SINGLE;

	float dx = fabsf((player.x + (player.width / 2)) - (enemy->x + (enemy->width / 2)));
	float dy = fabsf((player.y + (player.height / 2)) - (enemy->y + (enemy->height / 2)));
	float dist = sqrtf((dx * dx) + (dy * dy));

	if (dist < 200.0f)
		return (st.energy >= 3.0f) ? LASER_MODE_BURST : LASER_MODE_SINGLE;

	if (dist > 440.0f)
	{
		if (st.energy >= LASER_BEAM_CLIP_COST + 1.0f)
			return (Math::prand() % 100 < 60) ? LASER_MODE_BEAM : LASER_MODE_SWEEP;

		return LASER_MODE_SWEEP;
	}

	if (st.energy >= 4.0f)
		return LASER_MODE_SWEEP;

	return (st.energy >= 3.0f) ? LASER_MODE_BURST : LASER_MODE_SINGLE;
}

// Full magazine and a first preferred mode the first time a droid needs them
static void ensureDroidLaser(Entity *enemy, EnemyAIState &st)
{
	if (st.maxEnergy > 0.0f)
		return;

	st.maxEnergy = LASER_CLIP_SIZE;
	st.energy = st.maxEnergy;
	st.laserMode = pickLaserModeForEnemy(enemy, st);
}

// Half width (radians) of the arc the beam sweeps: it covers LASER_BEAM_HALF_WIDTH pixels around the player at any distance
static float getBeamHalfAngle(Entity *enemy)
{
	float dx = (player.x + (player.width / 2)) - (enemy->x + (enemy->width / 2));
	float dy = (player.y + (player.height / 2)) - (enemy->y + (enemy->height / 2));
	float dist = sqrtf((dx * dx) + (dy * dy));

	if (dist < 1.0f)
		dist = 1.0f;

	float half = atan2f(LASER_BEAM_HALF_WIDTH, dist) * (180.0f / 3.14159265f);

	if (half < LASER_BEAM_MIN_ANGLE)
		half = LASER_BEAM_MIN_ANGLE;

	if (half > LASER_BEAM_MAX_ANGLE)
		half = LASER_BEAM_MAX_ANGLE;

	return half * (3.14159265f / 180.0f);
}

// Starts a beam. It begins to one side of the player and turns across him, so there is time to get out of the way.
// Its direction is fixed when it starts (it does not follow the player); only its origin follows the droid.
static void startDroidBeam(Entity *enemy, EnemyAIState &st)
{
	float dx = (player.x + (player.width / 2)) - (enemy->x + (enemy->width / 2));
	float dy = (player.y + (player.height / 2)) - (enemy->y + (enemy->height / 2));
	float half = getBeamHalfAngle(enemy);

	st.beamDir = ((Math::prand() % 2) == 0) ? 1 : -1;
	st.beamTotal = LASER_BEAM_FRAMES;
	st.beamTimer = LASER_BEAM_FRAMES;
	st.beamTick = 0;
	st.beamAngle = atan2f(dy, dx) - (st.beamDir * half);
	st.beamStep = (st.beamDir * 2.0f * half) / LASER_BEAM_FRAMES;
	st.beamLen = 0.0f;

	// no other shot until the beam is over (shots are blocked while enemy->reload > 0)
	enemy->reload = LASER_BEAM_FRAMES + BURST_PAUSE;

	if (enemy->currentWeapon->fireSound > -1)
		audio.playSound(enemy->currentWeapon->fireSound, CH_ANY, enemy->x);
}

// One frame of a beam: turns it, finds where it ends (the first wall, or the player) and, while it touches the player,
// hurts him every LASER_BEAM_TICK frames. Called every frame for every active enemy; does nothing unless a beam is on.
static void updateDroidBeam(Entity *enemy, EnemyAIState &st)
{
	if (st.beamTimer <= 0)
		return;

	// a dead, stunned or confused droid, or a dead player: the beam goes out
	if ((enemy->health <= 0) || (st.stagger > 0) || (st.confused > 0) || (player.health <= 0))
	{
		st.beamTimer = 0;
		return;
	}

	st.beamTimer--;
	st.beamAngle += st.beamStep;

	if (st.beamTick > 0)
		st.beamTick--;

	float ox = enemy->x + (enemy->width / 2.0f);
	float oy = enemy->y + (enemy->height / 2.0f);
	float dirX = cosf(st.beamAngle);
	float dirY = sinf(st.beamAngle);
	float len = 0.0f;
	bool hitPlayer = false;

	while (len < LASER_BEAM_RANGE)
	{
		float next = len + LASER_BEAM_STEP;
		float tx = ox + (dirX * next);
		float ty = oy + (dirY * next);

		// walls (and the edges of the map) stop it
		if ((tx < 0) || (ty < 0))
			break;

		int bx = (int)tx >> BRICKSHIFT;
		int by = (int)ty >> BRICKSHIFT;

		if ((bx >= MAPWIDTH) || (by >= MAPHEIGHT) || map.isSolid(bx, by))
			break;

		len = next;

		// the player stops it too
		if ((tx >= player.x) && (tx < (player.x + player.width)) && (ty >= player.y) && (ty < (player.y + player.height)))
		{
			hitPlayer = true;
			break;
		}
	}

	st.beamLen = len;

	float endX = ox + (dirX * len);
	float endY = oy + (dirY * len);

	if (hitPlayer && (st.beamTick <= 0))
	{
		addDroidBeamHit(enemy, endX, endY, LASER_BEAM_DAMAGE);
		st.beamTick = LASER_BEAM_TICK;
	}

	// sparks where the beam ends (on a wall or on Bob)
	if ((aiFrame % 2) == 0)
	{
		for (int i = 0 ; i < 2 ; i++)
		{
			float sx = Math::rrand(-20, 20); sx /= 10;
			float sy = Math::rrand(-20, 20); sy /= 10;
			map.addParticle(endX, endY, sx, sy, Math::rrand(5, 15), (i == 0) ? graphics.red : graphics.white, NULL, 0);
		}
	}

	// no looping sound exists: repeat the laser sound at the rate of the hits
	if (((st.beamTimer % LASER_BEAM_TICK) == 0) && (enemy->currentWeapon->fireSound > -1))
		audio.playSound(enemy->currentWeapon->fireSound, CH_ANY, enemy->x);
}

// Angle (degrees) of the next bullet of a sweep: the fan goes from one side of the player to the other
static float getSweepAngle(Entity *enemy, EnemyAIState &st)
{
	if (st.volleyTotal <= 1)
		return 0.0f;

	float dx = (player.x + (player.width / 2)) - (enemy->x + (enemy->width / 2));
	float dy = (player.y + (player.height / 2)) - (enemy->y + (enemy->height / 2));
	float dist = sqrtf((dx * dx) + (dy * dy));

	if (dist < 1.0f)
		dist = 1.0f;

	float half = atan2f(LASER_SWEEP_HALF_WIDTH, dist) * (180.0f / 3.14159265f);

	if (half < LASER_SWEEP_MIN_ANGLE)
		half = LASER_SWEEP_MIN_ANGLE;

	if (half > LASER_SWEEP_MAX_ANGLE)
		half = LASER_SWEEP_MAX_ANGLE;

	float t = (float)st.volleyIndex / (float)(st.volleyTotal - 1);

	return st.sweepDir * (-half + (2.0f * half * t));
}

// Fires the droid's current volley, one bullet per call (single shot, burst, sweep), or starts a beam.
// Spends the magazine and starts the reload when it runs dry. A new volley starts when the previous one is over.
static void fireDroidLaser(Entity *enemy, EnemyAIState &st, bool continuing, float aimDY)
{
	ensureDroidLaser(enemy, st);

	bool newVolley = ((!continuing) || (st.volleyTotal <= 0));

	// The beam is not a volley of bullets: it starts here and then runs on its own (updateDroidBeam).
	// If the current mode would need more charge than the droid has, it can still fall back to the beam.
	if (newVolley && (st.laserMode == LASER_MODE_BEAM) && (st.energy >= LASER_BEAM_CLIP_COST))
	{
		startDroidBeam(enemy, st);

		st.energy -= LASER_BEAM_CLIP_COST;
		st.burstLeft = 0;
		st.volleyTotal = 0;
	}
	else
	{
		if (newVolley)
		{
			int shots = 1;

			if (st.laserMode == LASER_MODE_BURST)
				shots = getBurstSize(enemy);
			else if (st.laserMode == LASER_MODE_SWEEP)
				shots = LASER_SWEEP_SHOTS + game.skill;

			if (st.energy < 1.0f)
			{
				if ((st.energy >= LASER_BEAM_CLIP_COST) && (st.laserMode != LASER_MODE_BEAM))
				{
					st.laserMode = LASER_MODE_BEAM;
					startDroidBeam(enemy, st);
					st.energy -= LASER_BEAM_CLIP_COST;
					st.burstLeft = 0;
					st.volleyTotal = 0;
					return;
				}

				st.energy = st.maxEnergy;
				st.laserMode = pickLaserMode();
				enemy->reload = std::max(enemy->reload, LASER_RELOAD_FRAMES - (game.skill * LASER_RELOAD_SKILL_CUT));
				return;
			}

			if ((float)shots > st.energy)
			{
				if ((st.energy >= LASER_BEAM_CLIP_COST) && (st.laserMode != LASER_MODE_BEAM))
				{
					st.laserMode = LASER_MODE_BEAM;
					startDroidBeam(enemy, st);
					st.energy -= LASER_BEAM_CLIP_COST;
					st.burstLeft = 0;
					st.volleyTotal = 0;
					return;
				}

				shots = 1;
			}

			if (shots < 1)
				shots = 1;

			st.volleyMode = st.laserMode;

			// a sweep cut short by the energy is just a burst
			if ((st.volleyMode == LASER_MODE_SWEEP) && (shots < 3))
				st.volleyMode = LASER_MODE_BURST;

			st.volleyTotal = shots;
			st.volleyIndex = 0;
			st.sweepDir = ((Math::prand() % 2) == 0) ? 1 : -1;
			st.burstLeft = shots;
			st.burstTimeout = 120;
		}

		if (st.volleyMode == LASER_MODE_SWEEP)
		{
			addEnemyAimedShot(enemy, getSweepAngle(enemy, st));
			enemy->reload = LASER_SWEEP_GAP;
		}
		else
		{
			addBullet(enemy, enemy->currentWeapon->getSpeed(enemy->face), aimDY);
		}

		st.energy -= 1.0f;
		st.volleyIndex++;
		st.burstLeft--;

		if (st.burstLeft <= 0)
		{
			// volley over: rest a little after anything longer than a single shot
			if (st.volleyIndex > 1)
				enemy->reload += BURST_PAUSE;

			st.burstLeft = 0;
			st.volleyTotal = 0;
		}
	}

	// empty magazine: reload (shots are blocked while enemy->reload > 0) and pick a new preferred mode
	if (st.energy <= 0.0f)
	{
		int reloadFrames = LASER_RELOAD_FRAMES - (game.skill * LASER_RELOAD_SKILL_CUT);

		if (enemy->reload < reloadFrames)
			enemy->reload = reloadFrames;

		st.energy = st.maxEnergy;
		st.laserMode = pickLaserModeForEnemy(enemy, st);
		st.burstLeft = 0;
		st.volleyTotal = 0;
	}
}
// ---- DROID LASER END ----

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

		// remember which way the player was heading (used to decide where to search first)
		if (player.dx > 0)
			ai.lastDir = 1;
		else if (player.dx < 0)
			ai.lastDir = -1;
		else
			ai.lastDir = (player.face == 0) ? 1 : -1;
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

// Safe mode: weapons are off, except that a weak Bob in plain sight tempts the droid to risk a shot.
// What it fires is up to it: a single shot when the magazine is nearly empty, otherwise a burst up
// close or a sweep/beam from afar. Returns true when it gives in (the penalty is applied when it fires).
static bool droidSafeAggression(Entity *enemy, EnemyAIState &st, bool canSee)
{
	if ((!canSee) || (player.health <= 0) || (player.health > DROID_AGGRO_BOB_HEALTH))
		return false;

	if ((int)(Math::prand() % 100) >= DROID_AGGRO_CHANCE)
		return false;

	if (isDroidLaser(enemy))
	{
		ensureDroidLaser(enemy, st);
		st.laserMode = pickLaserModeForEnemy(enemy, st);
	}

	return true;
}

// Shooting from safe mode costs recharge time: a single shot adds DROID_AGGRO_PENALTY_FRAMES to what
// is left (never past the 10 s cap); a burst, sweep or beam puts the countdown back to the full 10 s.
static void applyDroidAggressionPenalty(EnemyAIState &st, bool heavy)
{
	if (heavy)
		st.rechargeTimer = DROID_RECHARGE_CAP;
	else
		st.rechargeTimer = std::min(DROID_RECHARGE_CAP, st.rechargeTimer + DROID_AGGRO_PENALTY_FRAMES);
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

			if ((enemy->flags & ENT_ALWAYSFIRES) && (!st.safeMode)) // safe mode: weapons off
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
			else if (st.safeMode ? droidSafeAggression(enemy, st, canSee) : (isLeader ? ((Math::prand() % 100) < (4 + (game.skill * 3))) : ((Math::prand() % 850) <= (game.skill * 5))))
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

							// a droid about to fire a sweep or beam warns a bit longer
							if (isDroidLaser(enemy))
							{
								ensureDroidLaser(enemy, st);

								if (st.laserMode == LASER_MODE_SWEEP)
									st.telegraph += LASER_SWEEP_WARNING;
								else if (st.laserMode == LASER_MODE_BEAM)
									st.telegraph += LASER_BEAM_WARNING;
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

				// Eye Droid laser: magazine, reload and firing modes. These droids fly, so nothing below applies to them
				if (isDroidLaser(enemy) && (!(enemy->flags & ENT_ALWAYSFIRES)))
				{
					// a volley fired from safe mode puts the recharge back (burst / sweep / beam) or extends it (single shot)
					if (st.safeMode && ((!continuing) || (st.volleyTotal <= 0)))
					{
						ensureDroidLaser(enemy, st);
						applyDroidAggressionPenalty(st, (st.laserMode != LASER_MODE_SINGLE));
					}

					fireDroidLaser(enemy, st, continuing, aimDY);
					return;
				}

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
					if (st.safeMode && (!continuing))
						applyDroidAggressionPenalty(st, false);

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

	// just started the warning (or in the middle of a burst or a beam): don't jump away
	if ((st.telegraph > 0) || (st.burstLeft > 0) || (st.beamTimer > 0))
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

// True when something solid or liquid lies below the box, all the way down to the map's bottom
// edge. A spot over a bottomless gap (nothing to land on) is where a dash must never end up.
static bool hasSomethingBelow(int px, int py, int w, int h)
{
	int x1 = px >> BRICKSHIFT;
	int x2 = (px + w - 1) >> BRICKSHIFT;
	int yStart = ((py + h - 1) >> BRICKSHIFT) + 1;

	for (int ty = yStart ; ty < MAPHEIGHT ; ty++)
	{
		for (int tx = x1 ; tx <= x2 ; tx++)
		{
			if (map.isSolid(tx, ty) || map.isLiquid(tx, ty))
				return true;
		}
	}

	return false;
}

// True when the enemy box fits at (px, py): inside the map, nothing solid or liquid in it,
// not below the level's bottom edge and not over a bottomless gap
static bool isDashSpotFree(Entity *enemy, int px, int py)
{
	if ((px < 16) || (py < 16) || ((px + enemy->width) > ((MAPWIDTH * BRICKSIZE) - 16)) || ((py + enemy->height) > ((MAPHEIGHT * BRICKSIZE) - 16)))
		return false;

	// same bottom edge doGame uses to decide Bob fell out of the map (limitDown + 500)
	if ((map.limitDown > 0) && ((py + enemy->height) > (map.limitDown + 500)))
		return false;

	int x1 = px >> BRICKSHIFT;
	int x2 = (px + enemy->width - 1) >> BRICKSHIFT;
	int y1 = py >> BRICKSHIFT;
	int y2 = (py + enemy->height - 1) >> BRICKSHIFT;

	for (int ty = y1 ; ty <= y2 ; ty++)
	{
		for (int tx = x1 ; tx <= x2 ; tx++)
		{
			if (map.isSolid(tx, ty) || map.isLiquid(tx, ty))
				return false;
		}
	}

	return hasSomethingBelow(px, py, enemy->width, enemy->height);
}

// Looks for a spot to teleport to. evade = prefer the one farthest from the player,
// otherwise any valid spot (random, so the jumps are hard to predict)
// line (optional) = {originX, originY, dirX, dirY, clearance}: a line of fire the spot must stay
// clearance pixels away from; the spot farthest from that line wins.
// preferHidden (safe mode) = a spot Bob can't see always beats one he can; among the same kind the usual score decides.
static bool findDashSpot(Entity *enemy, bool evade, int *outX, int *outY, const float *line = NULL, bool preferHidden = false)
{
	bool found = false;
	int bestScore = -1;
	int tries = preferHidden ? DASH_TRIES_HIDDEN : DASH_TRIES;

	for (int i = 0 ; i < tries ; i++)
	{
		int ox = Math::rrand(-DASH_MAX_RADIUS, DASH_MAX_RADIUS);
		int oy = Math::rrand(-DASH_MAX_RADIUS, DASH_MAX_RADIUS);
		int d2 = (ox * ox) + (oy * oy);

		if ((d2 < (DASH_MIN_RADIUS * DASH_MIN_RADIUS)) || (d2 > (DASH_MAX_RADIUS * DASH_MAX_RADIUS)))
			continue;

		int px = (int)enemy->x + ox;
		int py = (int)enemy->y + oy;

		if (!isDashSpotFree(enemy, px, py))
			continue;

		float lineDist = 0.0f;

		if (line != NULL)
		{
			float cx = (px + (enemy->width / 2.0f)) - line[0];
			float cy = (py + (enemy->height / 2.0f)) - line[1];

			lineDist = fabsf((cx * line[3]) - (cy * line[2]));

			if (lineDist < line[4])
				continue;
		}

		int score = (int)(Math::prand() % 1000);

		if (line != NULL)
		{
			score = (int)lineDist;
		}
		else if (evade)
		{
			int bx = px - (int)player.x;
			int by = py - (int)player.y;
			score = (bx * bx) + (by * by);
		}

		if (preferHidden && (!clearShotFromPoint(px + (enemy->width / 2.0f), py + (enemy->height / 2.0f))))
			score += 1000000;

		if (score > bestScore)
		{
			bestScore = score;
			*outX = px;
			*outY = py;
			found = true;
		}
	}

	return found;
}

// True when the straight line between two points crosses no solid brick (the droid can fly it)
static bool isFlightPathClear(float x0, float y0, float x1, float y1)
{
	float dx = x1 - x0;
	float dy = y1 - y0;
	int steps = (int)(sqrtf((dx * dx) + (dy * dy)) / 8.0f) + 1;

	for (int i = 1 ; i <= steps ; i++)
	{
		float t = (float)i / (float)steps;
		int mx = (int)(x0 + (dx * t)) >> BRICKSHIFT;
		int my = (int)(y0 + (dy * t)) >> BRICKSHIFT;

		if ((mx < 0) || (my < 0) || (mx >= MAPWIDTH) || (my >= MAPHEIGHT) || map.isSolid(mx, my))
			return false;
	}

	return true;
}

// Cover: the nearest free spot (rings of growing radius around the droid) from which a ray to Bob
// hits a wall (no hasClearShot) and that the droid can reach in a straight line.
static bool findCoverSpot(Entity *enemy, const EnemyAIState &ai, int *outX, int *outY)
{
	float fromX = enemy->x + (enemy->width / 2.0f);
	float fromY = enemy->y + (enemy->height / 2.0f);
	int first = (int)(Math::prand() % DROID_COVER_DIRS); // where each ring starts, so it doesn't always prefer one side

	for (int r = DROID_COVER_STEP ; r <= DROID_COVER_RADIUS ; r += DROID_COVER_STEP)
	{
		for (int i = 0 ; i < DROID_COVER_DIRS ; i++)
		{
			float angle = (6.2831853f * (float)((first + i) % DROID_COVER_DIRS)) / (float)DROID_COVER_DIRS;
			int px = (int)enemy->x + (int)(cosf(angle) * r);
			int py = (int)enemy->y + (int)(sinf(angle) * r);

			if (!isDashSpotFree(enemy, px, py))
				continue;

			// the spot it got stuck on the way to (and its surroundings) is not tried again
			if (ai.hasBadCover && (abs(px - ai.badCoverX) < DROID_COVER_STEP) && (abs(py - ai.badCoverY) < DROID_COVER_STEP))
				continue;

			float cx = px + (enemy->width / 2.0f);
			float cy = py + (enemy->height / 2.0f);

			if (clearShotFromPoint(cx, cy))
				continue;

			if (!isFlightPathClear(fromX, fromY, cx, cy))
				continue;

			*outX = px;
			*outY = py;
			return true;
		}
	}

	return false;
}

// Safe mode: keeps track of where the droid should hide. Already out of Bob's sight = stay where it is.
static void updateDroidCover(Entity *enemy, EnemyAIState &ai)
{
	if ((!ai.utilityDroid) || (!ai.safeMode) || (enemy->health <= 0))
		return;

	if (ai.coverRetry > 0)
		ai.coverRetry--;

	if (ai.coverCheck > 0)
		ai.coverCheck--;

	if (!clearShotFromPoint(enemy->x + (enemy->width / 2.0f), enemy->y + (enemy->height / 2.0f)))
	{
		ai.hasCover = true;
		ai.coverX = (int)enemy->x;
		ai.coverY = (int)enemy->y;
		ai.coverCheck = 0; // the moment Bob moves and it is seen again, the spot is checked at once
		return;
	}

	// heading for cover but not getting any closer (a wall in the way): drop that spot and look for another
	if (ai.hasCover && ((abs((int)enemy->x - ai.coverX) + abs((int)enemy->y - ai.coverY)) > DROID_COVER_ARRIVE_DIST))
	{
		if (--ai.coverTimer <= 0)
		{
			ai.coverTimer = DROID_COVER_STUCK_FRAMES;

			if ((abs((int)enemy->x - ai.snapX) + abs((int)enemy->y - ai.snapY)) < DROID_COVER_STUCK_MOVE)
			{
				ai.hasBadCover = true;
				ai.badCoverX = ai.coverX;
				ai.badCoverY = ai.coverY;
				ai.hasCover = false;
				ai.coverRetry = 0;
			}

			ai.snapX = (int)enemy->x;
			ai.snapY = (int)enemy->y;
		}
	}

	// seen: the spot it was heading for may be stale now that Bob moved
	if (ai.hasCover && (ai.coverCheck <= 0))
	{
		ai.coverCheck = DROID_COVER_RECHECK;

		if (clearShotFromPoint(ai.coverX + (enemy->width / 2.0f), ai.coverY + (enemy->height / 2.0f)))
			ai.hasCover = false;
	}

	if ((!ai.hasCover) && (ai.coverRetry <= 0))
	{
		int nx, ny;

		if (findCoverSpot(enemy, ai, &nx, &ny))
		{
			ai.hasCover = true;
			ai.coverX = nx;
			ai.coverY = ny;
			ai.coverCheck = DROID_COVER_RECHECK;
			ai.coverTimer = DROID_COVER_STUCK_FRAMES;
			ai.snapX = (int)enemy->x;
			ai.snapY = (int)enemy->y;
		}
		else
		{
			ai.coverRetry = DROID_COVER_RETRY;
		}
	}
}

static void addDroidDebris(Entity *enemy, int count, float speed); // defined below

// The jump itself: instant x/y change, silent (no sound, no travel animation). Costs one dash
// from the pool and DASH_SHIELD_COST shield points.
static void performDroidDash(Entity *enemy, EnemyAIState &ai, int nx, int ny)
{
	enemy->x = nx;
	enemy->y = ny;
	ai.dashes--;

	// in safe mode the shield is already down: the jump runs on the dash pool alone
	if (ai.shield > 0)
	{
		ai.shield -= DASH_SHIELD_COST;   // the jump drains the shield

		if (ai.shield <= 0)
		{
			ai.shield = 0;
			addDroidDebris(enemy, 18, 3.0f); // drained dry: the shield collapses like a broken one
			enterDroidSafeMode(ai);
		}
	}

	// the last dash of the pool: it is out of tricks, safe mode starts (shield or not)
	if (ai.dashes <= 0)
		enterDroidSafeMode(ai);

	ai.dashCooldown = ai.safeMode ? DASH_SAFE_COOLDOWN : DASH_COOLDOWN;
	ai.prevX = nx;   // a jump is not "being stuck"
	ai.stuck = 0;
}

// Utility droid dashes: an instant change of x and y, no travel animation.
// The pool (DROID_MAX_DASHES) holds 3 dashes. Each dash costs 1 shield point, so a dash needs
// a live shield. The pool refills together with the shield (updateDroidShield).
static void updateDroidDash(Entity *enemy, EnemyAIState &ai)
{
	if (!ai.utilityDroid)
		return;

	if (ai.dashCooldown > 0)
		ai.dashCooldown--;

	bool react = ai.dashReact;
	ai.dashReact = false;

	// without a shield the pool is only usable in safe mode (dodging is all it has left)
	bool shieldOk = (ai.shield > 0) || ai.safeMode;

	if ((enemy->health <= 0) || (player.health <= 0) || (ai.dashes <= 0) || (!shieldOk) || (ai.dashCooldown > 0) || (ai.stagger > 0) || (ai.confused > 0))
		return;

	bool want = false;
	bool evade = false;

	if (react)
	{
		want = ((int)(Math::prand() % 100) < DASH_HIT_CHANCE);
	}
	else if ((ai.alerted || ai.safeMode) && ai.canSee)
	{
		int dx = (int)(player.x - enemy->x);
		int dy = (int)(player.y - enemy->y);

		if ((((dx * dx) + (dy * dy)) < (DASH_CLOSE_RANGE * DASH_CLOSE_RANGE)) && ((int)(Math::prand() % 100) < (ai.safeMode ? DASH_SAFE_CLOSE_CHANCE : DASH_CLOSE_CHANCE)))
		{
			want = true;
			evade = true;
		}
		else if ((!ai.safeMode) && ((int)(Math::prand() % 1000) < DASH_IDLE_PERMILLE)) // safe mode keeps the pool for dodging
		{
			want = true;
		}
	}

	if (!want)
		return;

	int nx, ny;

	if (!findDashSpot(enemy, evade, &nx, &ny, NULL, ai.safeMode))
		return;

	performDroidDash(enemy, ai, nx, ny);
}

// Called from addBullet() (bullets.cpp) at the instant the player fires a straight shot.
// originX/Y = where the shot leaves Bob, dirX/Y = its direction. A utility droid standing in
// that line (having detected Bob, with a clear shot, in range, dash ready and shield up) dashes right away, to a spot
// that is NOT on the line of fire. Bullets already in the air are never looked at.
void notifyPlayerShot(float originX, float originY, float dirX, float dirY)
{
	float len = sqrtf((dirX * dirX) + (dirY * dirY));

	if ((len < 0.01f) || (player.health <= 0))
		return;

	dirX /= len;
	dirY /= len;

	Entity *enemy = (Entity*)map.enemyList.getHead();

	while (enemy->next != NULL)
	{
		enemy = (Entity*)enemy->next;

		if (enemy->health <= 0)
			continue;

		std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

		if ((it == aiState.end()) || (!it->second.utilityDroid))
			continue;

		EnemyAIState &ai = it->second;

		// a droid that has not noticed Bob (no "!" yet) is not expecting the shot: it doesn't dodge
		if ((ai.awareness < AWARE_ALERT) && (!ai.safeMode)) // in safe mode it is watching Bob closely
			continue;

		if ((ai.dashes <= 0) || ((ai.shield <= 0) && (!ai.safeMode)) || (ai.dashCooldown > 0) || (ai.stagger > 0) || (ai.confused > 0))
			continue;

		float ex = (enemy->x + (enemy->width / 2.0f)) - originX;
		float ey = (enemy->y + (enemy->height / 2.0f)) - originY;
		float along = (ex * dirX) + (ey * dirY);
		float across = fabsf((ex * dirY) - (ey * dirX));
		float reach = (((enemy->width > enemy->height) ? enemy->width : enemy->height) / 2.0f) + 6.0f;

		if ((along <= 0.0f) || (along > DASH_DODGE_RANGE) || (across > reach))
			continue;

		if (!hasClearShot(enemy)) // a wall between them: the shot can't reach it anyway
			continue;

		if ((int)(Math::prand() % 100) >= DASH_DODGE_CHANCE)
			continue;

		float line[5] = {originX, originY, dirX, dirY, reach + DASH_DODGE_CLEARANCE};
		int nx, ny;

		if (!findDashSpot(enemy, false, &nx, &ny, line, ai.safeMode))
			continue;

		performDroidDash(enemy, ai, nx, ny);
	}
}

// Utility droid recharge. A broken shield or an empty dash pool starts safe mode and its 10 s countdown.
// Nothing comes back while it runs; when it ends the shield and the dash pool are both restored in full
// and safe mode ends.
static void updateDroidShield(Entity *enemy, EnemyAIState &ai)
{
	if ((!ai.utilityDroid) || (enemy->health <= 0))
		return;

	// safety net: whatever took the last shield point or the last dash, safe mode starts here (and its 10 s count from now)
	if ((ai.shield <= 0) || (ai.dashes <= 0))
		enterDroidSafeMode(ai);

	if ((ai.shield > 0) && (ai.dashes > 0))
	{
		ai.rechargeTimer = 0;
		return;
	}

	if (ai.rechargeTimer <= 0)
		ai.rechargeTimer = DROID_RECHARGE_FRAMES; // shield down or no dashes left, and no countdown running: start it

	if (--ai.rechargeTimer > 0)
		return;

	ai.shield = ai.maxShield;
	ai.dashes = DROID_MAX_DASHES;
	ai.safeMode = false;
	ai.hasCover = false;

	// small cyan burst so the player sees it come back
	for (int i = 0 ; i < 12 ; i++)
	{
		float dx = Math::rrand(-20, 20); dx /= 10;
		float dy = Math::rrand(-20, 20); dy /= 10;
		map.addParticle(enemy->x + (enemy->width / 2), enemy->y + (enemy->height / 2), dx, dy, Math::rrand(15, 35), graphics.cyan, NULL, 0);
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
	if ((ai.telegraph > 0) || (ai.burstLeft > 0) || (ai.beamTimer > 0))
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

	// utility droid: teleport dashes
	updateDroidDash(enemy, ai);

	// utility droid: shield comes back 10 s after it broke
	updateDroidShield(enemy, ai);

	// utility droid in safe mode: find (and keep) a spot Bob can't see
	updateDroidCover(enemy, ai);

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

	// Several rules want to move the enemy: each one proposes a destination with a
	// priority and the highest wins (see MOVE_PRIO_*), instead of the last writer winning.
	MoveProposal mv;

	// Active search: it lost sight of the player. Once it reaches the last known position it
	// sweeps the area around it (first the side the player was heading to, then the other)
	// instead of standing still, until it calms down.
	bool canSearch = aware && (!ai.canSee) && (ai.lostSight > 0) && (ai.confused == 0) &&
		(!(enemy->flags & (ENT_FLIES|ENT_SWIMS|ENT_STATIC|ENT_NOMOVE|ENT_INANIMATE|ENT_GALDOV|ENT_BOSS)));

	if (!canSearch)
	{
		ai.searching = false;
	}
	else
	{
		int here = (int)enemy->x;

		if ((!ai.searching) && (abs(here - ai.lastX) <= SEARCH_ARRIVE_DIST))
		{
			ai.searching = true;
			ai.searchX = here;
			ai.searchDir = ai.lastDir;
			ai.searchWait = Math::rrand(SEARCH_WAIT_MIN, SEARCH_WAIT_MAX);
		}

		if (ai.searching)
		{
			if (abs(here - ai.searchX) <= SEARCH_POINT_TOLERANCE)
			{
				if (ai.searchWait > 0)
				{
					ai.searchWait--;
				}
				else
				{
					if (ai.searchDir == 0)
						ai.searchDir = ((Math::prand() % 2) == 0) ? 1 : -1;

					ai.searchX = ai.lastX + (ai.searchDir * Math::rrand(SEARCH_MIN_OFFSET, SEARCH_MAX_OFFSET));
					Math::limitInt(&ai.searchX, 15, (MAPWIDTH * BRICKSIZE) - 20);
					ai.searchDir = -ai.searchDir;
					ai.searchWait = Math::rrand(SEARCH_WAIT_MIN, SEARCH_WAIT_MAX);
				}
			}

			mv.propose(MOVE_PRIO_SEARCH, ai.searchX);
		}
	}

	// An aware enemy that can't get any closer (wall in the way) tries to jump
	// it, and gives up after a while
	int curX = (int)enemy->x;

	// while sweeping, the goal is the sweep point (tx is reset to the last known position every frame)
	int goalX = mv.has ? mv.tx : enemy->tx;

	if (aware && (goalX != curX) && (!enemy->falling) && (ai.telegraph == 0) && (ai.burstLeft == 0) && (ai.beamTimer == 0) && (curX == ai.prevX))
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

		// a sweep point it can't reach: count it as reached so it moves on to the other side
		if (ai.searching)
			ai.searchX = curX;
	}

	// Calm enemies that stopped moving forget their destination. Aware ones
	// keep it: otherwise a standing enemy could never start chasing.
	if ((enemy->dx == 0) && (!aware))
		enemy->tx = (int)enemy->x;

	// AVOID PLAYER'S GRENADES (highest priority - survival)
	if (avoidPlayerGrenade(enemy, mv))
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
					mv.propose(MOVE_PRIO_DISTANCE, (int)(enemy->x + 100));
				else
					mv.propose(MOVE_PRIO_DISTANCE, (int)(enemy->x - 100));
			}
			else if (currentDist > (prefDist + 50))
			{
				// Too far: move toward player (already handled by ai.lastX/lastY)
				mv.propose(MOVE_PRIO_DISTANCE, ai.lastX);
			}
			else
			{
				// At preferred distance: hold position
				mv.propose(MOVE_PRIO_DISTANCE, (int)enemy->x);
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
				mv.propose(MOVE_PRIO_GRENADE, (int)(enemy->x + 100));
			else
				mv.propose(MOVE_PRIO_GRENADE, (int)(enemy->x - 100));
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
					mv.propose(MOVE_PRIO_GRENADE, (int)(enemy->x + 100));
				else
					mv.propose(MOVE_PRIO_GRENADE, (int)(enemy->x - 100));
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
					mv.propose(MOVE_PRIO_SEPARATION, (int)(enemy->x + 50));
				else
					mv.propose(MOVE_PRIO_SEPARATION, (int)(enemy->x - 50));
			}
		}
	}

	// safe mode: head for cover, or just away from Bob when there is none
	if (ai.safeMode)
	{
		if (ai.hasCover)
			mv.propose(MOVE_PRIO_COVER, ai.coverX);
		else
			mv.propose(MOVE_PRIO_COVER, (int)((player.x < enemy->x) ? (enemy->x + 100) : (enemy->x - 100)));
	}

	// apply the winning destination (nothing proposed = keep the current one)
	if (mv.has)
		enemy->tx = mv.tx;

	// the cover (or the flight from Bob) also sets the height it flies at
	if (ai.safeMode && mv.has && (mv.prio == MOVE_PRIO_COVER))
		enemy->ty = ai.hasCover ? ai.coverY : (int)enemy->y;

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

	// staggered by a hit on the bare chassis: hold still, then a short grace period
	if (ai.stagger > 0)
	{
		if (--ai.stagger == 0)
			ai.staggerImmune = DROID_STAGGER_IMMUNITY;

		enemy->dx = 0;

		if ((enemy->flags & ENT_FLIES) || (enemy->flags & ENT_SWIMS))
			enemy->dy = 0;
	}
	else if (ai.staggerImmune > 0)
	{
		ai.staggerImmune--;
	}

	// winding up a shot, firing a burst or a beam: hold still
	if ((ai.telegraph > 0) || (ai.burstLeft > 0) || (ai.beamTimer > 0))
	{
		enemy->dx = 0;

		if ((enemy->flags & ENT_FLIES) || (enemy->flags & ENT_SWIMS))
			enemy->dy = 0;
	}

	// safe mode: it crawls, moving on only one frame in DROID_SAFE_MOVE_PERIOD
	if (ai.safeMode && ((aiFrame % DROID_SAFE_MOVE_PERIOD) != 0))
	{
		if (!enemy->falling)
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

// ---- Utility droid: shield feedback ----
// While the shield holds, a cyan bubble (soft fill + bright rim) pulses around the droid,
// drawn on top of the sprite and dimmer as the shield drops. Once it is broken the bubble
// is gone and the droid sheds debris that falls and bounces.
static SDL_Surface *droidBubble = NULL;
static int droidBubbleW = 0;
static int droidBubbleH = 0;
static const int DROID_BUBBLE_PAD = 4; // pixels the bubble extends beyond the droid on every side
static const float DROID_BUBBLE_OPACITY = 0.5f; // overall strength of the bubble (1.0 = full, lower = more subtle)

static void createDroidBubble(int w, int h)
{
	if ((droidBubble != NULL) && (droidBubbleW == w) && (droidBubbleH == h))
		return;

	if (droidBubble != NULL)
	{
		SDL_FreeSurface(droidBubble);
		droidBubble = NULL;
	}

	droidBubbleW = w;
	droidBubbleH = h;

	int bw = w + (DROID_BUBBLE_PAD * 2);
	int bh = h + (DROID_BUBBLE_PAD * 2);

	droidBubble = SDL_CreateRGBSurface(0, bw, bh, 32, 0xff0000, 0xff00, 0xff, 0xff000000);

	if (droidBubble == NULL)
		return;

	SDL_SetSurfaceBlendMode(droidBubble, SDL_BLENDMODE_BLEND);

	if (SDL_MUSTLOCK(droidBubble))
		SDL_LockSurface(droidBubble);

	for (int py = 0 ; py < bh ; py++)
	{
		Uint32 *row = (Uint32*)(((Uint8*)droidBubble->pixels) + (py * droidBubble->pitch));

		for (int px = 0 ; px < bw ; px++)
		{
			float nx = (px + 0.5f - (bw / 2.0f)) / (bw / 2.0f);
			float ny = (py + 0.5f - (bh / 2.0f)) / (bh / 2.0f);
			float d = sqrtf((nx * nx) + (ny * ny));
			int alpha = 0;

			if (d <= 1.0f)
			{
				// soft translucent fill, a bit stronger towards the edge
				float fill = 18.0f + (30.0f * d * d);
				float rim = 0.0f;

				// bright rim in the outer 15% of the radius
				if (d >= 0.85f)
				{
					rim = 170.0f * sinf(3.14159265f * ((d - 0.85f) / 0.15f));
				}

				alpha = (int)((rim > fill) ? rim : fill);

				if (alpha > 255)
					alpha = 255;
			}

			row[px] = SDL_MapRGBA(droidBubble->format, 0, 229, 255, (Uint8)alpha);
		}
	}

	if (SDL_MUSTLOCK(droidBubble))
		SDL_UnlockSurface(droidBubble);
}

// Pieces that come off the droid: physical particles (gravity, bounce on the terrain)
static void addDroidDebris(Entity *enemy, int count, float speed)
{
	Uint32 cyan = SDL_MapRGB(graphics.screen->format, 0, 229, 255);
	Uint32 steel = SDL_MapRGB(graphics.screen->format, 150, 160, 170);
	Uint32 dark = SDL_MapRGB(graphics.screen->format, 70, 75, 85);

	for (int i = 0 ; i < count ; i++)
	{
		float px = enemy->x + Math::rrand(0, enemy->width);
		float py = enemy->y + Math::rrand(0, enemy->height);
		float dx = (Math::rrand(-15, 15) / 10.0f) * speed;
		float dy = (Math::rrand(-20, 5) / 10.0f) * speed;

		int pick = (int)(Math::prand() % 3);
		Uint32 color = (pick == 0) ? cyan : ((pick == 1) ? steel : dark);

		map.addParticle(px, py, dx, dy, Math::rrand(30, 70), color, NULL, PAR_COLLIDES);
	}
}

static void drawDroidShield(Entity *enemy, int x, int y)
{
	if (enemy->health <= 0)
		return;

	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if ((it == aiState.end()) || (!it->second.utilityDroid))
		return;

	EnemyAIState &st = it->second;

	if (st.shield > 0)
	{
		createDroidBubble(enemy->width, enemy->height);

		if (droidBubble == NULL)
			return;

		int phase = (int)((aiFrame + (unsigned int)(((size_t)enemy) >> 4)) % 60);
		int tri = (phase < 30) ? phase : (60 - phase);
		float frac = (st.maxShield > 0) ? ((float)st.shield / st.maxShield) : 1.0f;
		float pulse = 0.70f + (0.30f * (tri / 30.0f));
		int alpha = (int)(255 * DROID_BUBBLE_OPACITY * pulse * (0.45f + (0.55f * frac)));

		// almost gone: the field flickers
		if ((frac <= 0.4f) && (((aiFrame / 3) % 2) == 0))
			alpha /= 2;

		if (alpha > 255)
			alpha = 255;

		SDL_SetSurfaceAlphaMod(droidBubble, (Uint8)alpha);
		graphics.blit(droidBubble, x - DROID_BUBBLE_PAD, y - DROID_BUBBLE_PAD, graphics.screen, false);
		return;
	}

	// shield down: it keeps losing pieces
	if ((aiFrame % 3) == 0)
		addDroidDebris(enemy, Math::rrand(1, 2), 1.0f);
}

// ---- Eye Droid beam: look ----
// The beam has no sprite. Two soft round dots, a wide dim glow and a thin bright core, are generated here (like the shield
// bubble) and stamped along the line from the droid to where the beam ends, so it works at any angle and any length.
static SDL_Surface *beamGlowDot = NULL;
static SDL_Surface *beamCoreDot = NULL;
static const int BEAM_GLOW_SIZE = 14;      // diameter of the glow dot (pixels)
static const int BEAM_CORE_SIZE = 6;       // diameter of the core dot
static const int BEAM_GLOW_SPACING = 4;    // pixels between two glow dots along the beam (they overlap)
static const int BEAM_CORE_SPACING = 3;    // same for the core
static const int BEAM_GLOW_ALPHA = 70;     // strength of each glow dot (they add up where they overlap)
static const int BEAM_FADE_FRAMES = 6;     // frames the beam takes to appear and to go out

// Round dot: solid up to 'hardness' of the radius, soft towards the edge
static SDL_Surface *createBeamDot(int size, int r, int g, int b, float hardness)
{
	SDL_Surface *dot = SDL_CreateRGBSurface(0, size, size, 32, 0xff0000, 0xff00, 0xff, 0xff000000);

	if (dot == NULL)
		return NULL;

	SDL_SetSurfaceBlendMode(dot, SDL_BLENDMODE_BLEND);

	if (SDL_MUSTLOCK(dot))
		SDL_LockSurface(dot);

	for (int py = 0 ; py < size ; py++)
	{
		Uint32 *row = (Uint32*)(((Uint8*)dot->pixels) + (py * dot->pitch));

		for (int px = 0 ; px < size ; px++)
		{
			float nx = (px + 0.5f - (size / 2.0f)) / (size / 2.0f);
			float ny = (py + 0.5f - (size / 2.0f)) / (size / 2.0f);
			float d = sqrtf((nx * nx) + (ny * ny));
			float a = 0.0f;

			if (d <= hardness)
				a = 1.0f;
			else if (d < 1.0f)
				a = (1.0f - d) / (1.0f - hardness);

			row[px] = SDL_MapRGBA(dot->format, r, g, b, (Uint8)(255 * a * a));
		}
	}

	if (SDL_MUSTLOCK(dot))
		SDL_UnlockSurface(dot);

	return dot;
}

// Stamps one of the dots along the beam
static void stampBeamDots(SDL_Surface *dot, int size, int spacing, int alpha, float sx, float sy, float dirX, float dirY, float len)
{
	SDL_SetSurfaceAlphaMod(dot, (Uint8)alpha);

	for (float d = 0.0f ; d <= len ; d += spacing)
	{
		int px = (int)(sx + (dirX * d));
		int py = (int)(sy + (dirY * d));

		if ((px < -size) || (py < -size) || (px > graphics.screen->w + size) || (py > graphics.screen->h + size))
			continue;

		graphics.blit(dot, px - (size / 2), py - (size / 2), graphics.screen, false);
	}
}

static void drawDroidBeam(Entity *enemy, int x, int y)
{
	if (enemy->health <= 0)
		return;

	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if ((it == aiState.end()) || (it->second.beamTimer <= 0) || (it->second.beamLen < 1.0f))
		return;

	EnemyAIState &st = it->second;

	if (beamGlowDot == NULL)
		beamGlowDot = createBeamDot(BEAM_GLOW_SIZE, 255, 40, 40, 0.0f);

	if (beamCoreDot == NULL)
		beamCoreDot = createBeamDot(BEAM_CORE_SIZE, 255, 225, 215, 0.5f);

	if ((beamGlowDot == NULL) || (beamCoreDot == NULL))
		return;

	float sx = x + (enemy->width / 2.0f);
	float sy = y + (enemy->height / 2.0f);
	float dirX = cosf(st.beamAngle);
	float dirY = sinf(st.beamAngle);

	// it fades in when it starts and out when it ends
	int elapsed = st.beamTotal - st.beamTimer;
	int edge = std::min(elapsed + 1, st.beamTimer + 1);
	float fade = std::min(1.0f, (float)edge / BEAM_FADE_FRAMES);

	// pulse, like the shield bubble
	int phase = (int)((aiFrame + (unsigned int)(((size_t)enemy) >> 4)) % 20);
	int tri = (phase < 10) ? phase : (20 - phase);
	float pulse = 0.75f + (0.25f * (tri / 10.0f));

	stampBeamDots(beamGlowDot, BEAM_GLOW_SIZE, BEAM_GLOW_SPACING, (int)(BEAM_GLOW_ALPHA * fade * pulse), sx, sy, dirX, dirY, st.beamLen);
	stampBeamDots(beamCoreDot, BEAM_CORE_SIZE, BEAM_CORE_SPACING, (int)(255 * fade), sx, sy, dirX, dirY, st.beamLen);
}

// True for a utility droid whose shield is already down
static bool isDroidShieldDown(Entity *enemy)
{
	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	return ((it != aiState.end()) && (it->second.utilityDroid) && (it->second.shield <= 0));
}

// Direct hit on the bare chassis: a micro-stun (doAI holds it still while it lasts)
static void staggerDroid(Entity *enemy)
{
	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if ((it == aiState.end()) || (!it->second.utilityDroid))
		return;

	if ((it->second.stagger > 0) || (it->second.staggerImmune > 0))
		return;

	it->second.stagger = DROID_STAGGER_FRAMES;
}

// The utility droid's shield soaks damage first. Whatever it can't absorb
// (a hit bigger than the shield left) carries over to health. Returns that remainder.
static int soakShieldDamage(Entity *enemy, int damage)
{
	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if ((it == aiState.end()) || (!it->second.utilityDroid) || (it->second.shield <= 0) || (damage <= 0))
		return damage;

	EnemyAIState &ai = it->second;

	if (damage < ai.shield)
	{
		ai.shield -= damage;
		return 0;
	}

	int overflow = damage - ai.shield;
	ai.shield = 0;
	enterDroidSafeMode(ai); // 10 s until the shield is back

	addDroidDebris(enemy, 18, 3.0f); // the shield shatters

	return overflow;
}

// Explosion damage to an enemy (called from explosions.cpp). Goes through the shield like
// bullets do; an explosion caused by the player also makes the utility droid want to dash.
int soakExplosionDamage(Entity *enemy, int damage, bool fromPlayer)
{
	int remaining = soakShieldDamage(enemy, damage);

	if (fromPlayer)
	{
		std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

		if ((it != aiState.end()) && (it->second.utilityDroid))
			it->second.dashReact = true;
	}

	return remaining;
}

// Gravitational deflection (called from doBullets() in bullets.cpp, once per frame, before the
// grenade moves). When one of the player's grenades gets inside the repulsor field of a
// utility droid that still has DROID_DEFLECT_COST shield points, the droid takes no damage and
// the grenade bounces off the field like a billiard ball: it is reflected about the line from the
// droid's center to the grenade, so it leaves at the mirror angle of how it arrived. Costs 2
// shield points. With fewer points the field is off and the grenade hits (and explodes) as usual.
// Returns true when the grenade was deflected.
bool deflectGrenadeAtDroids(Entity *bullet)
{
	if ((bullet->owner != &player) || (bullet->id != WP_GRENADES) || (bullet->health < 1))
		return false;

	float bx = bullet->x + (bullet->width / 2.0f);
	float by = bullet->y + (bullet->height / 2.0f);

	Entity *enemy = (Entity*)map.enemyList.getHead();

	while (enemy->next != NULL)
	{
		enemy = (Entity*)enemy->next;

		if (enemy->health <= 0)
			continue;

		std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

		if ((it == aiState.end()) || (!it->second.utilityDroid) || (it->second.shield < DROID_DEFLECT_COST))
			continue;

		float cx = enemy->x + (enemy->width / 2.0f);
		float cy = enemy->y + (enemy->height / 2.0f);
		float radius = (((enemy->width > enemy->height) ? enemy->width : enemy->height) / 2.0f) + DROID_DEFLECT_MARGIN;

		float nx = bx - cx;
		float ny = by - cy;
		float dist = sqrtf((nx * nx) + (ny * ny));

		if ((dist >= radius) || (dist < 0.01f))
			continue;

		nx /= dist;
		ny /= dist;

		float into = (bullet->dx * nx) + (bullet->dy * ny);

		if (into >= 0.0f) // already moving away from the droid: leave it alone
			continue;

		// mirror the velocity about the normal and push the grenade back to the field's edge
		bullet->dx -= 2.0f * into * nx;
		bullet->dy -= 2.0f * into * ny;
		bullet->x += nx * ((radius + 2.0f) - dist);
		bullet->y += ny * ((radius + 2.0f) - dist);

		if (bullet->dx != 0.0f)
			bullet->face = (bullet->dx < 0.0f) ? 1 : 0;

		EnemyAIState &ai = it->second;

		ai.shield -= DROID_DEFLECT_COST;

		// the point of contact flashes
		for (int i = 0 ; i < 10 ; i++)
		{
			float pdx = Math::rrand(-20, 20); pdx /= 10;
			float pdy = Math::rrand(-20, 20); pdy /= 10;
			map.addParticle(cx + (nx * (radius - 6.0f)), cy + (ny * (radius - 6.0f)), pdx, pdy, Math::rrand(10, 25), graphics.cyan, NULL, 0);
		}

		audio.playSound(SND_CLANG, CH_ANY, enemy->x);

		if (ai.shield <= 0)
		{
			ai.shield = 0;
			addDroidDebris(enemy, 18, 3.0f); // the field collapses like a broken shield
			enterDroidSafeMode(ai);
		}

		return true;
	}

	return false;
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
						hitState.dashReact = true;

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
						bool droidShieldWasDown = isDroidShieldDown(enemy);

						enemy->health -= soakShieldDamage(enemy, bullet->damage);

						if (droidShieldWasDown && (bullet->damage > 0))
							staggerDroid(enemy);
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

// Rank of an enemy for its drops; does not create AI state for an enemy that has none
static int getDropRank(Entity *enemy)
{
	std::map<Entity*, EnemyAIState>::iterator it = aiState.find(enemy);

	if (it == aiState.end())
		return RANK_SOLDIER;

	return it->second.rank;
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
				
				// Eye Droid beam: turns, finds its end and hurts for as long as it is on
				std::map<Entity*, EnemyAIState>::iterator beamIt = aiState.find(enemy);

				if ((beamIt != aiState.end()) && (beamIt->second.beamTimer > 0))
					updateDroidBeam(enemy, beamIt->second);

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
					drawDroidShield(enemy, x, y);
					drawDroidBeam(enemy, x, y);
					
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
								dropRandomItemsByRank((int)enemy->x, (int)enemy->y, getDropRank(enemy));
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
									dropRandomItemsByRank((int)enemy->x, (int)enemy->y, getDropRank(enemy));
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
