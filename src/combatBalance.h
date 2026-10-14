#ifndef COMBAT_BALANCE_H
#define COMBAT_BALANCE_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

struct CombatBalance
{
#define COMBAT_PARAMETER(type, name, initial, minimum, maximum) type name;
#include "combatBalance.def"
#undef COMBAT_PARAMETER

    CombatBalance()
    {
#define COMBAT_PARAMETER(type, name, initial, minimum, maximum) name = initial;
#include "combatBalance.def"
#undef COMBAT_PARAMETER
    }

    bool set(const std::string &key, double value, const char *path, int line)
    {
#define COMBAT_PARAMETER(type, name, initial, minimum, maximum) \
        if (key == #name) { \
            if (!(value >= minimum && value <= maximum) || \
                (std::string(#type) != "float" && std::floor(value) != value)) { \
                std::fprintf(stderr, "%s:%d: invalid %s (allowed %g..%g)\n", \
                    path, line, #name, double(minimum), double(maximum)); \
                return false; \
            } \
            name = static_cast<type>(value); \
            return true; \
        }
#include "combatBalance.def"
#undef COMBAT_PARAMETER
        std::fprintf(stderr, "%s:%d: unknown key %s\n", path, line, key.c_str());
        return false;
    }

    bool consistent() const
    {
        return DASH_MIN_RADIUS <= DASH_MAX_RADIUS &&
            SEARCH_MIN_OFFSET <= SEARCH_MAX_OFFSET &&
            SEARCH_WAIT_MIN <= SEARCH_WAIT_MAX &&
            LASER_SWEEP_MIN_ANGLE <= LASER_SWEEP_MAX_ANGLE &&
            LASER_BEAM_MIN_ANGLE <= LASER_BEAM_MAX_ANGLE &&
            DROID_RECHARGE_FRAMES <= DROID_RECHARGE_CAP &&
            VETERAN_MAX_CHANCE + SERGEANT_MAX_CHANCE <= 100 &&
            LASER_BEAM_CLIP_COST <= LASER_CLIP_SIZE;
    }

    bool load(const char *path)
    {
        // Local file intentionally bypasses the pak so tuning never needs a rebuild.
        *this = CombatBalance();
        std::ifstream file(path);
        if (!file)
        {
            std::fprintf(stderr, "%s: unavailable; using default combat balance\n", path);
            return false;
        }

        CombatBalance candidate;
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line))
        {
            ++lineNumber;
            line.erase(line.find('#') == std::string::npos ? line.size() : line.find('#'));
            std::istringstream tokens(line);
            std::string key, number, extra;
            char equals;
            if (!(tokens >> key))
                continue;
            if (!(tokens >> equals >> number) || equals != '=' || (tokens >> extra))
            {
                std::fprintf(stderr, "%s:%d: expected KEY = number\n", path, lineNumber);
                continue;
            }
            char *end;
            double value = std::strtod(number.c_str(), &end);
            if (*end != '\0')
            {
                std::fprintf(stderr, "%s:%d: invalid number\n", path, lineNumber);
                continue;
            }
            candidate.set(key, value, path, lineNumber);
        }
        if (file.bad() || !candidate.consistent())
        {
            std::fprintf(stderr, "%s: inconsistent ranges or read error; using defaults\n", path);
            return false;
        }
        *this = candidate;
        return true;
    }
};

#endif
