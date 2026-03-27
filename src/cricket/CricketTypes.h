#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class MatchFormat { Test, ODI };
enum class InningsPhase { Early, Middle, Late, Death };

enum class ShotType {
    Defensive, Drive, Cut, Pull, Sweep, LoftedDrive, Flick, UpperCut,
    LeaveAlone, EdgeBehind, PlayAndMiss
};

enum class DismissalType {
    NotOut, Bowled, Caught, LBW, RunOut, Stumped, CaughtBehind, CaughtAndBowled
};

enum class BowlingType { Fast, MediumFast, Medium, OffSpin, LegSpin };
enum class DeliveryType { Good, Short, Full, Yorker, Bouncer, Wide, NoBall };

enum class WeatherType { Sunny, Overcast, Cloudy, Drizzle, Humid };

struct BallCondition {
    float shine = 1.0f;       // 1.0 = new, 0.0 = completely worn
    float seamIntegrity = 1.0f;
    int ballAge = 0;          // deliveries bowled with this ball
    bool isReversing = false;

    void AgeBall() {
        ballAge++;
        shine = std::max(0.0f, shine - 0.003f);
        seamIntegrity = std::max(0.1f, seamIntegrity - 0.001f);
        if (shine < 0.25f && ballAge > 150) isReversing = true;
    }

    void TakeNewBall() {
        shine = 1.0f;
        seamIntegrity = 1.0f;
        ballAge = 0;
        isReversing = false;
    }
};

struct PitchCondition {
    float hardness;      // 0-1, affects bounce
    float grassCoverage; // 0-1, affects seam movement
    float moisture;      // 0-1, affects grip
    float deterioration; // increases over match, affects spin/variable bounce

    void Deteriorate(float amount) {
        deterioration = std::min(1.0f, deterioration + amount);
        moisture = std::max(0.0f, moisture - amount * 0.3f);
    }
};

struct Weather {
    WeatherType type;
    float humidity;       // 0-1
    float windSpeed;      // km/h
    float temperature;    // celsius
    std::string description;

    float GetSwingFactor() const {
        float factor = 1.0f;
        if (type == WeatherType::Overcast || type == WeatherType::Cloudy)
            factor += 0.4f;
        if (humidity > 0.7f) factor += 0.3f * ((humidity - 0.7f) / 0.3f);
        return factor;
    }

    float GetSpinFactor() const {
        float factor = 1.0f;
        if (type == WeatherType::Sunny) factor += 0.2f;
        if (humidity < 0.4f) factor += 0.15f;
        return factor;
    }
};

struct Stadium {
    std::string name;
    std::string city;
    std::string country;
    int capacity;
    float boundarySize;     // meters, avg boundary distance
    float pitchHardness;
    float grassCoverage;
    float baseMoisture;
    WeatherType typicalWeather;
    float typicalHumidity;
    float typicalWindSpeed;
    float typicalTemp;
};

struct PlayerStats {
    std::string name;
    int battingSkill;     // 0-100
    int bowlingSkill;     // 0-100
    int fieldingSkill;    // 0-100
    BowlingType bowlType;
    bool isWicketKeeper;
    // batting style weights
    float powerHitting;   // 0-1
    float technique;      // 0-1
    float temperament;    // 0-1, patience
    float runningSpeed;   // 0-1
};

struct BatsmanInnings {
    std::string name;
    int runs = 0;
    int ballsFaced = 0;
    int fours = 0;
    int sixes = 0;
    DismissalType howOut = DismissalType::NotOut;
    std::string dismissedBy;
    std::string caughtBy;
    bool isOut = false;

    float GetStrikeRate() const {
        return ballsFaced > 0 ? (runs * 100.0f / ballsFaced) : 0.0f;
    }
};

struct BowlerFigures {
    std::string name;
    int overs = 0;
    int ballsInCurrentOver = 0;
    int maidens = 0;
    int runsConceded = 0;
    int wickets = 0;

    std::string GetFigures() const;
    float GetEconomy() const {
        float totalOvers = overs + ballsInCurrentOver / 6.0f;
        return totalOvers > 0 ? runsConceded / totalOvers : 0.0f;
    }
};

struct InningsScore {
    int totalRuns = 0;
    int wickets = 0;
    int oversCompleted = 0;
    int ballsInOver = 0;
    int extras = 0;
    int wides = 0;
    int noBalls = 0;
    int byes = 0;
    int legByes = 0;
    std::vector<BatsmanInnings> battingCard;
    std::vector<BowlerFigures> bowlingCard;
    std::vector<std::string> fallOfWickets;

    float GetRunRate() const {
        float totalOvers = oversCompleted + ballsInOver / 6.0f;
        return totalOvers > 0 ? totalRuns / totalOvers : 0.0f;
    }

    std::string GetScoreString() const;
    float GetOversFloat() const {
        return oversCompleted + ballsInOver / 6.0f;
    }
};
