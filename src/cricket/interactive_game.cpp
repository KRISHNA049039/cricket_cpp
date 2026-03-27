#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define NOMINMAX

#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <random>
#include <chrono>
#include <algorithm>
#include "CricketTypes.h"
#include "TeamData.h"

// ============================================================
//  CONSTANTS
// ============================================================
static const int W = 1200, H = 800;
static const int FX = 370, FY = 380;   // field centre
static const int FR = 300;              // field radius
static const float PI = 3.14159265f;

// ============================================================
//  ENUMS / GAME PHASE
// ============================================================
enum class Phase {
    Menu,           // choose format / teams / stadium
    TossResult,
    BattingReady,   // waiting for bowler to run in (press Space)
    BallInAir,      // ball travelling to batsman
    ShotSelect,     // pick your shot (timed window)
    ShotResult,     // show result, animate ball to field
    BowlingReady,   // you are bowling: pick delivery
    BowlAim,        // aim the delivery
    BowlResult,     // AI bats, show result
    InningsBreak,
    MatchOver
};

// ============================================================
//  SHOT / DELIVERY MAPS  (EA Cricket 07 style keys)
// ============================================================
struct ShotDef {
    int key;                // virtual key
    const wchar_t* name;
    float angle;            // direction on field (radians, 0=straight)
    float powerBase;        // base power multiplier
    float riskFactor;       // dismissal risk multiplier
    bool isLofted;
};

static ShotDef g_shots[] = {
    { 'W', L"Straight Drive",   0.0f,       0.85f, 1.0f, false },
    { 'E', L"Lofted Drive",     0.0f,       0.95f, 2.2f, true  },
    { 'A', L"Cover Drive",      0.45f,      0.82f, 1.0f, false },
    { 'Q', L"Square Cut",       1.2f,       0.80f, 1.3f, false },
    { 'D', L"Flick",           -0.45f,      0.78f, 1.0f, false },
    { 'Z', L"Sweep",           -1.0f,       0.75f, 1.5f, false },
    { 'X', L"Pull Shot",       -0.8f,       0.88f, 1.6f, false },
    { 'C', L"Upper Cut",        1.4f,       0.90f, 2.0f, true  },
    { 'S', L"Defensive Block",  0.0f,       0.20f, 0.3f, false },
    { VK_SPACE, L"Leave",       0.0f,       0.0f,  0.1f, false },
};
static const int NUM_SHOTS = sizeof(g_shots)/sizeof(g_shots[0]);

struct DeliveryDef {
    int key;
    const wchar_t* name;
    float speedFactor;
    float swingFactor;
    float difficultyAdd;
};

static DeliveryDef g_deliveries[] = {
    { '1', L"Good Length",  1.0f, 1.0f, 0.0f  },
    { '2', L"Short Ball",  1.1f, 0.7f, -0.03f },
    { '3', L"Full Ball",   0.9f, 1.2f, 0.02f  },
    { '4', L"Yorker",      0.85f,0.8f, 0.12f  },
    { '5', L"Bouncer",     1.2f, 0.5f, 0.08f  },
    { '6', L"Slower Ball", 0.6f, 0.4f, 0.05f  },
};
static const int NUM_DELIVERIES = sizeof(g_deliveries)/sizeof(g_deliveries[0]);

// ============================================================
//  GAME STATE
// ============================================================
struct GameState {
    // RNG
    std::mt19937 rng;

    // Match config
    MatchFormat format = MatchFormat::ODI;
    Team teams[2];
    Stadium stadium;
    Weather weather;
    PitchCondition pitch;
    BallCondition ballCond;

    // Match progress
    Phase phase = Phase::Menu;
    int currentInnings = 0;   // 0 or 1 for ODI
    int battingTeamIdx = 0;
    int bowlingTeamIdx = 1;
    bool userBatting = true;  // user controls batting team (team 0)

    // Innings state
    InningsScore score;
    int strikerIdx = 0;
    int nonStrikerIdx = 1;
    int nextBatIdx = 2;
    int currentBowlerIdx = 7;
    int lastBowlerIdx = -1;
    int maxOvers = 50;
    int target = -1;

    // Timing
    float shotTimerMax = 1.2f;  // seconds to pick shot
    float shotTimer = 0.0f;
    bool shotTimerActive = false;
    int selectedShotIdx = -1;

    // Ball animation
    float ballAnimT = 0.0f;
    float ballStartX, ballStartY;
    float ballEndX, ballEndY;
    float ballMidX, ballMidY; // arc control point
    bool ballAnimActive = false;

    // Last delivery result
    int lastRuns = 0;
    bool lastIsFour = false;
    bool lastIsSix = false;
    bool lastIsWicket = false;
    bool lastIsWide = false;
    bool lastIsNoBall = false;
    DismissalType lastDismissal = DismissalType::NotOut;
    std::wstring lastDesc;
    std::wstring lastShotName;

    // Bowling selection
    int selectedDeliveryIdx = -1;

    // Commentary
    std::vector<std::wstring> commentary;

    // Wagon wheel (all scoring shots this innings)
    struct WagonDot { float angle; float dist; int runs; bool isWicket; };
    std::vector<WagonDot> wagonWheel;

    // This over balls
    struct OverBall { int runs; bool isWicket; bool isExtra; };
    std::vector<OverBall> thisOver;

    // Menu state
    int menuStep = 0;  // 0=format, 1=stadium
    int menuChoice = 0;
    std::vector<Stadium> allStadiums;

    // Pause between phases
    int pauseFrames = 0;

    // Helpers
    float RandF(float lo = 0.f, float hi = 1.f) {
        std::uniform_real_distribution<float> d(lo, hi);
        return d(rng);
    }
    int RandI(int lo, int hi) {
        std::uniform_int_distribution<int> d(lo, hi);
        return d(rng);
    }

    void AddComment(const std::wstring& s) {
        commentary.push_back(s);
        if (commentary.size() > 16) commentary.erase(commentary.begin());
    }

    void SwapStrike() { std::swap(strikerIdx, nonStrikerIdx); }

    float GetOvers() const {
        return score.oversCompleted + score.ballsInOver / 6.0f;
    }

    bool IsInningsOver() const {
        if (score.wickets >= 10) return true;
        if (score.oversCompleted >= maxOvers) return true;
        if (target > 0 && score.totalRuns >= target) return true;
        return false;
    }

    PlayerStats& Striker() { return teams[battingTeamIdx].players[strikerIdx]; }
    PlayerStats& NonStriker() { return teams[battingTeamIdx].players[nonStrikerIdx]; }
    PlayerStats& Bowler() { return teams[bowlingTeamIdx].players[currentBowlerIdx]; }

    BatsmanInnings& StrikerInnings() { return score.battingCard[strikerIdx]; }
    BatsmanInnings& NonStrikerInnings() { return score.battingCard[nonStrikerIdx]; }
};

static GameState G;
static HWND g_hwnd = nullptr;
static HDC g_memDC = nullptr;
static HBITMAP g_memBmp = nullptr;
static LARGE_INTEGER g_freq, g_lastTime;
static bool g_keys[256] = {};
static bool g_keysJust[256] = {};

// ============================================================
//  DRAWING HELPERS
// ============================================================
std::wstring ToW(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), 0, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

void FillCirc(HDC dc, int cx, int cy, int r, COLORREF c) {
    HBRUSH b = CreateSolidBrush(c);
    HPEN p = CreatePen(PS_SOLID, 1, c);
    auto ob = SelectObject(dc, b); auto op = SelectObject(dc, p);
    Ellipse(dc, cx-r, cy-r, cx+r, cy+r);
    SelectObject(dc, ob); SelectObject(dc, op);
    DeleteObject(b); DeleteObject(p);
}

void DrawTxt(HDC dc, int x, int y, const std::wstring& t, COLORREF c,
             int sz = 14, bool bold = false, const wchar_t* face = L"Consolas") {
    HFONT f = CreateFontW(sz,0,0,0, bold?FW_BOLD:FW_NORMAL,0,0,0,
        DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_SWISS, face);
    auto of = SelectObject(dc, f);
    SetTextColor(dc, c); SetBkMode(dc, TRANSPARENT);
    TextOutW(dc, x, y, t.c_str(), (int)t.size());
    SelectObject(dc, of); DeleteObject(f);
}

void FillBox(HDC dc, int x, int y, int w, int h, COLORREF c) {
    RECT r = {x, y, x+w, y+h};
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

void DrawLine(HDC dc, int x1, int y1, int x2, int y2, COLORREF c, int width = 1) {
    HPEN p = CreatePen(PS_SOLID, width, c);
    auto op = SelectObject(dc, p);
    MoveToEx(dc, x1, y1, nullptr); LineTo(dc, x2, y2);
    SelectObject(dc, op); DeleteObject(p);
}

// ============================================================
//  INIT MATCH
// ============================================================
void InitInnings() {
    G.score = InningsScore{};
    G.strikerIdx = 0;
    G.nonStrikerIdx = 1;
    G.nextBatIdx = 2;
    G.lastBowlerIdx = -1;
    G.ballCond.TakeNewBall();
    G.thisOver.clear();
    G.wagonWheel.clear();
    G.selectedShotIdx = -1;
    G.selectedDeliveryIdx = -1;

    auto& batTeam = G.teams[G.battingTeamIdx];
    for (auto& p : batTeam.players) {
        BatsmanInnings bi; bi.name = p.name;
        G.score.battingCard.push_back(bi);
    }
    auto& bowlTeam = G.teams[G.bowlingTeamIdx];
    for (auto& p : bowlTeam.players) {
        if (p.bowlingSkill >= 40) {
            BowlerFigures bf; bf.name = p.name;
            G.score.bowlingCard.push_back(bf);
        }
    }

    // Pick first bowler
    for (int i = 0; i < (int)bowlTeam.players.size(); i++) {
        if (bowlTeam.players[i].bowlingSkill >= 70) {
            G.currentBowlerIdx = i; break;
        }
    }
}

void StartMatch() {
    G.rng.seed((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());
    G.teams[0] = TeamDatabase::CreateEngland();
    G.teams[1] = TeamDatabase::CreateAustralia();
    G.maxOvers = (G.format == MatchFormat::Test) ? 90 : 50;
    G.target = -1;
    G.currentInnings = 0;

    // Weather from stadium
    G.weather.type = G.stadium.typicalWeather;
    G.weather.humidity = G.stadium.typicalHumidity + G.RandF(-0.08f, 0.08f);
    G.weather.humidity = std::clamp(G.weather.humidity, 0.f, 1.f);
    G.weather.windSpeed = G.stadium.typicalWindSpeed + G.RandF(-2, 2);
    G.weather.temperature = G.stadium.typicalTemp + G.RandF(-2, 2);

    G.pitch.hardness = G.stadium.pitchHardness;
    G.pitch.grassCoverage = G.stadium.grassCoverage;
    G.pitch.moisture = G.stadium.baseMoisture;
    G.pitch.deterioration = 0;

    // Toss
    G.battingTeamIdx = G.RandI(0, 1);
    G.bowlingTeamIdx = 1 - G.battingTeamIdx;
    G.userBatting = (G.battingTeamIdx == 0); // user is always team 0 (England)

    G.AddComment(ToW(G.teams[G.battingTeamIdx].name + " win the toss and bat first."));
    G.commentary.clear();
    G.AddComment(L"Welcome to " + ToW(G.stadium.name) + L", " + ToW(G.stadium.city));
    G.AddComment(ToW(G.teams[0].name) + L" vs " + ToW(G.teams[1].name));
    G.AddComment(ToW(G.teams[G.battingTeamIdx].name) + L" won the toss, elected to bat.");

    InitInnings();
    G.phase = Phase::TossResult;
    G.pauseFrames = 90;
}

// ============================================================
//  BOWLING AI / SELECTION
// ============================================================
void SelectNextBowler() {
    auto& t = G.teams[G.bowlingTeamIdx];
    std::vector<int> eligible;
    for (int i = 0; i < (int)t.players.size(); i++) {
        if (t.players[i].bowlingSkill >= 40 && i != G.lastBowlerIdx) {
            if (G.format == MatchFormat::ODI) {
                int ov = 0;
                for (auto& bf : G.score.bowlingCard)
                    if (bf.name == t.players[i].name) { ov = bf.overs; break; }
                if (ov >= 10) continue;
            }
            eligible.push_back(i);
        }
    }
    if (eligible.empty()) {
        for (int i = 0; i < (int)t.players.size(); i++)
            if (t.players[i].bowlingSkill >= 20 && i != G.lastBowlerIdx)
                eligible.push_back(i);
    }
    if (!eligible.empty())
        G.currentBowlerIdx = eligible[G.RandI(0, (int)eligible.size()-1)];
}

BowlerFigures* FindBowlerFig() {
    auto& name = G.Bowler().name;
    for (auto& bf : G.score.bowlingCard)
        if (bf.name == name) return &bf;
    return nullptr;
}

// ============================================================
//  SIMULATE DELIVERY RESULT
// ============================================================
struct DeliveryOutcome {
    int runs = 0;
    bool isFour = false, isSix = false, isWicket = false;
    bool isWide = false, isNoBall = false;
    DismissalType dismissal = DismissalType::NotOut;
    std::wstring desc;
};

float CalcDifficulty(int deliveryIdx) {
    auto& bowler = G.Bowler();
    float diff = (bowler.bowlingSkill / 100.f) * 0.35f;

    // Swing
    float swing = 0;
    if (bowler.bowlType == BowlingType::Fast || bowler.bowlType == BowlingType::MediumFast)
        swing = (bowler.bowlingSkill / 100.f) * 0.3f * (G.ballCond.shine > 0.5f ? 1.3f : 0.6f);
    swing *= G.weather.GetSwingFactor();
    diff += swing * 0.2f;

    // Seam
    float seam = 0;
    if (bowler.bowlType == BowlingType::Fast || bowler.bowlType == BowlingType::MediumFast)
        seam = (bowler.bowlingSkill / 100.f) * 0.25f * (1 + G.pitch.grassCoverage * 0.5f);
    diff += seam * 0.15f;

    // Spin
    float spin = 0;
    if (bowler.bowlType == BowlingType::OffSpin || bowler.bowlType == BowlingType::LegSpin)
        spin = (bowler.bowlingSkill / 100.f) * 0.4f * (1 + G.pitch.deterioration * 1.5f);
    diff += spin * 0.2f;

    // Delivery type
    if (deliveryIdx >= 0 && deliveryIdx < NUM_DELIVERIES)
        diff += g_deliveries[deliveryIdx].difficultyAdd;

    // New ball bonus
    if (G.score.oversCompleted < 10) diff += 0.08f;
    else if (G.score.oversCompleted < 30) diff += 0.03f;

    if (G.ballCond.isReversing) diff += 0.07f;

    return std::clamp(diff, 0.05f, 0.80f);
}

DeliveryOutcome ResolveUserBatting(int shotIdx, float difficulty) {
    DeliveryOutcome out;
    auto& bat = G.Striker();
    auto& bowler = G.Bowler();

    // Wide / no-ball check
    float wideChance = 0.025f - (bowler.bowlingSkill / 100.f) * 0.015f;
    if (G.RandF() < wideChance) {
        out.isWide = true; out.runs = 1;
        out.desc = L"Wide ball!";
        return out;
    }
    if (G.RandF() < 0.012f) {
        out.isNoBall = true; out.runs = 1;
        out.desc = L"No ball!";
        return out;
    }

    // If no shot selected (timer ran out) → defensive block
    if (shotIdx < 0) shotIdx = 8; // defensive

    auto& shot = g_shots[shotIdx];

    // Dismissal check
    float dismissChance = difficulty * 0.07f;
    dismissChance *= shot.riskFactor;
    dismissChance *= (1.f - bat.battingSkill / 150.f);
    dismissChance *= (1.f - bat.technique * 0.25f);
    auto& bi = G.StrikerInnings();
    if (bi.ballsFaced < 8) dismissChance *= 1.4f;
    dismissChance = std::clamp(dismissChance, 0.004f, 0.14f);

    if (shot.powerBase == 0.0f) {
        // Leave
        out.runs = 0;
        out.desc = ToW(bat.name) + L" leaves the ball alone.";
        // Small chance of bowled if ball comes in
        if (G.RandF() < difficulty * 0.015f) {
            out.isWicket = true;
            out.dismissal = DismissalType::Bowled;
            out.desc = ToW(bat.name) + L" BOWLED! Left one that came back in!";
        }
        return out;
    }

    if (G.RandF() < dismissChance) {
        out.isWicket = true;
        float r = G.RandF();
        bool pace = (bowler.bowlType == BowlingType::Fast || bowler.bowlType == BowlingType::MediumFast);
        if (r < 0.2f) out.dismissal = DismissalType::Bowled;
        else if (r < 0.5f) out.dismissal = DismissalType::Caught;
        else if (r < 0.65f) out.dismissal = pace ? DismissalType::CaughtBehind : DismissalType::Stumped;
        else if (r < 0.8f) out.dismissal = DismissalType::LBW;
        else out.dismissal = DismissalType::CaughtAndBowled;

        switch (out.dismissal) {
            case DismissalType::Bowled: out.desc = ToW(bat.name) + L" BOWLED by " + ToW(bowler.name) + L"!"; break;
            case DismissalType::Caught: out.desc = ToW(bat.name) + L" CAUGHT! b " + ToW(bowler.name); break;
            case DismissalType::CaughtBehind: out.desc = ToW(bat.name) + L" caught behind! b " + ToW(bowler.name); break;
            case DismissalType::LBW: out.desc = ToW(bat.name) + L" LBW! b " + ToW(bowler.name); break;
            case DismissalType::Stumped: out.desc = ToW(bat.name) + L" STUMPED! b " + ToW(bowler.name); break;
            case DismissalType::CaughtAndBowled: out.desc = ToW(bat.name) + L" c&b " + ToW(bowler.name) + L"!"; break;
            default: out.desc = ToW(bat.name) + L" OUT!"; break;
        }
        return out;
    }

    // Runs
    float power = shot.powerBase * (bat.battingSkill / 100.f);
    power *= (0.5f + bat.powerHitting * 0.5f);
    float contact = G.RandF();
    float val = power * contact;

    if (val > 0.88f && shot.isLofted) {
        out.runs = 6; out.isSix = true;
        out.desc = ToW(bat.name) + L" smashes a " + shot.name + L" for SIX!";
    } else if (val > 0.68f) {
        out.runs = 4; out.isFour = true;
        out.desc = ToW(bat.name) + L" plays a " + shot.name + L" to the boundary! FOUR!";
    } else if (val > 0.45f) {
        out.runs = G.RandI(1, 3);
        out.desc = ToW(bat.name) + L" " + shot.name + L" for " + std::to_wstring(out.runs) + L".";
    } else if (val > 0.2f) {
        out.runs = 1;
        out.desc = ToW(bat.name) + L" taps for a single.";
    } else {
        out.runs = 0;
        out.desc = ToW(bat.name) + L" " + shot.name + L", can't beat the field. Dot ball.";
    }

    // Big boundary penalty
    if (out.isFour && G.stadium.boundarySize > 72 && G.RandF() < 0.12f) {
        out.runs = 3; out.isFour = false;
        out.desc = ToW(bat.name) + L" doesn't quite reach the rope, 3 runs.";
    }

    return out;
}

DeliveryOutcome ResolveAIBatting(int deliveryIdx, float difficulty) {
    DeliveryOutcome out;
    auto& bat = G.Striker();
    auto& bowler = G.Bowler();

    if (G.RandF() < 0.025f - bowler.bowlingSkill/100.f*0.015f) {
        out.isWide = true; out.runs = 1; out.desc = L"Wide!"; return out;
    }
    if (G.RandF() < 0.012f) {
        out.isNoBall = true; out.runs = 1; out.desc = L"No ball!"; return out;
    }

    // AI shot selection based on skill
    float aggression = 1.f - bat.temperament;
    if (G.target > 0) {
        float need = (float)(G.target - G.score.totalRuns);
        float ovLeft = G.maxOvers - G.GetOvers();
        if (ovLeft > 0 && need/ovLeft > 7) aggression += 0.25f;
    }
    aggression = std::clamp(aggression, 0.1f, 0.9f);

    // Dismissal
    float dc = difficulty * 0.07f;
    dc *= (1.f - bat.battingSkill / 150.f);
    dc *= (1.f - bat.technique * 0.25f);
    if (G.StrikerInnings().ballsFaced < 8) dc *= 1.4f;
    // Delivery bonus
    if (deliveryIdx >= 0) dc += g_deliveries[deliveryIdx].difficultyAdd * 0.3f;
    dc = std::clamp(dc, 0.004f, 0.13f);

    if (G.RandF() < dc) {
        out.isWicket = true;
        float r = G.RandF();
        if (r < 0.2f) out.dismissal = DismissalType::Bowled;
        else if (r < 0.5f) out.dismissal = DismissalType::Caught;
        else if (r < 0.65f) out.dismissal = DismissalType::CaughtBehind;
        else if (r < 0.8f) out.dismissal = DismissalType::LBW;
        else out.dismissal = DismissalType::CaughtAndBowled;
        out.desc = ToW(bat.name) + L" OUT! b " + ToW(bowler.name);
        return out;
    }

    float val = (bat.battingSkill / 100.f) * G.RandF() * (0.5f + aggression * 0.5f);
    if (val > 0.88f && G.RandF() < aggression) {
        out.runs = 6; out.isSix = true;
        out.desc = ToW(bat.name) + L" launches it for SIX!";
    } else if (val > 0.65f) {
        out.runs = 4; out.isFour = true;
        out.desc = ToW(bat.name) + L" finds the boundary! FOUR!";
    } else if (val > 0.4f) {
        out.runs = G.RandI(1, 3);
        out.desc = ToW(bat.name) + L" works it for " + std::to_wstring(out.runs) + L".";
    } else if (val > 0.2f) {
        out.runs = 1;
        out.desc = ToW(bat.name) + L" pushes for one.";
    } else {
        out.runs = 0;
        out.desc = ToW(bat.name) + L" defends. Dot ball.";
    }
    return out;
}

// ============================================================
//  APPLY DELIVERY RESULT
// ============================================================
void ApplyResult(const DeliveryOutcome& out) {
    G.lastRuns = out.runs;
    G.lastIsFour = out.isFour;
    G.lastIsSix = out.isSix;
    G.lastIsWicket = out.isWicket;
    G.lastIsWide = out.isWide;
    G.lastIsNoBall = out.isNoBall;
    G.lastDismissal = out.dismissal;
    G.lastDesc = out.desc;
    G.AddComment(out.desc);

    auto* bf = FindBowlerFig();

    if (out.isWide) {
        G.score.totalRuns++; G.score.extras++; G.score.wides++;
        if (bf) bf->runsConceded++;
        G.thisOver.push_back({1, false, true});
        return;
    }
    if (out.isNoBall) {
        G.score.totalRuns++; G.score.extras++; G.score.noBalls++;
        if (bf) bf->runsConceded++;
        G.thisOver.push_back({1, false, true});
        return;
    }

    // Legal delivery
    G.score.ballsInOver++;
    G.ballCond.AgeBall();
    auto& bi = G.StrikerInnings();
    bi.ballsFaced++;

    if (out.isWicket) {
        bi.runs += out.runs;
        G.score.totalRuns += out.runs;
        if (bf) { bf->runsConceded += out.runs; bf->wickets++; }
        bi.isOut = true;
        bi.howOut = out.dismissal;
        bi.dismissedBy = G.Bowler().name;
        G.score.wickets++;

        std::wostringstream fow;
        fow << G.score.wickets << L"-" << G.score.totalRuns;
        std::wstring fowW = fow.str();
        std::string fowS;
        for (wchar_t ch : fowW) fowS += (char)ch;
        G.score.fallOfWickets.push_back(fowS);

        G.thisOver.push_back({0, true, false});

        if (G.nextBatIdx < (int)G.teams[G.battingTeamIdx].players.size()) {
            G.strikerIdx = G.nextBatIdx++;
        }
    } else {
        G.score.totalRuns += out.runs;
        bi.runs += out.runs;
        if (bf) bf->runsConceded += out.runs;
        if (out.isFour) bi.fours++;
        if (out.isSix) bi.sixes++;
        G.thisOver.push_back({out.runs, false, false});
        if (out.runs % 2 == 1) G.SwapStrike();

        // Wagon wheel
        if (out.runs > 0) {
            float angle = G.RandF(-PI, PI);
            float dist = 0.2f + out.runs * 0.12f;
            if (out.isSix) dist = 0.95f;
            if (out.isFour) dist = 0.85f;
            G.wagonWheel.push_back({angle, dist, out.runs, false});
        }
    }

    // Over complete?
    if (G.score.ballsInOver >= 6) {
        G.score.oversCompleted++;
        if (bf) {
            bf->overs++;
            bf->ballsInCurrentOver = 0;
            int runsThisOver = 0;
            for (auto& b : G.thisOver) if (!b.isExtra) runsThisOver += b.runs;
            if (runsThisOver == 0) bf->maidens++;
        }
        G.score.ballsInOver = 0;
        G.SwapStrike();
        G.lastBowlerIdx = G.currentBowlerIdx;
        SelectNextBowler();
        G.thisOver.clear();

        std::wostringstream ss;
        ss << L"End of over " << G.score.oversCompleted << L": "
           << G.score.totalRuns << L"/" << G.score.wickets;
        G.AddComment(ss.str());
    }
}

// ============================================================
//  BALL ANIMATION ENDPOINTS
// ============================================================
void SetBallAnim(int runs, bool isWicket, bool isSix, float shotAngle) {
    G.ballStartX = (float)FX;
    G.ballStartY = (float)(FY - 60); // bowler end

    if (isWicket) {
        G.ballEndX = (float)FX;
        G.ballEndY = (float)(FY + 45);
    } else if (runs == 0) {
        G.ballEndX = (float)FX;
        G.ballEndY = (float)(FY + 55);
    } else {
        float dist = FR * (0.15f + runs * 0.13f);
        if (isSix) dist = FR * 0.95f;
        else if (runs == 4) dist = FR * 0.85f;
        G.ballEndX = FX + sinf(shotAngle) * dist;
        G.ballEndY = FY - cosf(shotAngle) * dist * 0.85f;
    }
    // Arc midpoint
    G.ballMidX = (G.ballStartX + G.ballEndX) / 2 + G.RandF(-20, 20);
    G.ballMidY = std::min(G.ballStartY, G.ballEndY) - 30;
    G.ballAnimT = 0;
    G.ballAnimActive = true;
}

// ============================================================
//  UPDATE (called every frame ~60fps)
// ============================================================
void Update(float dt) {
    // Clear just-pressed
    memset(g_keysJust, 0, sizeof(g_keysJust));

    if (G.pauseFrames > 0) { G.pauseFrames--; return; }

    switch (G.phase) {
    // ---- MENU ----
    case Phase::Menu: {
        if (G.menuStep == 0) {
            if (g_keys['1']) { G.format = MatchFormat::ODI; G.menuStep = 1; G.menuChoice = 0; G.allStadiums = TeamDatabase::GetStadiums(); Sleep(150); }
            if (g_keys['2']) { G.format = MatchFormat::Test; G.menuStep = 1; G.menuChoice = 0; G.allStadiums = TeamDatabase::GetStadiums(); Sleep(150); }
        } else if (G.menuStep == 1) {
            if (g_keys[VK_UP] && G.menuChoice > 0) { G.menuChoice--; Sleep(120); }
            if (g_keys[VK_DOWN] && G.menuChoice < (int)G.allStadiums.size()-1) { G.menuChoice++; Sleep(120); }
            if (g_keys[VK_RETURN]) {
                G.stadium = G.allStadiums[G.menuChoice];
                StartMatch();
                Sleep(150);
            }
        }
        break;
    }

    case Phase::TossResult:
        G.phase = G.userBatting ? Phase::BattingReady : Phase::BowlingReady;
        break;

    // ---- USER BATTING ----
    case Phase::BattingReady: {
        // Show "Press SPACE when bowler runs in"
        if (g_keys[VK_SPACE]) {
            // Ball starts travelling
            float diff = CalcDifficulty(-1);
            G.shotTimer = G.shotTimerMax;
            G.shotTimerActive = true;
            G.selectedShotIdx = -1;
            G.phase = Phase::ShotSelect;

            // Start ball anim from bowler to batsman
            G.ballStartX = (float)FX;
            G.ballStartY = (float)(FY - 60);
            G.ballEndX = (float)FX;
            G.ballEndY = (float)(FY + 45);
            G.ballMidX = (float)FX;
            G.ballMidY = (float)(FY - 10);
            G.ballAnimT = 0;
            G.ballAnimActive = true;
        }
        break;
    }

    case Phase::ShotSelect: {
        G.shotTimer -= dt;
        G.ballAnimT = std::min(1.f, G.ballAnimT + dt / G.shotTimerMax);

        // Check for shot key press
        for (int i = 0; i < NUM_SHOTS; i++) {
            if (g_keys[g_shots[i].key]) {
                G.selectedShotIdx = i;
                G.lastShotName = g_shots[i].name;
                break;
            }
        }

        if (G.shotTimer <= 0 || G.selectedShotIdx >= 0) {
            G.shotTimerActive = false;
            G.ballAnimActive = false;

            // Resolve
            float diff = CalcDifficulty(-1);
            // Timing bonus: if picked shot early, better contact
            if (G.selectedShotIdx >= 0 && G.shotTimer > 0.3f) diff *= 0.9f;

            auto result = ResolveUserBatting(G.selectedShotIdx, diff);

            // Set ball animation to field
            float angle = (G.selectedShotIdx >= 0) ? g_shots[G.selectedShotIdx].angle + G.RandF(-0.2f, 0.2f) : 0;
            SetBallAnim(result.runs, result.isWicket, result.isSix, angle);

            ApplyResult(result);
            G.phase = Phase::ShotResult;
            G.pauseFrames = result.isWicket ? 80 : (result.isFour || result.isSix ? 50 : 25);
        }
        break;
    }

    case Phase::ShotResult: {
        G.ballAnimT = std::min(1.f, G.ballAnimT + dt * 2.5f);
        if (G.ballAnimT >= 1.f) G.ballAnimActive = false;

        // After pause, next ball or innings over
        if (G.IsInningsOver()) {
            G.phase = Phase::InningsBreak;
            G.pauseFrames = 90;
        } else {
            G.phase = Phase::BattingReady;
        }
        break;
    }

    // ---- USER BOWLING ----
    case Phase::BowlingReady: {
        // Pick delivery with 1-6
        for (int i = 0; i < NUM_DELIVERIES; i++) {
            if (g_keys[g_deliveries[i].key]) {
                G.selectedDeliveryIdx = i;
                G.phase = Phase::BowlResult;

                float diff = CalcDifficulty(i);
                auto result = ResolveAIBatting(i, diff);

                float angle = G.RandF(-PI, PI);
                SetBallAnim(result.runs, result.isWicket, result.isSix, angle);

                ApplyResult(result);
                G.pauseFrames = result.isWicket ? 70 : (result.isFour || result.isSix ? 40 : 18);
                Sleep(100);
                break;
            }
        }
        break;
    }

    case Phase::BowlResult: {
        G.ballAnimT = std::min(1.f, G.ballAnimT + dt * 2.5f);
        if (G.ballAnimT >= 1.f) G.ballAnimActive = false;

        if (G.IsInningsOver()) {
            G.phase = Phase::InningsBreak;
            G.pauseFrames = 90;
        } else {
            G.phase = Phase::BowlingReady;
        }
        break;
    }

    // ---- INNINGS BREAK ----
    case Phase::InningsBreak: {
        if (G.format == MatchFormat::ODI && G.currentInnings == 0) {
            G.target = G.score.totalRuns + 1;
            G.currentInnings = 1;
            std::swap(G.battingTeamIdx, G.bowlingTeamIdx);
            G.userBatting = !G.userBatting;
            G.AddComment(L"--- Innings Break ---");
            std::wostringstream ss;
            ss << ToW(G.teams[G.battingTeamIdx].name) << L" need " << G.target << L" to win.";
            G.AddComment(ss.str());
            InitInnings();
            G.phase = G.userBatting ? Phase::BattingReady : Phase::BowlingReady;
        } else {
            G.phase = Phase::MatchOver;
            // Determine winner
            if (G.currentInnings == 1 && G.score.totalRuns >= G.target) {
                G.AddComment(ToW(G.teams[G.battingTeamIdx].name) + L" WIN by " +
                    std::to_wstring(10 - G.score.wickets) + L" wickets!");
            } else {
                G.AddComment(ToW(G.teams[G.bowlingTeamIdx].name) + L" WIN by " +
                    std::to_wstring(G.target - 1 - G.score.totalRuns) + L" runs!");
            }
        }
        break;
    }

    case Phase::MatchOver:
        // Press R to restart
        if (g_keys['R']) {
            G.phase = Phase::Menu;
            G.menuStep = 0;
            G.commentary.clear();
            Sleep(200);
        }
        break;
    }
}

// ============================================================
//  RENDER
// ============================================================
void Render(HDC dc) {
    // Background
    FillBox(dc, 0, 0, W, H, RGB(20, 30, 20));

    if (G.phase == Phase::Menu) {
        DrawTxt(dc, 80, 60, L"CRICKET GAME", RGB(255,255,100), 36, true, L"Arial");
        DrawTxt(dc, 80, 110, L"England vs Australia", RGB(200,200,200), 20, false, L"Arial");

        if (G.menuStep == 0) {
            DrawTxt(dc, 80, 180, L"Select Format:", RGB(255,255,255), 18, true);
            DrawTxt(dc, 100, 220, L"[1]  ODI  (50 overs)", RGB(200,255,200), 16);
            DrawTxt(dc, 100, 250, L"[2]  Test Match  (90 overs/innings)", RGB(200,255,200), 16);
        } else {
            DrawTxt(dc, 80, 180, L"Select Stadium (Up/Down, Enter):", RGB(255,255,255), 18, true);
            for (int i = 0; i < (int)G.allStadiums.size(); i++) {
                COLORREF c = (i == G.menuChoice) ? RGB(255,255,100) : RGB(180,180,180);
                std::wstring label = (i == G.menuChoice ? L"> " : L"  ") +
                    ToW(G.allStadiums[i].name) + L"  (" + ToW(G.allStadiums[i].city) + L", " +
                    ToW(G.allStadiums[i].country) + L")";
                DrawTxt(dc, 100, 220 + i * 28, label, c, 15, i == G.menuChoice);
            }
        }

        // Controls help
        DrawTxt(dc, 80, 560, L"CONTROLS:", RGB(150,200,255), 16, true);
        DrawTxt(dc, 80, 585, L"BATTING: SPACE=play ball, then W/A/S/D/Q/E/Z/X/C = shots", RGB(180,180,180), 13);
        DrawTxt(dc, 80, 605, L"  W=Straight Drive  A=Cover Drive  D=Flick  S=Defend", RGB(160,160,160), 13);
        DrawTxt(dc, 80, 625, L"  Q=Cut  E=Lofted Drive  Z=Sweep  X=Pull  C=Upper Cut", RGB(160,160,160), 13);
        DrawTxt(dc, 80, 650, L"BOWLING: 1=Good  2=Short  3=Full  4=Yorker  5=Bouncer  6=Slower", RGB(180,180,180), 13);
        DrawTxt(dc, 80, 675, L"ESC=Quit   R=Restart (after match)", RGB(150,150,150), 13);
        return;
    }

    // ---- FIELD ----
    // Green oval
    HBRUSH fb = CreateSolidBrush(RGB(34,120,50));
    HPEN fp = CreatePen(PS_SOLID, 2, RGB(200,200,200));
    SelectObject(dc, fb); SelectObject(dc, fp);
    Ellipse(dc, FX-FR, FY-FR+20, FX+FR, FY+FR-20);
    DeleteObject(fb); DeleteObject(fp);

    // 30-yard circle
    HPEN ip = CreatePen(PS_DOT, 1, RGB(180,180,180));
    SelectObject(dc, ip); SelectObject(dc, (HBRUSH)GetStockObject(NULL_BRUSH));
    int ir = FR * 45 / 100;
    Ellipse(dc, FX-ir, FY-ir+10, FX+ir, FY+ir-10);
    DeleteObject(ip);

    // Pitch
    FillBox(dc, FX-7, FY-50, 14, 100, RGB(194,164,110));
    // Creases
    DrawLine(dc, FX-18, FY+42, FX+18, FY+42, RGB(255,255,255));
    DrawLine(dc, FX-18, FY-42, FX+18, FY-42, RGB(255,255,255));
    // Stumps
    FillBox(dc, FX-2, FY+38, 4, 8, RGB(200,180,100));
    FillBox(dc, FX-2, FY-46, 4, 8, RGB(200,180,100));

    // Fielders
    struct FP { float a, d; };
    FP fpos[] = {{0,0.35f},{PI,0.35f},{PI/2,0.55f},{-PI/2,0.55f},
                 {PI/4,0.45f},{-PI/4,0.45f},{PI*3/4,0.7f},{-PI*3/4,0.7f},{0.15f,0.15f}};
    for (auto& f : fpos) {
        int fx = FX + (int)(sinf(f.a)*FR*f.d);
        int fy = FY - (int)(cosf(f.a)*(FR-20)*f.d);
        FillCirc(dc, fx, fy, 4, RGB(255,255,255));
    }

    // Batsman
    FillCirc(dc, FX+8, FY+42, 6, RGB(255,220,50));
    // Non-striker
    FillCirc(dc, FX-8, FY-42, 5, RGB(255,220,50));
    // Bowler
    FillCirc(dc, FX, FY-70, 6, RGB(255,255,255));
    // Keeper
    FillCirc(dc, FX, FY+55, 5, RGB(255,255,255));

    // Wagon wheel
    for (auto& w : G.wagonWheel) {
        int ex = FX + (int)(sinf(w.angle) * FR * w.dist);
        int ey = FY - (int)(cosf(w.angle) * (FR-20) * w.dist);
        COLORREF lc = w.runs >= 6 ? RGB(255,200,0) : (w.runs >= 4 ? RGB(0,200,0) : RGB(150,150,200));
        DrawLine(dc, FX, FY+42, ex, ey, lc, 1);
    }

    // Ball animation
    if (G.ballAnimActive) {
        float t = G.ballAnimT;
        float u = 1-t;
        float bx = u*u*G.ballStartX + 2*u*t*G.ballMidX + t*t*G.ballEndX;
        float by = u*u*G.ballStartY + 2*u*t*G.ballMidY + t*t*G.ballEndY;
        COLORREF bc = G.lastIsSix ? RGB(255,200,0) : (G.lastIsFour ? RGB(0,220,0) :
                      (G.lastIsWicket ? RGB(255,50,50) : RGB(200,30,30)));
        FillCirc(dc, (int)bx, (int)by, 5, bc);

        // Trail
        for (float s = 0; s < t; s += 0.05f) {
            float su = 1-s;
            int tx = (int)(su*su*G.ballStartX + 2*su*s*G.ballMidX + s*s*G.ballEndX);
            int ty = (int)(su*su*G.ballStartY + 2*su*s*G.ballMidY + s*s*G.ballEndY);
            FillCirc(dc, tx, ty, 2, RGB(180,80,80));
        }
    }

    // Result flash
    if ((G.phase == Phase::ShotResult || G.phase == Phase::BowlResult) && G.pauseFrames > 0) {
        std::wstring flash;
        COLORREF fc = RGB(255,255,255);
        if (G.lastIsWicket) { flash = L"WICKET!"; fc = RGB(255,50,50); }
        else if (G.lastIsSix) { flash = L"SIX!"; fc = RGB(255,220,0); }
        else if (G.lastIsFour) { flash = L"FOUR!"; fc = RGB(0,220,0); }
        if (!flash.empty())
            DrawTxt(dc, FX - 50, FY - 15, flash, fc, 32, true, L"Arial");
    }

    // ---- SCOREBOARD (right panel) ----
    int sx = 750, sy = 10;
    FillBox(dc, sx-5, sy-5, W-sx+10, H-10, RGB(15,15,50));
    DrawLine(dc, sx-5, sy-5, sx-5, H-5, RGB(80,80,150), 2);

    int y = sy;
    // Format + venue
    DrawTxt(dc, sx, y, G.format == MatchFormat::ODI ? L"ODI" : L"TEST MATCH", RGB(100,180,255), 13);
    y += 16;
    DrawTxt(dc, sx, y, ToW(G.stadium.name + ", " + G.stadium.city), RGB(150,150,150), 12);
    y += 20;

    // Team + Score
    DrawTxt(dc, sx, y, ToW(G.teams[G.battingTeamIdx].name), RGB(255,255,100), 18, true);
    y += 22;
    std::wostringstream sc;
    sc << G.score.totalRuns << L"/" << G.score.wickets << L"  ("
       << G.score.oversCompleted;
    if (G.score.ballsInOver > 0) sc << L"." << G.score.ballsInOver;
    sc << L" ov)";
    DrawTxt(dc, sx, y, sc.str(), RGB(255,255,255), 26, true);
    y += 32;

    // Run rate + target
    float rr = G.GetOvers() > 0 ? G.score.totalRuns / G.GetOvers() : 0;
    std::wostringstream rrS;
    rrS << L"RR: " << std::fixed << std::setprecision(2) << rr;
    if (G.target > 0) {
        int need = G.target - G.score.totalRuns;
        if (need > 0) {
            float ovLeft = G.maxOvers - G.GetOvers();
            float reqRR = ovLeft > 0 ? need / ovLeft : 99;
            rrS << L"   Need " << need << L" off " << std::setprecision(1) << ovLeft
                << L" ov (RRR:" << std::setprecision(2) << reqRR << L")";
        }
    }
    DrawTxt(dc, sx, y, rrS.str(), RGB(180,180,180), 12);
    y += 22;

    DrawLine(dc, sx, y, W-15, y, RGB(60,60,120));
    y += 6;

    // Batsmen
    DrawTxt(dc, sx, y, L"BATTING", RGB(100,180,255), 12, true); y += 16;
    {
        auto& bi = G.score.battingCard[G.strikerIdx];
        std::wostringstream s;
        s << L"* " << ToW(G.Striker().name) << L"  " << bi.runs
          << L" (" << bi.ballsFaced << L"b " << bi.fours << L"x4 " << bi.sixes << L"x6)";
        DrawTxt(dc, sx, y, s.str(), RGB(255,255,255), 14, true); y += 18;
    }
    {
        auto& bi = G.score.battingCard[G.nonStrikerIdx];
        std::wostringstream s;
        s << L"  " << ToW(G.NonStriker().name) << L"  " << bi.runs
          << L" (" << bi.ballsFaced << L"b)";
        DrawTxt(dc, sx, y, s.str(), RGB(180,180,180), 13); y += 22;
    }

    // Bowler
    DrawTxt(dc, sx, y, L"BOWLING", RGB(100,180,255), 12, true); y += 16;
    {
        auto* bf = FindBowlerFig();
        std::wostringstream s;
        s << ToW(G.Bowler().name);
        if (bf) s << L"  " << bf->wickets << L"/" << bf->runsConceded
                  << L" (" << bf->overs << L"." << bf->ballsInCurrentOver << L" ov)";
        DrawTxt(dc, sx, y, s.str(), RGB(255,255,255), 14); y += 22;
    }

    DrawLine(dc, sx, y, W-15, y, RGB(60,60,120));
    y += 6;

    // This over
    DrawTxt(dc, sx, y, L"THIS OVER", RGB(100,180,255), 12, true); y += 18;
    int ox = sx;
    for (auto& b : G.thisOver) {
        COLORREF c;
        std::wstring lbl;
        if (b.isWicket) { c = RGB(200,40,40); lbl = L"W"; }
        else if (b.isExtra) { c = RGB(100,100,180); lbl = L"+"; }
        else if (b.runs == 0) { c = RGB(80,80,80); lbl = L"0"; }
        else if (b.runs == 4) { c = RGB(0,160,0); lbl = L"4"; }
        else if (b.runs == 6) { c = RGB(200,180,0); lbl = L"6"; }
        else { c = RGB(150,150,150); lbl = std::to_wstring(b.runs); }
        FillCirc(dc, ox+12, y+10, 12, c);
        DrawTxt(dc, ox+7, y+3, lbl, RGB(255,255,255), 14, true);
        ox += 28;
    }
    y += 32;

    DrawLine(dc, sx, y, W-15, y, RGB(60,60,120));
    y += 6;

    // Commentary
    DrawTxt(dc, sx, y, L"COMMENTARY", RGB(100,180,255), 12, true); y += 16;
    for (int i = (int)G.commentary.size()-1; i >= 0 && y < H - 80; i--) {
        DrawTxt(dc, sx, y, G.commentary[i], RGB(180,180,180), 11);
        y += 14;
    }

    // ---- CONTROLS PROMPT (bottom) ----
    int py = H - 55;
    FillBox(dc, 0, py - 5, 750, 60, RGB(10,10,30));

    if (G.phase == Phase::BattingReady) {
        DrawTxt(dc, 20, py, L"Press SPACE to face the delivery...", RGB(100,255,100), 16, true);
        DrawTxt(dc, 20, py+22, L"Then pick your shot: W=Drive A=Cover D=Flick Q=Cut E=Loft Z=Sweep X=Pull C=UpperCut S=Block SPACE=Leave",
                RGB(180,180,180), 12);
    } else if (G.phase == Phase::ShotSelect) {
        // Timer bar
        float pct = G.shotTimer / G.shotTimerMax;
        int barW = 300;
        FillBox(dc, 20, py, barW, 16, RGB(60,60,60));
        FillBox(dc, 20, py, (int)(barW * pct), 16, pct > 0.3f ? RGB(0,200,0) : RGB(200,50,0));
        DrawTxt(dc, 330, py, L"PICK YOUR SHOT NOW!", RGB(255,255,100), 16, true);
        DrawTxt(dc, 20, py+22, L"W=Drive A=Cover D=Flick Q=Cut E=Loft Z=Sweep X=Pull C=UpperCut S=Block SPACE=Leave",
                RGB(200,200,200), 12);
    } else if (G.phase == Phase::BowlingReady) {
        DrawTxt(dc, 20, py, L"You're bowling! Pick delivery:", RGB(100,200,255), 16, true);
        DrawTxt(dc, 20, py+22, L"1=Good Length  2=Short  3=Full  4=Yorker  5=Bouncer  6=Slower Ball",
                RGB(200,200,200), 13);
    } else if (G.phase == Phase::MatchOver) {
        DrawTxt(dc, 20, py, L"MATCH OVER!  Press R to play again.", RGB(255,255,100), 18, true);
        if (!G.commentary.empty())
            DrawTxt(dc, 20, py+24, G.commentary.back(), RGB(255,200,100), 16, true);
    }
}

// ============================================================
//  WIN32 WINDOW
// ============================================================
LRESULT CALLBACK WndProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HDC hdc = GetDC(hw);
        g_memDC = CreateCompatibleDC(hdc);
        g_memBmp = CreateCompatibleBitmap(hdc, W, H);
        SelectObject(g_memDC, g_memBmp);
        ReleaseDC(hw, hdc);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hw, &ps);
        Render(g_memDC);
        BitBlt(hdc, 0, 0, W, H, g_memDC, 0, 0, SRCCOPY);
        EndPaint(hw, &ps);
        return 0;
    }
    case WM_KEYDOWN:
        if (wp < 256) g_keys[wp] = true;
        if (wp == VK_ESCAPE) { PostQuitMessage(0); }
        return 0;
    case WM_KEYUP:
        if (wp < 256) g_keys[wp] = false;
        return 0;
    case WM_DESTROY:
        if (g_memBmp) DeleteObject(g_memBmp);
        if (g_memDC) DeleteDC(g_memDC);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hw, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = L"CricketInteractive";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    RECT wr = {0,0,W,H};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);

    g_hwnd = CreateWindowExW(0, L"CricketInteractive",
        L"Cricket Game - Interactive",
        (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right-wr.left, wr.bottom-wr.top,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(g_hwnd, nShow);
    UpdateWindow(g_hwnd);

    QueryPerformanceFrequency(&g_freq);
    QueryPerformanceCounter(&g_lastTime);

    MSG message;
    bool running = true;
    while (running) {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (!running) break;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float dt = (float)(now.QuadPart - g_lastTime.QuadPart) / g_freq.QuadPart;
        g_lastTime = now;
        dt = std::min(dt, 0.05f);

        Update(dt);
        InvalidateRect(g_hwnd, nullptr, FALSE);
        Sleep(16);
    }
    return 0;
}
