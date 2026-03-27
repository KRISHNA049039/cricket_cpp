#include "MatchEngine.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <chrono>

MatchEngine::MatchEngine() {
    auto seed = static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    rng.seed(seed);
}

void MatchEngine::SetupMatch(const Team& home, const Team& away,
                              const Stadium& stad, MatchFormat fmt) {
    format = fmt;
    teams[0] = home;
    teams[1] = away;
    stadium = stad;
    totalInnings = (format == MatchFormat::Test) ? 4 : 2;
    maxOvers = (format == MatchFormat::Test) ? 90 : 50;

    for (int i = 0; i < 4; i++) innings[i] = InningsScore{};
    ball[0] = BallCondition{};
    ball[1] = BallCondition{};

    InitWeather();
    InitPitch();
}

void MatchEngine::InitWeather() {
    weather.type = stadium.typicalWeather;
    weather.humidity = stadium.typicalHumidity + RandFloat(-0.1f, 0.1f);
    weather.humidity = std::clamp(weather.humidity, 0.0f, 1.0f);
    weather.windSpeed = stadium.typicalWindSpeed + RandFloat(-3.0f, 3.0f);
    weather.temperature = stadium.typicalTemp + RandFloat(-3.0f, 3.0f);

    switch (weather.type) {
        case WeatherType::Sunny:    weather.description = "Bright sunshine"; break;
        case WeatherType::Overcast: weather.description = "Overcast skies, good for swing"; break;
        case WeatherType::Cloudy:   weather.description = "Cloudy conditions"; break;
        case WeatherType::Drizzle:  weather.description = "Light drizzle"; break;
        case WeatherType::Humid:    weather.description = "Hot and humid"; break;
    }
}

void MatchEngine::InitPitch() {
    pitch.hardness = stadium.pitchHardness + RandFloat(-0.05f, 0.05f);
    pitch.grassCoverage = stadium.grassCoverage + RandFloat(-0.05f, 0.05f);
    pitch.moisture = stadium.baseMoisture + RandFloat(-0.05f, 0.05f);
    pitch.deterioration = 0.0f;
    pitch.hardness = std::clamp(pitch.hardness, 0.0f, 1.0f);
    pitch.grassCoverage = std::clamp(pitch.grassCoverage, 0.0f, 1.0f);
    pitch.moisture = std::clamp(pitch.moisture, 0.0f, 1.0f);
}

void MatchEngine::SimulateMatch() {
    Say("========================================");
    Say("  " + teams[0].name + " vs " + teams[1].name);
    Say("  " + stadium.name + ", " + stadium.city);
    Say("  Format: " + std::string(format == MatchFormat::Test ? "TEST MATCH" : "ODI"));
    Say("  Weather: " + weather.description);
    std::ostringstream ws;
    ws << "  Humidity: " << std::fixed << std::setprecision(0)
       << (weather.humidity * 100) << "%, Wind: " << weather.windSpeed << " km/h";
    Say(ws.str());
    Say("========================================\n");

    // Toss
    battingTeamIdx = RandInt(0, 1);
    bowlingTeamIdx = 1 - battingTeamIdx;
    Say(teams[battingTeamIdx].name + " win the toss and elect to bat.\n");

    for (currentInnings = 0; currentInnings < totalInnings; currentInnings++) {
        if (currentInnings == 1) {
            std::swap(battingTeamIdx, bowlingTeamIdx);
        }
        if (format == MatchFormat::Test && currentInnings == 2) {
            // 2nd innings: team that batted first bats again (simplified)
            std::swap(battingTeamIdx, bowlingTeamIdx);
        }
        if (format == MatchFormat::Test && currentInnings == 3) {
            std::swap(battingTeamIdx, bowlingTeamIdx);
        }

        // Set target for final innings
        if (format == MatchFormat::ODI && currentInnings == 1) {
            target = innings[0].totalRuns + 1;
        }
        if (format == MatchFormat::Test && currentInnings == 3) {
            // The team batting 4th needs to chase
            target = (innings[0].totalRuns + innings[2].totalRuns)
                   - innings[1].totalRuns + 1;
        }

        Say("--- " + teams[battingTeamIdx].name + " Innings "
            + std::to_string((currentInnings / 2) + 1) + " ---\n");

        // Reset for new innings
        ball[0].TakeNewBall();
        ball[1].TakeNewBall();
        currentBallEnd = 0;
        strikerIdx = 0;
        nonStrikerIdx = 1;
        nextBatIdx = 2;
        lastBowlerIdx = -1;

        // Init batting card
        auto& score = innings[currentInnings];
        auto& batTeam = teams[battingTeamIdx];
        for (auto& p : batTeam.players) {
            BatsmanInnings bi;
            bi.name = p.name;
            score.battingCard.push_back(bi);
        }
        // Init bowling card
        auto& bowlTeam = teams[bowlingTeamIdx];
        for (auto& p : bowlTeam.players) {
            if (p.bowlingSkill >= 40) {
                BowlerFigures bf;
                bf.name = p.name;
                score.bowlingCard.push_back(bf);
            }
        }

        SimulateInnings();

        Say("\n" + teams[battingTeamIdx].name + ": " + score.GetScoreString());
        PrintInningsScorecard(currentInnings);
        Say("");

        // Pitch deteriorates between innings (Test)
        if (format == MatchFormat::Test) {
            pitch.Deteriorate(0.08f);
        }

        // Check if match is decided
        if (format == MatchFormat::ODI && currentInnings == 1) {
            if (innings[1].totalRuns >= target) {
                Say(teams[battingTeamIdx].name + " win by "
                    + std::to_string(10 - innings[1].wickets) + " wickets!");
                break;
            } else {
                Say(teams[bowlingTeamIdx].name + " win by "
                    + std::to_string(target - 1 - innings[1].totalRuns) + " runs!");
                break;
            }
        }
    }

    if (format == MatchFormat::Test) {
        int team0 = innings[0].totalRuns + innings[2].totalRuns;
        int team1 = innings[1].totalRuns + innings[3].totalRuns;
        Say("\n========== MATCH RESULT ==========");
        if (team0 > team1)
            Say(teams[0].name + " win by " + std::to_string(team0 - team1) + " runs!");
        else if (team1 > team0)
            Say(teams[1].name + " win by " + std::to_string(team1 - team0) + " runs!");
        else
            Say("Match drawn!");
    }

    Say("\n========== FULL SCORECARD ==========");
    for (int i = 0; i < totalInnings; i++) {
        if (innings[i].battingCard.empty()) continue;
        PrintInningsScorecard(i);
    }
}

void MatchEngine::SimulateInnings() {
    auto& score = innings[currentInnings];

    while (!IsInningsOver()) {
        SimulateOver();
        if (IsInningsOver()) break;

        // Handle new ball in Test after 80 overs
        if (format == MatchFormat::Test) {
            HandleNewBallTest();
        }

        // ODI: ball wears from both ends, new balls after 40 overs not needed
        // but the 2 balls wear out differently
        HandleBallWear();

        // Pitch deterioration per over
        if (format == MatchFormat::Test) {
            pitch.Deteriorate(0.001f);
        }
    }
}

void MatchEngine::SimulateOver() {
    auto& score = innings[currentInnings];
    SelectBowler();

    int runsThisOver = 0;
    int ballsThisOver = 0;

    auto& bowlTeam = teams[bowlingTeamIdx];
    auto& batTeam = teams[battingTeamIdx];

    // Find bowler figures
    BowlerFigures* bowlerFig = nullptr;
    for (auto& bf : score.bowlingCard) {
        if (bf.name == bowlTeam.players[currentBowlerIdx].name) {
            bowlerFig = &bf;
            break;
        }
    }

    while (ballsThisOver < 6 && !IsInningsOver()) {
        auto& bowler = bowlTeam.players[currentBowlerIdx];
        auto& batsman = batTeam.players[strikerIdx];

        auto result = SimulateDelivery(bowler, batsman);

        // Update ball condition
        ball[currentBallEnd].AgeBall();

        if (result.isWide) {
            score.totalRuns++;
            score.extras++;
            score.wides++;
            if (bowlerFig) bowlerFig->runsConceded++;
            Say("  Wide ball! " + score.GetScoreString());
            continue; // doesn't count as a ball
        }
        if (result.isNoBall) {
            score.totalRuns++;
            score.extras++;
            score.noBalls++;
            if (bowlerFig) bowlerFig->runsConceded++;
            Say("  No ball! Free hit next delivery.");
            // Still count runs off the bat
        }

        ballsThisOver++;
        score.ballsInOver = ballsThisOver;

        // Update batsman
        auto& batsmanInnings = score.battingCard[strikerIdx];
        batsmanInnings.ballsFaced++;

        if (result.isWicket && !result.isNoBall) {
            batsmanInnings.runs += result.runs;
            score.totalRuns += result.runs;
            if (bowlerFig) bowlerFig->runsConceded += result.runs;

            batsmanInnings.isOut = true;
            batsmanInnings.howOut = result.dismissal;
            batsmanInnings.dismissedBy = bowler.name;
            score.wickets++;

            std::ostringstream fow;
            fow << score.wickets << "-" << score.totalRuns
                << " (" << batsman.name << ", "
                << score.oversCompleted << "." << ballsThisOver << " ov)";
            score.fallOfWickets.push_back(fow.str());

            Say("  WICKET! " + batsman.name + " " + result.description
                + " " + score.GetScoreString());

            if (bowlerFig) bowlerFig->wickets++;

            // Next batsman
            if (nextBatIdx < (int)batTeam.players.size()) {
                strikerIdx = nextBatIdx++;
            } else {
                break; // all out
            }
        } else {
            score.totalRuns += result.runs;
            batsmanInnings.runs += result.runs;
            if (bowlerFig) bowlerFig->runsConceded += result.runs;

            if (result.isFour) {
                batsmanInnings.fours++;
                Say("  FOUR! " + result.description + " " + score.GetScoreString());
            } else if (result.isSix) {
                batsmanInnings.sixes++;
                Say("  SIX! " + result.description + " " + score.GetScoreString());
            } else if (result.runs > 0) {
                // Only print occasionally to avoid spam
                if (result.runs >= 2 || RandFloat() < 0.3f) {
                    Say("  " + std::to_string(result.runs) + " run"
                        + (result.runs > 1 ? "s" : "") + ". " + result.description);
                }
            }

            runsThisOver += result.runs;
            if (result.runs % 2 == 1) SwapStrike();
        }

        // Check chase target
        if (target > 0 && score.totalRuns >= target) {
            break;
        }
    }

    // Over complete
    if (ballsThisOver == 6) {
        score.oversCompleted++;
        score.ballsInOver = 0;
        if (bowlerFig) {
            bowlerFig->overs++;
            bowlerFig->ballsInCurrentOver = 0;
            if (runsThisOver == 0) bowlerFig->maidens++;
        }
        SwapStrike(); // change ends

        // In ODI, alternate ball ends each over
        if (format == MatchFormat::ODI) {
            currentBallEnd = 1 - currentBallEnd;
        }

        // Print over summary periodically
        if (score.oversCompleted % 5 == 0 || score.oversCompleted == maxOvers) {
            std::ostringstream ss;
            ss << "\n  After " << score.oversCompleted << " overs: "
               << score.GetScoreString() << " (RR: "
               << std::fixed << std::setprecision(2) << score.GetRunRate() << ")";
            if (target > 0) {
                int needed = target - score.totalRuns;
                float reqRR = 0;
                float oversLeft = maxOvers - score.GetOversFloat();
                if (oversLeft > 0) reqRR = needed / oversLeft;
                ss << " Need " << needed << " from "
                   << std::setprecision(1) << oversLeft << " ov (RRR: "
                   << std::setprecision(2) << reqRR << ")";
            }
            Say(ss.str());

            // Ball condition report
            if (format == MatchFormat::Test && score.oversCompleted % 20 == 0) {
                std::ostringstream bs;
                bs << "  Ball condition: shine=" << std::setprecision(0)
                   << (ball[0].shine * 100) << "%, age=" << ball[0].ballAge
                   << " deliveries";
                if (ball[0].isReversing) bs << " (REVERSING)";
                Say(bs.str());
            }
            if (format == MatchFormat::ODI && score.oversCompleted % 10 == 0) {
                for (int e = 0; e < 2; e++) {
                    std::ostringstream bs;
                    bs << "  Ball (end " << (e+1) << "): shine="
                       << std::setprecision(0) << (ball[e].shine * 100)
                       << "%, age=" << ball[e].ballAge;
                    if (ball[e].isReversing) bs << " (REVERSING)";
                    Say(bs.str());
                }
            }
            Say("");
        }
    } else if (bowlerFig) {
        bowlerFig->ballsInCurrentOver = ballsThisOver;
    }

    lastBowlerIdx = currentBowlerIdx;
}

MatchEngine::DeliveryResult MatchEngine::SimulateDelivery(
    const PlayerStats& bowler, const PlayerStats& batsman) {

    DeliveryResult result;
    DeliveryType delivery = ChooseDelivery(bowler);

    // Calculate ball movement factors
    float swingAmount = CalculateSwing(bowler);
    float seamAmount = CalculateSeam(bowler);
    float spinAmount = CalculateSpin(bowler);
    float bounceVar = CalculateBounce();

    // Total difficulty for batsman (0-1)
    float difficulty = 0.0f;
    difficulty += swingAmount * 0.25f;
    difficulty += seamAmount * 0.2f;
    difficulty += spinAmount * 0.2f;
    difficulty += bounceVar * 0.15f;
    difficulty += (bowler.bowlingSkill / 100.0f) * 0.2f;

    // Delivery type modifiers
    if (delivery == DeliveryType::Yorker) difficulty += 0.15f;
    if (delivery == DeliveryType::Bouncer) difficulty += 0.1f;
    if (delivery == DeliveryType::Short) difficulty -= 0.05f;

    // Test match: ball does more in first 40-50 overs (new ball + conditions)
    auto& score = innings[currentInnings];
    if (format == MatchFormat::Test) {
        if (score.oversCompleted < 20) {
            difficulty += 0.12f; // new ball, lots of movement
        } else if (score.oversCompleted < 40) {
            difficulty += 0.06f; // still some assistance
        } else if (score.oversCompleted < 50) {
            difficulty += 0.03f; // ball getting older
        }
        // Old ball with reverse swing
        if (ball[0].isReversing) difficulty += 0.08f;
    }

    // ODI: both balls wear out after 40 overs
    if (format == MatchFormat::ODI) {
        if (score.oversCompleted >= 40) {
            // Balls are worn, less movement but reverse possible
            difficulty -= 0.05f;
            if (ball[currentBallEnd].isReversing) difficulty += 0.1f;
        }
        if (score.oversCompleted < 10) {
            difficulty += 0.08f; // powerplay, new ball
        }
    }

    // Weather effect on difficulty
    difficulty *= weather.GetSwingFactor() * 0.5f + 0.5f;

    difficulty = std::clamp(difficulty, 0.05f, 0.85f);

    // Check for wide/no-ball
    float wideChance = 0.03f - (bowler.bowlingSkill / 100.0f) * 0.02f;
    float noBallChance = 0.015f;
    if (RandFloat() < wideChance) {
        result.isWide = true;
        result.description = "strays down leg side, called wide.";
        return result;
    }
    if (RandFloat() < noBallChance) {
        result.isNoBall = true;
    }

    // Batsman chooses shot
    ShotType shot = ChooseShot(batsman, difficulty);
    result.shot = shot;

    // Check for dismissal
    DismissalType dismissal = CheckDismissal(batsman, difficulty, shot, bowler);
    if (dismissal != DismissalType::NotOut) {
        result.isWicket = true;
        result.dismissal = dismissal;
        switch (dismissal) {
            case DismissalType::Bowled:
                result.description = "b " + bowler.name;
                break;
            case DismissalType::Caught:
                result.description = "c sub b " + bowler.name;
                break;
            case DismissalType::CaughtBehind:
                result.description = "c †wk b " + bowler.name;
                break;
            case DismissalType::LBW:
                result.description = "lbw b " + bowler.name;
                break;
            case DismissalType::CaughtAndBowled:
                result.description = "c&b " + bowler.name;
                break;
            case DismissalType::RunOut:
                result.description = "run out";
                break;
            default:
                result.description = "dismissed by " + bowler.name;
                break;
        }
        return result;
    }

    // Calculate runs
    float batPower = CalculateBatPower(batsman, shot);
    float contactQuality = RandFloat();

    // Shot outcome based on power and contact
    float shotValue = batPower * contactQuality;

    if (shot == ShotType::LeaveAlone || shot == ShotType::PlayAndMiss) {
        result.runs = 0;
        result.description = (shot == ShotType::LeaveAlone)
            ? batsman.name + " leaves well alone."
            : batsman.name + " plays and misses outside off.";
    } else if (shot == ShotType::Defensive) {
        if (shotValue > 0.8f) {
            result.runs = RandInt(1, 2);
            result.description = batsman.name + " pushes into the gap for " + std::to_string(result.runs) + ".";
        } else {
            result.runs = 0;
            result.description = batsman.name + " defends solidly.";
        }
    } else if (shot == ShotType::EdgeBehind) {
        result.runs = RandInt(0, 4);
        if (result.runs == 4) {
            result.isFour = true;
            result.description = batsman.name + " edges through the slips for FOUR!";
        } else {
            result.description = batsman.name + " gets an edge, " + std::to_string(result.runs) + " runs.";
        }
    } else {
        // Attacking shots
        if (shotValue > 0.92f && (shot == ShotType::LoftedDrive || shot == ShotType::Pull
                                   || shot == ShotType::UpperCut)) {
            result.runs = 6;
            result.isSix = true;
            std::string shotName;
            switch (shot) {
                case ShotType::LoftedDrive: shotName = "lofted drive"; break;
                case ShotType::Pull: shotName = "pull shot"; break;
                case ShotType::UpperCut: shotName = "upper cut"; break;
                default: shotName = "big shot"; break;
            }
            result.description = batsman.name + " launches a magnificent " + shotName + " for SIX!";
        } else if (shotValue > 0.7f) {
            result.runs = 4;
            result.isFour = true;
            std::string shotName;
            switch (shot) {
                case ShotType::Drive: shotName = "drives through covers"; break;
                case ShotType::Cut: shotName = "cuts hard past point"; break;
                case ShotType::Pull: shotName = "pulls to the boundary"; break;
                case ShotType::Sweep: shotName = "sweeps fine"; break;
                case ShotType::Flick: shotName = "flicks off the pads"; break;
                case ShotType::LoftedDrive: shotName = "lofts over mid-off"; break;
                default: shotName = "finds the boundary"; break;
            }
            result.description = batsman.name + " " + shotName + " for FOUR!";
        } else if (shotValue > 0.45f) {
            result.runs = RandInt(1, 3);
            result.description = batsman.name + " works it for " + std::to_string(result.runs) + ".";
        } else if (shotValue > 0.2f) {
            result.runs = 1;
            result.description = batsman.name + " nudges for a single.";
        } else {
            result.runs = 0;
            result.description = batsman.name + " can't get it away, dot ball.";
        }
    }

    // Boundary size affects fours/sixes
    if (result.isFour && stadium.boundarySize > 72.0f && RandFloat() < 0.15f) {
        result.runs = 3;
        result.isFour = false;
        result.description = batsman.name + " doesn't quite reach the boundary, 3 runs.";
    }

    return result;
}

float MatchEngine::CalculateSwing(const PlayerStats& bowler) {
    float base = 0.0f;
    if (bowler.bowlType == BowlingType::Fast || bowler.bowlType == BowlingType::MediumFast) {
        base = (bowler.bowlingSkill / 100.0f) * 0.4f;
    } else if (bowler.bowlType == BowlingType::Medium) {
        base = (bowler.bowlingSkill / 100.0f) * 0.25f;
    }

    // Ball condition affects swing
    auto& bc = ball[currentBallEnd];
    if (bc.shine > 0.5f) {
        base *= (1.0f + bc.shine * 0.3f); // conventional swing
    } else if (bc.isReversing) {
        base *= 1.4f; // reverse swing
    } else {
        base *= 0.5f; // old ball, no swing
    }

    // Weather
    base *= weather.GetSwingFactor();

    return std::clamp(base, 0.0f, 1.0f);
}

float MatchEngine::CalculateSeam(const PlayerStats& bowler) {
    float base = 0.0f;
    if (bowler.bowlType == BowlingType::Fast || bowler.bowlType == BowlingType::MediumFast) {
        base = (bowler.bowlingSkill / 100.0f) * 0.35f;
    }
    base *= (1.0f + pitch.grassCoverage * 0.5f);
    base *= (1.0f + pitch.moisture * 0.3f);
    base *= ball[currentBallEnd].seamIntegrity;
    return std::clamp(base, 0.0f, 1.0f);
}

float MatchEngine::CalculateSpin(const PlayerStats& bowler) {
    float base = 0.0f;
    if (bowler.bowlType == BowlingType::OffSpin || bowler.bowlType == BowlingType::LegSpin) {
        base = (bowler.bowlingSkill / 100.0f) * 0.5f;
    }
    // Pitch deterioration helps spin
    base *= (1.0f + pitch.deterioration * 1.5f);
    // Dry pitch helps spin
    base *= (1.0f + (1.0f - pitch.moisture) * 0.3f);
    base *= weather.GetSpinFactor();
    return std::clamp(base, 0.0f, 1.0f);
}

float MatchEngine::CalculateBounce() {
    float base = pitch.hardness * 0.5f;
    base += RandFloat(-0.1f, 0.1f);
    // Variable bounce on deteriorated pitch
    base += pitch.deterioration * RandFloat(0.0f, 0.3f);
    return std::clamp(base, 0.0f, 1.0f);
}

float MatchEngine::CalculateBatPower(const PlayerStats& batsman, ShotType shot) {
    float base = batsman.battingSkill / 100.0f;

    switch (shot) {
        case ShotType::LoftedDrive:
        case ShotType::Pull:
        case ShotType::UpperCut:
            base *= (0.6f + batsman.powerHitting * 0.4f);
            break;
        case ShotType::Drive:
        case ShotType::Cut:
            base *= (0.5f + batsman.technique * 0.3f + batsman.powerHitting * 0.2f);
            break;
        case ShotType::Defensive:
            base *= (0.3f + batsman.technique * 0.5f);
            break;
        case ShotType::Sweep:
        case ShotType::Flick:
            base *= (0.5f + batsman.technique * 0.3f);
            break;
        default:
            base *= 0.5f;
            break;
    }
    return std::clamp(base, 0.0f, 1.0f);
}

ShotType MatchEngine::ChooseShot(const PlayerStats& batsman, float difficulty) {
    auto& score = innings[currentInnings];
    InningsPhase phase = GetPhase();

    // High technique batsmen leave/defend more in tough conditions
    if (difficulty > 0.5f && batsman.technique > 0.8f && RandFloat() < 0.3f) {
        return ShotType::LeaveAlone;
    }
    if (difficulty > 0.6f && RandFloat() < 0.2f) {
        return ShotType::PlayAndMiss;
    }

    // Temperament affects shot selection
    float aggression = 1.0f - batsman.temperament;
    if (target > 0) {
        float needed = target - score.totalRuns;
        float oversLeft = maxOvers - score.GetOversFloat();
        if (oversLeft > 0) {
            float reqRR = needed / oversLeft;
            if (reqRR > 8.0f) aggression += 0.3f;
            else if (reqRR > 6.0f) aggression += 0.15f;
        }
    }

    // Death overs in ODI = more aggressive
    if (format == MatchFormat::ODI && phase == InningsPhase::Death) {
        aggression += 0.25f;
    }

    aggression = std::clamp(aggression, 0.0f, 1.0f);

    float roll = RandFloat();
    if (roll < aggression * 0.15f) return ShotType::LoftedDrive;
    if (roll < aggression * 0.25f) return ShotType::Pull;
    if (roll < aggression * 0.35f) return ShotType::Cut;
    if (roll < aggression * 0.5f) return ShotType::Drive;
    if (roll < aggression * 0.6f) return ShotType::Flick;
    if (roll < aggression * 0.65f) return ShotType::Sweep;
    if (roll < 0.8f) return ShotType::Defensive;
    return ShotType::LeaveAlone;
}

DeliveryType MatchEngine::ChooseDelivery(const PlayerStats& bowler) {
    float roll = RandFloat();
    auto phase = GetPhase();

    if (bowler.bowlType == BowlingType::Fast || bowler.bowlType == BowlingType::MediumFast) {
        if (phase == InningsPhase::Death && RandFloat() < 0.3f) return DeliveryType::Yorker;
        if (roll < 0.5f) return DeliveryType::Good;
        if (roll < 0.7f) return DeliveryType::Short;
        if (roll < 0.85f) return DeliveryType::Full;
        if (roll < 0.92f) return DeliveryType::Bouncer;
        return DeliveryType::Yorker;
    } else {
        // Spinners
        if (roll < 0.6f) return DeliveryType::Good;
        if (roll < 0.8f) return DeliveryType::Full;
        if (roll < 0.95f) return DeliveryType::Short;
        return DeliveryType::Yorker;
    }
}

DismissalType MatchEngine::CheckDismissal(const PlayerStats& batsman, float difficulty,
                                            ShotType shot, const PlayerStats& bowler) {
    float dismissalChance = difficulty * 0.08f;

    // Batsman skill reduces chance
    dismissalChance *= (1.0f - batsman.battingSkill / 150.0f);

    // Shot-specific risks
    if (shot == ShotType::LoftedDrive) dismissalChance *= 2.5f;
    if (shot == ShotType::Pull) dismissalChance *= 1.8f;
    if (shot == ShotType::UpperCut) dismissalChance *= 2.0f;
    if (shot == ShotType::Defensive) dismissalChance *= 0.5f;
    if (shot == ShotType::LeaveAlone) dismissalChance *= 0.15f;
    if (shot == ShotType::PlayAndMiss) dismissalChance *= 0.3f;

    // Technique helps survival
    dismissalChance *= (1.0f - batsman.technique * 0.3f);

    // New batsman more vulnerable
    auto& batsmanInnings = innings[currentInnings].battingCard[strikerIdx];
    if (batsmanInnings.ballsFaced < 10) dismissalChance *= 1.5f;
    if (batsmanInnings.ballsFaced < 3) dismissalChance *= 1.3f;

    dismissalChance = std::clamp(dismissalChance, 0.005f, 0.15f);

    if (RandFloat() > dismissalChance) return DismissalType::NotOut;

    // Determine type of dismissal
    float roll = RandFloat();
    bool isFastBowler = (bowler.bowlType == BowlingType::Fast ||
                         bowler.bowlType == BowlingType::MediumFast);

    if (isFastBowler) {
        if (roll < 0.2f) return DismissalType::Bowled;
        if (roll < 0.5f) return DismissalType::Caught;
        if (roll < 0.65f) return DismissalType::CaughtBehind;
        if (roll < 0.8f) return DismissalType::LBW;
        if (roll < 0.9f) return DismissalType::CaughtAndBowled;
        return DismissalType::RunOut;
    } else {
        // Spinners
        if (roll < 0.25f) return DismissalType::Bowled;
        if (roll < 0.5f) return DismissalType::Caught;
        if (roll < 0.65f) return DismissalType::LBW;
        if (roll < 0.8f) return DismissalType::Stumped;
        if (roll < 0.9f) return DismissalType::CaughtAndBowled;
        return DismissalType::RunOut;
    }
}

void MatchEngine::HandleBallWear() {
    // ODI: 2 balls used from both ends, they both wear independently
    // After 40 overs both balls are significantly worn
    if (format == MatchFormat::ODI) {
        auto& score = innings[currentInnings];
        // Each ball gets used every other over, so ~25 overs each by over 50
        // The wear is already handled per delivery in AgeBall()
        // After 40 overs, report wear status
    }
}

void MatchEngine::HandleNewBallTest() {
    auto& score = innings[currentInnings];
    // New ball available after 80 overs in Test cricket
    if (score.oversCompleted == 80 && score.ballsInOver == 0) {
        ball[0].TakeNewBall();
        Say("\n  *** NEW BALL TAKEN! Over 80 reached. ***\n");
    }
}

void MatchEngine::SelectBowler() {
    auto& bowlTeam = teams[bowlingTeamIdx];
    auto& score = innings[currentInnings];

    // Build list of eligible bowlers (skill >= 40, not last over's bowler)
    std::vector<int> eligible;
    for (int i = 0; i < (int)bowlTeam.players.size(); i++) {
        if (bowlTeam.players[i].bowlingSkill >= 40 && i != lastBowlerIdx) {
            // In ODI, max 10 overs per bowler
            if (format == MatchFormat::ODI) {
                int oversUsed = 0;
                for (auto& bf : score.bowlingCard) {
                    if (bf.name == bowlTeam.players[i].name) {
                        oversUsed = bf.overs;
                        break;
                    }
                }
                if (oversUsed >= 10) continue;
            }
            eligible.push_back(i);
        }
    }

    if (eligible.empty()) {
        // Fallback: anyone who can bowl
        for (int i = 0; i < (int)bowlTeam.players.size(); i++) {
            if (bowlTeam.players[i].bowlingSkill >= 20 && i != lastBowlerIdx)
                eligible.push_back(i);
        }
    }

    if (eligible.empty()) {
        currentBowlerIdx = (lastBowlerIdx + 1) % bowlTeam.players.size();
        return;
    }

    // Prefer fast bowlers early, spinners later
    InningsPhase phase = GetPhase();
    if (phase == InningsPhase::Early) {
        // Prefer pace
        std::vector<int> pacers;
        for (int i : eligible) {
            if (bowlTeam.players[i].bowlType == BowlingType::Fast ||
                bowlTeam.players[i].bowlType == BowlingType::MediumFast)
                pacers.push_back(i);
        }
        if (!pacers.empty()) {
            currentBowlerIdx = pacers[RandInt(0, pacers.size() - 1)];
            return;
        }
    } else if (phase == InningsPhase::Middle || phase == InningsPhase::Late) {
        // Mix of pace and spin, slight preference for spinners on deteriorated pitch
        if (pitch.deterioration > 0.3f && RandFloat() < 0.6f) {
            std::vector<int> spinners;
            for (int i : eligible) {
                if (bowlTeam.players[i].bowlType == BowlingType::OffSpin ||
                    bowlTeam.players[i].bowlType == BowlingType::LegSpin)
                    spinners.push_back(i);
            }
            if (!spinners.empty()) {
                currentBowlerIdx = spinners[RandInt(0, spinners.size() - 1)];
                return;
            }
        }
    }

    currentBowlerIdx = eligible[RandInt(0, eligible.size() - 1)];
}

void MatchEngine::SwapStrike() {
    std::swap(strikerIdx, nonStrikerIdx);
}

bool MatchEngine::IsInningsOver() const {
    auto& score = innings[currentInnings];
    if (score.wickets >= 10) return true;
    if (score.oversCompleted >= maxOvers) return true;
    if (target > 0 && score.totalRuns >= target) return true;
    // Test: declare if lead is huge (simplified)
    if (format == MatchFormat::Test && currentInnings >= 2) {
        if (score.totalRuns > 500 && score.oversCompleted > 60) return true;
    }
    return false;
}

float MatchEngine::GetMatchSituation() const {
    auto& score = innings[currentInnings];
    float tension = 0.0f;
    if (target > 0) {
        float needed = target - score.totalRuns;
        float oversLeft = maxOvers - score.GetOversFloat();
        if (oversLeft > 0) {
            float reqRR = needed / oversLeft;
            tension = std::min(1.0f, reqRR / 12.0f);
        }
        tension += score.wickets * 0.08f;
    }
    return std::clamp(tension, 0.0f, 1.0f);
}

InningsPhase MatchEngine::GetPhase() const {
    auto& score = innings[currentInnings];
    float progress = (float)score.oversCompleted / maxOvers;
    if (progress < 0.2f) return InningsPhase::Early;
    if (progress < 0.5f) return InningsPhase::Middle;
    if (progress < 0.8f) return InningsPhase::Late;
    return InningsPhase::Death;
}

void MatchEngine::PrintScorecard() const {
    for (int i = 0; i < totalInnings; i++) {
        PrintInningsScorecard(i);
    }
}

void MatchEngine::PrintInningsScorecard(int idx) const {
    auto& score = innings[idx];
    if (score.battingCard.empty()) return;

    Say("\n  ---- Batting ----");
    for (auto& b : score.battingCard) {
        std::ostringstream ss;
        ss << "  " << std::left << std::setw(18) << b.name;
        if (b.isOut) {
            switch (b.howOut) {
                case DismissalType::Bowled: ss << "b " << b.dismissedBy; break;
                case DismissalType::Caught: ss << "c sub b " << b.dismissedBy; break;
                case DismissalType::CaughtBehind: ss << "c †wk b " << b.dismissedBy; break;
                case DismissalType::LBW: ss << "lbw b " << b.dismissedBy; break;
                case DismissalType::CaughtAndBowled: ss << "c&b " << b.dismissedBy; break;
                case DismissalType::Stumped: ss << "st †wk b " << b.dismissedBy; break;
                case DismissalType::RunOut: ss << "run out"; break;
                default: ss << "out"; break;
            }
        } else if (b.ballsFaced > 0) {
            ss << "not out";
        } else {
            ss << "did not bat";
            Say(ss.str());
            continue;
        }
        ss << "  " << std::right << std::setw(4) << b.runs
           << " (" << b.ballsFaced << "b, " << b.fours << "x4, " << b.sixes << "x6)"
           << " SR:" << std::fixed << std::setprecision(1) << b.GetStrikeRate();
        Say(ss.str());
    }

    Say("  Extras: " + std::to_string(score.extras)
        + " (w:" + std::to_string(score.wides)
        + " nb:" + std::to_string(score.noBalls) + ")");
    Say("  Total: " + score.GetScoreString());

    if (!score.bowlingCard.empty()) {
        Say("\n  ---- Bowling ----");
        for (auto& bf : score.bowlingCard) {
            if (bf.overs == 0 && bf.ballsInCurrentOver == 0) continue;
            std::ostringstream ss;
            ss << "  " << std::left << std::setw(18) << bf.name
               << bf.GetFigures()
               << " Econ:" << std::fixed << std::setprecision(2) << bf.GetEconomy();
            if (bf.maidens > 0) ss << " M:" << bf.maidens;
            Say(ss.str());
        }
    }

    if (!score.fallOfWickets.empty()) {
        Say("\n  Fall of wickets:");
        std::ostringstream fow;
        fow << "  ";
        for (size_t i = 0; i < score.fallOfWickets.size(); i++) {
            fow << score.fallOfWickets[i];
            if (i < score.fallOfWickets.size() - 1) fow << ", ";
        }
        Say(fow.str());
    }
}

void MatchEngine::Say(const std::string& msg) const {
    if (commentary) {
        commentary(msg);
    } else {
        std::cout << msg << std::endl;
    }
}

float MatchEngine::RandFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int MatchEngine::RandInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}
