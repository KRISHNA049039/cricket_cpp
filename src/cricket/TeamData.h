#pragma once
#include "CricketTypes.h"
#include <vector>

struct Team {
    std::string name;
    std::string shortName;
    std::vector<PlayerStats> players;
};

namespace TeamDatabase {
    Team CreateEngland();
    Team CreateAustralia();
    std::vector<Stadium> GetStadiums();
    Stadium GetStadium(const std::string& name);
}
