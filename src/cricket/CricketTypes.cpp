#include "CricketTypes.h"
#include <sstream>

std::string BowlerFigures::GetFigures() const {
    std::ostringstream ss;
    ss << wickets << "/" << runsConceded << " (" << overs;
    if (ballsInCurrentOver > 0) ss << "." << ballsInCurrentOver;
    ss << " ov)";
    return ss.str();
}

std::string InningsScore::GetScoreString() const {
    std::ostringstream ss;
    ss << totalRuns << "/" << wickets << " (" << oversCompleted;
    if (ballsInOver > 0) ss << "." << ballsInOver;
    ss << " ov)";
    return ss.str();
}
