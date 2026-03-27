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
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include "MatchEngine.h"
#include "TeamData.h"

// --- Constants ---
static const int WIN_W = 1100;
static const int WIN_H = 750;
static const int FIELD_CX = 400;
static const int FIELD_CY = 370;
static const int FIELD_R = 280;
static const int PITCH_W = 14;
static const int PITCH_H = 100;
static const float PI_F = 3.14159265f;

// --- Global State ---
struct BallAnim {
    float startX, startY;
    float endX, endY;
    float progress = 0.0f; // 0-1
    bool active = false;
    bool isFour = false;
    bool isSix = false;
    bool isWicket = false;
    int runs = 0;
    std::wstring shotDesc;
};

struct GameVisualState {
    std::mutex mtx;
    // Score
    std::wstring teamBatting;
    std::wstring teamBowling;
    int totalRuns = 0;
    int wickets = 0;
    int oversCompleted = 0;
    int ballsInOver = 0;
    float runRate = 0.0f;
    int target = -1;
    std::wstring matchFormat;
    std::wstring stadiumName;
    std::wstring weatherDesc;

    // Current players
    std::wstring strikerName;
    int strikerRuns = 0;
    int strikerBalls = 0;
    std::wstring nonStrikerName;
    int nonStrikerRuns = 0;
    std::wstring bowlerName;
    int bowlerWickets = 0;
    int bowlerRuns = 0;
    int bowlerOvers = 0;

    // Ball animation
    BallAnim ball;

    // Commentary log (last 12 lines)
    std::vector<std::wstring> commentary;

    // Match state
    bool matchStarted = false;
    bool matchEnded = false;
    std::wstring resultText;

    // Innings info
    int currentInnings = 0;

    // Ball trail dots
    struct TrailDot { float x, y; int runs; bool isWicket; };
    std::vector<TrailDot> thisOverDots;

    void AddCommentary(const std::wstring& line) {
        commentary.push_back(line);
        if (commentary.size() > 14) commentary.erase(commentary.begin());
    }
};

static GameVisualState g_state;
static HWND g_hwnd = nullptr;
static std::atomic<bool> g_running{true};
static std::atomic<bool> g_simDone{false};
static HBITMAP g_backbuffer = nullptr;
static HDC g_backDC = nullptr;

// --- Helpers ---
std::wstring ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int sz = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], sz);
    return w;
}

COLORREF FieldGreen() { return RGB(34, 120, 50); }
COLORREF PitchBrown() { return RGB(194, 164, 110); }
COLORREF BoundaryWhite() { return RGB(220, 220, 220); }
COLORREF SkyBlue() { return RGB(135, 190, 230); }
COLORREF ScoreboardBg() { return RGB(20, 20, 60); }
COLORREF BallRed() { return RGB(200, 30, 30); }
COLORREF PlayerWhite() { return RGB(255, 255, 255); }
COLORREF PlayerYellow() { return RGB(255, 220, 50); }

void FillCircle(HDC hdc, int cx, int cy, int r, COLORREF fill) {
    HBRUSH br = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, fill);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br);
    DeleteObject(pen);
}

void DrawTextStr(HDC hdc, int x, int y, const std::wstring& text, COLORREF color, int fontSize = 14, bool bold = false) {
    HFONT font = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, x, y, text.c_str(), (int)text.size());
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// --- Drawing ---
void DrawField(HDC hdc) {
    // Sky background
    RECT rc = {0, 0, WIN_W, WIN_H};
    HBRUSH skyBr = CreateSolidBrush(SkyBlue());
    FillRect(hdc, &rc, skyBr);
    DeleteObject(skyBr);

    // Field circle (green oval)
    HBRUSH fieldBr = CreateSolidBrush(FieldGreen());
    HPEN fieldPen = CreatePen(PS_SOLID, 2, BoundaryWhite());
    SelectObject(hdc, fieldBr);
    SelectObject(hdc, fieldPen);
    Ellipse(hdc, FIELD_CX - FIELD_R, FIELD_CY - FIELD_R + 20,
                 FIELD_CX + FIELD_R, FIELD_CY + FIELD_R - 20);
    DeleteObject(fieldBr);
    DeleteObject(fieldPen);

    // Inner circle (30-yard)
    HPEN innerPen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(hdc, innerPen);
    SelectObject(hdc, nullBr);
    int innerR = FIELD_R * 45 / 100;
    Ellipse(hdc, FIELD_CX - innerR, FIELD_CY - innerR + 10,
                 FIELD_CX + innerR, FIELD_CY + innerR - 10);
    DeleteObject(innerPen);

    // Pitch rectangle
    HBRUSH pitchBr = CreateSolidBrush(PitchBrown());
    RECT pitchRc = {FIELD_CX - PITCH_W/2, FIELD_CY - PITCH_H/2,
                    FIELD_CX + PITCH_W/2, FIELD_CY + PITCH_H/2};
    FillRect(hdc, &pitchRc, pitchBr);
    DeleteObject(pitchBr);

    // Crease lines
    HPEN creasePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    SelectObject(hdc, creasePen);
    // Batting crease (bottom)
    MoveToEx(hdc, FIELD_CX - 20, FIELD_CY + PITCH_H/2 - 8, nullptr);
    LineTo(hdc, FIELD_CX + 20, FIELD_CY + PITCH_H/2 - 8);
    // Bowling crease (top)
    MoveToEx(hdc, FIELD_CX - 20, FIELD_CY - PITCH_H/2 + 8, nullptr);
    LineTo(hdc, FIELD_CX + 20, FIELD_CY - PITCH_H/2 + 8);
    DeleteObject(creasePen);

    // Stumps (small rectangles)
    HBRUSH stumpBr = CreateSolidBrush(RGB(200, 180, 100));
    RECT stump1 = {FIELD_CX - 3, FIELD_CY + PITCH_H/2 - 12, FIELD_CX + 3, FIELD_CY + PITCH_H/2 - 4};
    RECT stump2 = {FIELD_CX - 3, FIELD_CY - PITCH_H/2 + 4, FIELD_CX + 3, FIELD_CY - PITCH_H/2 + 12};
    FillRect(hdc, &stump1, stumpBr);
    FillRect(hdc, &stump2, stumpBr);
    DeleteObject(stumpBr);

    // Fielder positions (static dots)
    struct FielderPos { float angle; float dist; };
    FielderPos fielders[] = {
        {0.0f, 0.35f},       // mid-off
        {PI_F, 0.35f},       // mid-on
        {PI_F/2, 0.55f},     // point
        {-PI_F/2, 0.55f},    // square leg
        {PI_F/4, 0.45f},     // cover
        {-PI_F/4, 0.45f},    // midwicket
        {PI_F*3/4, 0.7f},    // fine leg
        {-PI_F*3/4, 0.7f},   // third man
        {0.15f, 0.15f},      // slip
    };
    for (auto& f : fielders) {
        int fx = FIELD_CX + (int)(sinf(f.angle) * FIELD_R * f.dist);
        int fy = FIELD_CY - (int)(cosf(f.angle) * (FIELD_R - 20) * f.dist);
        FillCircle(hdc, fx, fy, 5, PlayerWhite());
    }

    // Batsman (at batting crease)
    FillCircle(hdc, FIELD_CX + 10, FIELD_CY + PITCH_H/2 - 8, 6, PlayerYellow());
    // Non-striker
    FillCircle(hdc, FIELD_CX - 10, FIELD_CY - PITCH_H/2 + 8, 5, PlayerYellow());
    // Bowler (running in)
    FillCircle(hdc, FIELD_CX, FIELD_CY - PITCH_H/2 - 20, 6, PlayerWhite());
    // Keeper
    FillCircle(hdc, FIELD_CX, FIELD_CY + PITCH_H/2 + 15, 6, PlayerWhite());
}

void DrawBallTrajectory(HDC hdc) {
    std::lock_guard<std::mutex> lock(g_state.mtx);
    auto& b = g_state.ball;
    if (!b.active) return;

    float t = b.progress;
    float cx = b.startX + (b.endX - b.startX) * t;
    float cy = b.startY + (b.endY - b.startY) * t;

    // Trail
    HPEN trailPen = CreatePen(PS_DOT, 1, RGB(255, 100, 100));
    SelectObject(hdc, trailPen);
    MoveToEx(hdc, (int)b.startX, (int)b.startY, nullptr);
    LineTo(hdc, (int)cx, (int)cy);
    DeleteObject(trailPen);

    // Ball
    COLORREF ballCol = b.isSix ? RGB(255, 200, 0) : (b.isFour ? RGB(0, 200, 0) : BallRed());
    FillCircle(hdc, (int)cx, (int)cy, 5, ballCol);

    // Runs text near ball
    if (t > 0.7f) {
        std::wstring runText;
        if (b.isWicket) runText = L"OUT!";
        else if (b.isSix) runText = L"SIX!";
        else if (b.isFour) runText = L"FOUR!";
        else if (b.runs > 0) runText = std::to_wstring(b.runs);
        if (!runText.empty()) {
            COLORREF tc = b.isWicket ? RGB(255, 50, 50) : (b.isSix ? RGB(255, 200, 0) : RGB(255, 255, 255));
            DrawTextStr(hdc, (int)cx + 10, (int)cy - 10, runText, tc, 18, true);
        }
    }
}

void DrawThisOverDots(HDC hdc) {
    std::lock_guard<std::mutex> lock(g_state.mtx);
    int startX = 720;
    int y = 680;
    DrawTextStr(hdc, startX, y - 20, L"This Over:", RGB(200, 200, 200), 13, false);
    int x = startX;
    for (auto& dot : g_state.thisOverDots) {
        COLORREF col;
        std::wstring label;
        if (dot.isWicket) { col = RGB(255, 50, 50); label = L"W"; }
        else if (dot.runs == 0) { col = RGB(100, 100, 100); label = L"0"; }
        else if (dot.runs == 4) { col = RGB(0, 180, 0); label = L"4"; }
        else if (dot.runs == 6) { col = RGB(255, 200, 0); label = L"6"; }
        else { col = RGB(200, 200, 200); label = std::to_wstring(dot.runs); }
        FillCircle(hdc, x + 12, y + 10, 12, col);
        DrawTextStr(hdc, x + 7, y + 3, label, RGB(0, 0, 0), 14, true);
        x += 30;
    }
}

void DrawScoreboard(HDC hdc) {
    std::lock_guard<std::mutex> lock(g_state.mtx);

    // Scoreboard panel (right side)
    int sx = 720;
    int sy = 10;
    RECT sbRect = {sx - 5, sy - 5, WIN_W - 5, 660};
    HBRUSH sbBr = CreateSolidBrush(ScoreboardBg());
    FillRect(hdc, &sbRect, sbBr);
    DeleteObject(sbBr);

    // Border
    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 180));
    SelectObject(hdc, borderPen);
    SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
    Rectangle(hdc, sbRect.left, sbRect.top, sbRect.right, sbRect.bottom);
    DeleteObject(borderPen);

    int y = sy + 5;

    // Match info
    DrawTextStr(hdc, sx + 5, y, g_state.matchFormat, RGB(150, 200, 255), 13, false);
    y += 18;
    DrawTextStr(hdc, sx + 5, y, g_state.stadiumName, RGB(180, 180, 180), 12, false);
    y += 16;
    DrawTextStr(hdc, sx + 5, y, g_state.weatherDesc, RGB(150, 180, 150), 12, false);
    y += 22;

    // Team score
    DrawTextStr(hdc, sx + 5, y, g_state.teamBatting, RGB(255, 255, 100), 18, true);
    y += 22;

    std::wostringstream scoreStr;
    scoreStr << g_state.totalRuns << L"/" << g_state.wickets
             << L"  (" << g_state.oversCompleted;
    if (g_state.ballsInOver > 0) scoreStr << L"." << g_state.ballsInOver;
    scoreStr << L" ov)";
    DrawTextStr(hdc, sx + 5, y, scoreStr.str(), RGB(255, 255, 255), 24, true);
    y += 30;

    std::wostringstream rrStr;
    rrStr << L"RR: " << std::fixed << std::setprecision(2) << g_state.runRate;
    if (g_state.target > 0) {
        int need = g_state.target - g_state.totalRuns;
        if (need > 0) rrStr << L"  Need: " << need;
    }
    DrawTextStr(hdc, sx + 5, y, rrStr.str(), RGB(200, 200, 200), 13, false);
    y += 24;

    // Separator
    HPEN sepPen = CreatePen(PS_SOLID, 1, RGB(80, 80, 120));
    SelectObject(hdc, sepPen);
    MoveToEx(hdc, sx + 5, y, nullptr);
    LineTo(hdc, WIN_W - 15, y);
    DeleteObject(sepPen);
    y += 8;

    // Batsmen
    DrawTextStr(hdc, sx + 5, y, L"BATTING", RGB(150, 200, 255), 12, true);
    y += 16;
    {
        std::wostringstream ss;
        ss << L"* " << g_state.strikerName << L"  " << g_state.strikerRuns
           << L" (" << g_state.strikerBalls << L"b)";
        DrawTextStr(hdc, sx + 5, y, ss.str(), RGB(255, 255, 255), 14, true);
        y += 18;
    }
    {
        std::wostringstream ss;
        ss << L"  " << g_state.nonStrikerName << L"  " << g_state.nonStrikerRuns;
        DrawTextStr(hdc, sx + 5, y, ss.str(), RGB(200, 200, 200), 13, false);
        y += 22;
    }

    // Bowler
    DrawTextStr(hdc, sx + 5, y, L"BOWLING", RGB(150, 200, 255), 12, true);
    y += 16;
    {
        std::wostringstream ss;
        ss << g_state.bowlerName << L"  " << g_state.bowlerWickets << L"/"
           << g_state.bowlerRuns << L" (" << g_state.bowlerOvers << L" ov)";
        DrawTextStr(hdc, sx + 5, y, ss.str(), RGB(255, 255, 255), 14, false);
        y += 26;
    }

    // Separator
    sepPen = CreatePen(PS_SOLID, 1, RGB(80, 80, 120));
    SelectObject(hdc, sepPen);
    MoveToEx(hdc, sx + 5, y, nullptr);
    LineTo(hdc, WIN_W - 15, y);
    DeleteObject(sepPen);
    y += 8;

    // Commentary
    DrawTextStr(hdc, sx + 5, y, L"COMMENTARY", RGB(150, 200, 255), 12, true);
    y += 16;
    for (auto& line : g_state.commentary) {
        DrawTextStr(hdc, sx + 5, y, line, RGB(200, 200, 200), 12, false);
        y += 15;
        if (y > 640) break;
    }

    // Result
    if (g_state.matchEnded && !g_state.resultText.empty()) {
        DrawTextStr(hdc, FIELD_CX - 150, FIELD_CY - 20, g_state.resultText,
                    RGB(255, 255, 50), 22, true);
    }
}

void Render(HDC hdc) {
    DrawField(hdc);
    DrawBallTrajectory(hdc);
    DrawScoreboard(hdc);
    DrawThisOverDots(hdc);
}

// --- Ball animation target calculation ---
void CalcBallEndpoint(int runs, bool isWicket, bool isSix, float& ex, float& ey) {
    // Bowler end = top of pitch, batsman = bottom
    float batsmanX = (float)FIELD_CX + 10;
    float batsmanY = (float)(FIELD_CY + PITCH_H / 2 - 8);

    if (isWicket) {
        // Ball hits stumps
        ex = (float)FIELD_CX;
        ey = batsmanY;
        return;
    }
    if (runs == 0) {
        // Dot ball - ball goes to keeper
        ex = (float)FIELD_CX;
        ey = batsmanY + 30;
        return;
    }

    // Random angle based on runs
    float angle;
    float dist;
    srand((unsigned)GetTickCount());
    if (isSix) {
        angle = -PI_F / 2 + ((rand() % 100) / 100.0f) * PI_F;
        dist = (float)FIELD_R * 0.95f;
    } else if (runs == 4) {
        angle = -PI_F / 2 + ((rand() % 100) / 100.0f) * PI_F;
        dist = (float)FIELD_R * 0.85f;
    } else {
        angle = -PI_F / 2 + ((rand() % 100) / 100.0f) * PI_F;
        dist = (float)FIELD_R * (0.2f + runs * 0.15f);
    }
    ex = FIELD_CX + sinf(angle) * dist;
    ey = FIELD_CY - cosf(angle) * dist * 0.85f;
}

// --- Match simulation thread ---
void SimulationThread() {
    // Auto-select: ODI, England home, Lord's
    auto england = TeamDatabase::CreateEngland();
    auto australia = TeamDatabase::CreateAustralia();

    // Use ODI for shorter visual demo
    MatchFormat format = MatchFormat::ODI;
    Stadium stadium = TeamDatabase::GetStadium("Lord's");

    {
        std::lock_guard<std::mutex> lock(g_state.mtx);
        g_state.matchFormat = L"ODI - 50 Overs";
        g_state.stadiumName = ToWide(stadium.name + ", " + stadium.city);
        g_state.matchStarted = true;
    }

    MatchEngine engine;
    engine.SetupMatch(england, australia, stadium, format);

    // Hook into the engine's commentary
    engine.SetCommentaryCallback([&](const std::string& msg) {
        if (msg.empty() || msg[0] == '=' || msg[0] == '-') return;
        std::lock_guard<std::mutex> lock(g_state.mtx);
        g_state.AddCommentary(ToWide(msg));
    });

    // We need to intercept per-delivery for animation.
    // Since MatchEngine runs internally, we'll run it and update state via commentary parsing.
    // Better approach: run the match and update visual state from commentary callback.

    // For visual updates, we'll modify the approach: run the engine in a way that
    // we can extract state after each delivery. The simplest way is to use the
    // commentary callback to parse state and trigger animations.

    // Actually, let's just run the match and parse commentary for visual updates.
    // The engine already outputs ball-by-ball. We'll add delays for animation.

    // Override the commentary to also update visual state
    auto origCallback = [&](const std::string& msg) {
        if (!g_running) return;

        std::wstring wmsg = ToWide(msg);

        // Parse score from messages like "123/4 (20.3 ov)"
        // Look for patterns
        bool hasBallEvent = false;
        int runs = 0;
        bool isFour = false;
        bool isSix = false;
        bool isWicket = false;

        if (msg.find("FOUR!") != std::string::npos) { isFour = true; runs = 4; hasBallEvent = true; }
        else if (msg.find("SIX!") != std::string::npos) { isSix = true; runs = 6; hasBallEvent = true; }
        else if (msg.find("WICKET!") != std::string::npos) { isWicket = true; hasBallEvent = true; }
        else if (msg.find("Wide ball") != std::string::npos) { runs = 1; hasBallEvent = true; }
        else if (msg.find("No ball") != std::string::npos) { runs = 1; hasBallEvent = true; }
        else if (msg.find("run.") != std::string::npos || msg.find("runs.") != std::string::npos) {
            hasBallEvent = true;
            // Extract run count
            for (int i = 1; i <= 3; i++) {
                if (msg.find(std::to_string(i) + " run") != std::string::npos) { runs = i; break; }
            }
        }
        else if (msg.find("dot ball") != std::string::npos) { hasBallEvent = true; runs = 0; }
        else if (msg.find("defends") != std::string::npos) { hasBallEvent = true; runs = 0; }
        else if (msg.find("leaves") != std::string::npos) { hasBallEvent = true; runs = 0; }
        else if (msg.find("plays and misses") != std::string::npos) { hasBallEvent = true; runs = 0; }

        if (hasBallEvent) {
            // Animate ball
            float ex, ey;
            CalcBallEndpoint(runs, isWicket, isSix, ex, ey);

            {
                std::lock_guard<std::mutex> lock(g_state.mtx);
                g_state.ball.startX = (float)FIELD_CX;
                g_state.ball.startY = (float)(FIELD_CY - PITCH_H / 2 - 20);
                g_state.ball.endX = ex;
                g_state.ball.endY = ey;
                g_state.ball.progress = 0.0f;
                g_state.ball.active = true;
                g_state.ball.isFour = isFour;
                g_state.ball.isSix = isSix;
                g_state.ball.isWicket = isWicket;
                g_state.ball.runs = runs;
                g_state.ball.shotDesc = wmsg;

                // Add to this-over dots
                GameVisualState::TrailDot dot;
                dot.x = ex; dot.y = ey; dot.runs = runs; dot.isWicket = isWicket;
                g_state.thisOverDots.push_back(dot);
            }

            // Animate over ~600ms
            for (int frame = 0; frame < 20 && g_running; frame++) {
                {
                    std::lock_guard<std::mutex> lock(g_state.mtx);
                    g_state.ball.progress = (frame + 1) / 20.0f;
                }
                InvalidateRect(g_hwnd, nullptr, FALSE);
                Sleep(30);
            }

            // Hold result for a moment
            Sleep(isFour || isSix || isWicket ? 500 : 200);

            {
                std::lock_guard<std::mutex> lock(g_state.mtx);
                g_state.ball.active = false;
            }
        }

        // Check for over completion (clear dots)
        if (msg.find("After ") != std::string::npos && msg.find("overs:") != std::string::npos) {
            std::lock_guard<std::mutex> lock(g_state.mtx);
            g_state.thisOverDots.clear();
        }

        // Parse score: look for "X/Y (Z.W ov)"
        {
            size_t slashPos = msg.find('/');
            size_t ovPos = msg.find(" ov)");
            if (slashPos != std::string::npos && ovPos != std::string::npos) {
                // Try to extract score
                try {
                    // Find the score portion - look backwards from slash for digits
                    size_t scoreStart = slashPos;
                    while (scoreStart > 0 && (isdigit(msg[scoreStart-1]))) scoreStart--;
                    std::string runsStr = msg.substr(scoreStart, slashPos - scoreStart);
                    // Wickets
                    size_t wickEnd = slashPos + 1;
                    while (wickEnd < msg.size() && isdigit(msg[wickEnd])) wickEnd++;
                    std::string wickStr = msg.substr(slashPos + 1, wickEnd - slashPos - 1);

                    if (!runsStr.empty() && !wickStr.empty()) {
                        std::lock_guard<std::mutex> lock(g_state.mtx);
                        g_state.totalRuns = std::stoi(runsStr);
                        g_state.wickets = std::stoi(wickStr);
                    }
                } catch (...) {}
            }
        }

        // Update commentary
        if (!msg.empty() && msg.find("===") == std::string::npos) {
            std::lock_guard<std::mutex> lock(g_state.mtx);
            g_state.AddCommentary(wmsg);
        }

        InvalidateRect(g_hwnd, nullptr, FALSE);
    };

    engine.SetCommentaryCallback(origCallback);

    // Update weather
    {
        std::lock_guard<std::mutex> lock(g_state.mtx);
        g_state.teamBatting = L"England";
        g_state.teamBowling = L"Australia";
        g_state.weatherDesc = ToWide("Overcast, humid conditions");
        g_state.strikerName = ToWide(england.players[0].name);
        g_state.nonStrikerName = ToWide(england.players[1].name);
        g_state.bowlerName = ToWide(australia.players[7].name);
    }

    engine.SimulateMatch();

    {
        std::lock_guard<std::mutex> lock(g_state.mtx);
        g_state.matchEnded = true;
        if (g_state.commentary.size() > 0) {
            g_state.resultText = g_state.commentary.back();
        }
    }
    g_simDone = true;
    InvalidateRect(g_hwnd, nullptr, FALSE);
}

// --- Win32 Window Proc ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HDC hdc = GetDC(hwnd);
        g_backDC = CreateCompatibleDC(hdc);
        g_backbuffer = CreateCompatibleBitmap(hdc, WIN_W, WIN_H);
        SelectObject(g_backDC, g_backbuffer);
        ReleaseDC(hwnd, hdc);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Render(g_backDC);
        BitBlt(hdc, 0, 0, WIN_W, WIN_H, g_backDC, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            g_running = false;
            PostQuitMessage(0);
        }
        return 0;
    case WM_DESTROY:
        g_running = false;
        if (g_backbuffer) DeleteObject(g_backbuffer);
        if (g_backDC) DeleteDC(g_backDC);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// --- Entry Point ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"CricketGameWindow";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    RECT wr = {0, 0, WIN_W, WIN_H};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);

    g_hwnd = CreateWindowExW(0, L"CricketGameWindow",
        L"Cricket Game - England vs Australia",
        (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!g_hwnd) return 1;

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // Start simulation in background thread
    std::thread simThread(SimulationThread);
    simThread.detach();

    // Message loop with periodic refresh
    MSG message;
    while (g_running) {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                g_running = false;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        Sleep(16); // ~60fps
        InvalidateRect(g_hwnd, nullptr, FALSE);
    }

    return 0;
}
