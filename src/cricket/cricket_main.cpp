#include "MatchEngine.h"
#include "TeamData.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int getChoice(int min, int max) {
    int choice;
    while (true) {
        std::cout << "> ";
        if (std::cin >> choice && choice >= min && choice <= max) {
            clearInput();
            return choice;
        }
        std::cout << "Invalid choice. Try again.\n";
        clearInput();
    }
}

int main() {
    std::cout << R"(
  ========================================
       CRICKET GAME - Console Edition
  ========================================
    )" << std::endl;

    // Select format
    std::cout << "Select match format:\n";
    std::cout << "  1. Test Match (90 overs per innings)\n";
    std::cout << "  2. ODI (50 overs)\n";
    int formatChoice = getChoice(1, 2);
    MatchFormat format = (formatChoice == 1) ? MatchFormat::Test : MatchFormat::ODI;

    // Select teams
    auto england = TeamDatabase::CreateEngland();
    auto australia = TeamDatabase::CreateAustralia();

    std::cout << "\nTeams: England vs Australia\n";
    std::cout << "\nSelect home team:\n";
    std::cout << "  1. England\n";
    std::cout << "  2. Australia\n";
    int homeChoice = getChoice(1, 2);

    Team& homeTeam = (homeChoice == 1) ? england : australia;
    Team& awayTeam = (homeChoice == 1) ? australia : england;

    // Select stadium
    auto stadiums = TeamDatabase::GetStadiums();
    std::cout << "\nSelect stadium:\n";
    for (int i = 0; i < (int)stadiums.size(); i++) {
        std::cout << "  " << (i + 1) << ". " << stadiums[i].name
                  << " (" << stadiums[i].city << ", " << stadiums[i].country << ")\n";
    }
    int stadChoice = getChoice(1, stadiums.size());
    Stadium& selectedStadium = stadiums[stadChoice - 1];

    // Show squads
    std::cout << "\n--- " << homeTeam.name << " Squad ---\n";
    for (auto& p : homeTeam.players) {
        std::string role;
        if (p.isWicketKeeper) role = "WK";
        else if (p.bowlingSkill >= 70 && p.battingSkill >= 60) role = "AR";
        else if (p.bowlingSkill >= 60) {
            switch (p.bowlType) {
                case BowlingType::Fast: role = "RF"; break;
                case BowlingType::MediumFast: role = "RMF"; break;
                case BowlingType::OffSpin: role = "OS"; break;
                case BowlingType::LegSpin: role = "LS"; break;
                default: role = "MED"; break;
            }
        } else role = "BAT";
        std::cout << "  " << p.name << " [" << role
                  << " Bat:" << p.battingSkill << " Bowl:" << p.bowlingSkill << "]\n";
    }

    std::cout << "\n--- " << awayTeam.name << " Squad ---\n";
    for (auto& p : awayTeam.players) {
        std::string role;
        if (p.isWicketKeeper) role = "WK";
        else if (p.bowlingSkill >= 70 && p.battingSkill >= 60) role = "AR";
        else if (p.bowlingSkill >= 60) {
            switch (p.bowlType) {
                case BowlingType::Fast: role = "RF"; break;
                case BowlingType::MediumFast: role = "RMF"; break;
                case BowlingType::OffSpin: role = "OS"; break;
                case BowlingType::LegSpin: role = "LS"; break;
                default: role = "MED"; break;
            }
        } else role = "BAT";
        std::cout << "  " << p.name << " [" << role
                  << " Bat:" << p.battingSkill << " Bowl:" << p.bowlingSkill << "]\n";
    }

    std::cout << "\nPress Enter to start the match...";
    clearInput();

    // Run match
    MatchEngine engine;
    engine.SetupMatch(homeTeam, awayTeam, selectedStadium, format);
    engine.SimulateMatch();

    std::cout << "\nMatch complete. Press Enter to exit.";
    std::cin.get();

    return 0;
}
