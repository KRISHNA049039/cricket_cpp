#include "TeamData.h"

namespace TeamDatabase {

Team CreateEngland() {
    Team eng;
    eng.name = "England";
    eng.shortName = "ENG";
    eng.players = {
        // Top order batsmen - high technique, good temperament
        {"Z Crawley",    78, 15, 65, BowlingType::Medium, false, 0.65f, 0.70f, 0.55f, 0.75f},
        {"B Duckett",    80, 10, 60, BowlingType::Medium, false, 0.72f, 0.68f, 0.50f, 0.70f},
        {"J Root",       92, 20, 75, BowlingType::OffSpin, false, 0.55f, 0.95f, 0.92f, 0.65f},
        {"H Brook",      85, 12, 70, BowlingType::Medium, false, 0.80f, 0.72f, 0.60f, 0.72f},
        {"J Bairstow",   76, 10, 65, BowlingType::Medium, false, 0.75f, 0.60f, 0.50f, 0.68f},
        // All-rounder
        {"B Stokes",     82, 72, 78, BowlingType::MediumFast, false, 0.78f, 0.70f, 0.65f, 0.60f},
        // Wicketkeeper
        {"J Foakes",     68, 5, 85, BowlingType::Medium, true, 0.35f, 0.75f, 0.80f, 0.65f},
        // Bowlers
        {"M Wood",       25, 85, 55, BowlingType::Fast, false, 0.30f, 0.20f, 0.40f, 0.70f},
        {"J Anderson",   15, 90, 50, BowlingType::MediumFast, false, 0.10f, 0.15f, 0.50f, 0.45f},
        {"S Broad",      20, 86, 60, BowlingType::MediumFast, false, 0.25f, 0.18f, 0.45f, 0.50f},
        {"J Leach",      18, 78, 55, BowlingType::OffSpin, false, 0.15f, 0.20f, 0.55f, 0.50f},
    };
    return eng;
}

Team CreateAustralia() {
    Team aus;
    aus.name = "Australia";
    aus.shortName = "AUS";
    aus.players = {
        // Top order - aggressive Aussie style
        {"U Khawaja",    84, 10, 70, BowlingType::Medium, false, 0.45f, 0.88f, 0.90f, 0.55f},
        {"D Warner",     82, 15, 65, BowlingType::Medium, false, 0.85f, 0.60f, 0.45f, 0.80f},
        {"M Labuschagne",88, 25, 72, BowlingType::LegSpin, false, 0.50f, 0.90f, 0.88f, 0.65f},
        {"S Smith",      93, 18, 75, BowlingType::LegSpin, false, 0.48f, 0.96f, 0.95f, 0.60f},
        {"T Head",       79, 15, 68, BowlingType::OffSpin, false, 0.72f, 0.65f, 0.55f, 0.70f},
        // All-rounder
        {"C Green",      72, 70, 80, BowlingType::MediumFast, false, 0.65f, 0.68f, 0.60f, 0.78f},
        // Wicketkeeper
        {"A Carey",      70, 5, 82, BowlingType::Medium, true, 0.60f, 0.65f, 0.55f, 0.72f},
        // Bowlers
        {"P Cummins",    22, 92, 65, BowlingType::Fast, false, 0.20f, 0.15f, 0.50f, 0.65f},
        {"M Starc",      30, 88, 55, BowlingType::Fast, false, 0.35f, 0.22f, 0.40f, 0.60f},
        {"J Hazlewood",  15, 87, 60, BowlingType::MediumFast, false, 0.10f, 0.12f, 0.55f, 0.55f},
        {"N Lyon",       20, 82, 58, BowlingType::OffSpin, false, 0.18f, 0.20f, 0.60f, 0.50f},
    };
    return aus;
}

std::vector<Stadium> GetStadiums() {
    return {
        // England stadiums
        {"Lord's", "London", "England", 30000, 68.0f,
         0.55f, 0.50f, 0.55f, WeatherType::Overcast, 0.72f, 12.0f, 18.0f},
        {"The Oval", "London", "England", 25500, 65.0f,
         0.60f, 0.35f, 0.45f, WeatherType::Cloudy, 0.68f, 10.0f, 19.0f},
        {"Headingley", "Leeds", "England", 18350, 64.0f,
         0.50f, 0.65f, 0.60f, WeatherType::Overcast, 0.78f, 15.0f, 16.0f},
        {"Edgbaston", "Birmingham", "England", 25000, 66.0f,
         0.58f, 0.45f, 0.50f, WeatherType::Cloudy, 0.70f, 11.0f, 17.0f},
        // Australia stadiums
        {"MCG", "Melbourne", "Australia", 100024, 75.0f,
         0.72f, 0.25f, 0.25f, WeatherType::Sunny, 0.45f, 8.0f, 28.0f},
        {"SCG", "Sydney", "Australia", 48000, 70.0f,
         0.65f, 0.30f, 0.30f, WeatherType::Sunny, 0.50f, 10.0f, 26.0f},
        {"The Gabba", "Brisbane", "Australia", 42000, 72.0f,
         0.75f, 0.35f, 0.35f, WeatherType::Humid, 0.65f, 7.0f, 30.0f},
        {"Adelaide Oval", "Adelaide", "Australia", 53583, 71.0f,
         0.68f, 0.28f, 0.28f, WeatherType::Sunny, 0.40f, 12.0f, 27.0f},
    };
}

Stadium GetStadium(const std::string& name) {
    auto stadiums = GetStadiums();
    for (auto& s : stadiums) {
        if (s.name == name) return s;
    }
    return stadiums[0]; // default Lord's
}

} // namespace TeamDatabase
