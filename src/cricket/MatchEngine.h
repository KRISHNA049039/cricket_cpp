#pragma once
#include "CricketTypes.h"
#include "TeamData.h"
#include <random>
#include <functional>

class MatchEngine {
public:
    MatchEngine();

    void SetupMatch(const Team& home, const Team& away,
                    const Stadium& stadium, MatchFormat format);
    void SimulateMatch();

    // Callbacks for UI
    using CommentaryCallback = std::function<void(const std::string&)>;
    void SetCommentaryCallback(CommentaryCallback cb) { commentary = cb; }

private:
    // Match state
    MatchFormat format;
    Team teams[2];
    Stadium stadium;
    Weather weather;
    PitchCondition pitch;
    BallCondition ball[2]; // one per end in ODIs
    int currentBallEnd = 0; // which ball is in use (ODI: alternates)

    InningsScore innings[4]; // up to 4 innings in Test
    int currentInnings = 0;
    int battingTeamIdx = 0;
    int bowlingTeamIdx = 1;
    int totalInnings; // 2 for ODI, 4 for Test

    int maxOvers; // 50 for ODI, 90 per day for Test (simplified: 90 per innings)
    int target = -1;

    // Current players
    int strikerIdx = 0;
    int nonStrikerIdx = 1;
    int currentBowlerIdx = 7;
    int nextBatIdx = 2;
    int lastBowlerIdx = -1;
    std::vector<int> bowlerRotation;

    // RNG
    std::mt19937 rng;

    // Methods
    void InitWeather();
    void InitPitch();
    void SimulateInnings();
    void SimulateBall();
    void SimulateOver();

    // Ball physics simulation
    struct DeliveryResult {
        int runs = 0;
        bool isWicket = false;
        bool isWide = false;
        bool isNoBall = false;
        bool isBye = false;
        bool isFour = false;
        bool isSix = false;
        DismissalType dismissal = DismissalType::NotOut;
        ShotType shot = ShotType::Defensive;
        std::string description;
    };

    DeliveryResult SimulateDelivery(const PlayerStats& bowler,
                                     const PlayerStats& batsman);

    float CalculateSwing(const PlayerStats& bowler);
    float CalculateSeam(const PlayerStats& bowler);
    float CalculateSpin(const PlayerStats& bowler);
    float CalculateBounce();
    float CalculateBatPower(const PlayerStats& batsman, ShotType shot);
    ShotType ChooseShot(const PlayerStats& batsman, float difficulty);
    DeliveryType ChooseDelivery(const PlayerStats& bowler);
    DismissalType CheckDismissal(const PlayerStats& batsman, float difficulty,
                                  ShotType shot, const PlayerStats& bowler);

    // Ball wear for ODI (2 balls from both ends)
    void HandleBallWear();
    void HandleNewBallTest();

    // Helpers
    void SelectBowler();
    void SwapStrike();
    void RecordWicket(DismissalType type, const std::string& bowlerName,
                      const std::string& fielderName);
    bool IsInningsOver() const;
    float GetMatchSituation() const; // 0=calm, 1=tense
    InningsPhase GetPhase() const;
    void PrintScorecard() const;
    void PrintInningsScorecard(int inningsIdx) const;
    void Say(const std::string& msg) const;

    float RandFloat(float min = 0.0f, float max = 1.0f);
    int RandInt(int min, int max);

    CommentaryCallback commentary;
};
