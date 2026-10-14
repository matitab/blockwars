#include "../src/combatBalance.h"
#include <cassert>

static void fixture(const char *path, const char *text)
{
    std::ofstream file(path);
    assert(file);
    file << text;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    const char *path = argv[1];
    CombatBalance balance, defaults;
    assert(balance.load("combat.cfg"));
#define COMBAT_PARAMETER(type, name, initial, minimum, maximum) assert(balance.name == defaults.name);
#include "../src/combatBalance.def"
#undef COMBAT_PARAMETER

    fixture(path, "# Partial override with CRLF and inline comment\r\nDASH_DODGE_CHANCE = 35 # percent\r\nAWARE_DECAY = 0.25\r\nBIOMECH_KEEPS_DISTANCE = 1\r\n");
    assert(balance.load(path));
    assert(balance.DASH_DODGE_CHANCE == 35);
    assert(balance.AWARE_DECAY == 0.25f);
    assert(balance.BIOMECH_KEEPS_DISTANCE);
    assert(balance.DROID_SHIELD_POINTS == defaults.DROID_SHIELD_POINTS);

    fixture(path, "DASH_DODGE_CHANCE = 101\nDROID_SHIELD_POINTS = -1\nDROID_MAX_DASHES = 1.5\nAWARE_DECAY = nan\nVISION_RANGE = inf\nLASER_BEAM_TICK = 0\nBIOMECH_KEEPS_DISTANCE = 2\nUNKNOWN = 3\nDASH_COOLDOWN = garbage\nDASH_CLOSE_CHANCE = 3 trailing\nmalformed\nDASH_HIT_CHANCE = 25\n");
    assert(balance.load(path));
    assert(balance.DASH_HIT_CHANCE == 25);
    assert(balance.DASH_DODGE_CHANCE == defaults.DASH_DODGE_CHANCE);
    assert(balance.DROID_SHIELD_POINTS == defaults.DROID_SHIELD_POINTS);
    assert(balance.DROID_MAX_DASHES == defaults.DROID_MAX_DASHES);
    assert(balance.AWARE_DECAY == defaults.AWARE_DECAY);
    assert(balance.VISION_RANGE == defaults.VISION_RANGE);
    assert(balance.LASER_BEAM_TICK == defaults.LASER_BEAM_TICK);
    assert(!balance.BIOMECH_KEEPS_DISTANCE);

    fixture(path, "DASH_MIN_RADIUS = 200\nDASH_MAX_RADIUS = 100\nDASH_DODGE_CHANCE = 35\n");
    assert(!balance.load(path));
    assert(balance.DASH_DODGE_CHANCE == defaults.DASH_DODGE_CHANCE);
    assert(std::remove(path) == 0);
    assert(!balance.load(path));
    assert(balance.DASH_DODGE_CHANCE == defaults.DASH_DODGE_CHANCE);
    std::puts("Combat balance tests passed");
}
