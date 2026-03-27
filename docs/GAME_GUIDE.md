# Cricket Game - Complete Player Guide

## Table of Contents
1. [Quick Start](#quick-start)
2. [Running the Game on Any Windows PC](#running-the-game-on-any-windows-pc)
3. [Game Menus and Options](#game-menus-and-options)
4. [Match Formats](#match-formats)
5. [Teams and Players](#teams-and-players)
6. [Stadiums](#stadiums)
7. [Cricket Simulation Mechanics](#cricket-simulation-mechanics)
8. [Weather and Pitch System](#weather-and-pitch-system)
9. [Ball Condition and Wear](#ball-condition-and-wear)
10. [Reading the Scorecard](#reading-the-scorecard)
11. [Building from Source](#building-from-source)
12. [Troubleshooting](#troubleshooting)

---

## Quick Start

The fastest way to play:

1. Double-click `CricketSim.exe` in the project root folder
2. Choose Test Match or ODI
3. Pick home team (England or Australia)
4. Select a stadium
5. Press Enter and watch the match unfold

No installation, no dependencies, no setup needed. The `.exe` runs standalone on any 64-bit Windows PC.

---

## Running the Game on Any Windows PC

### Method 1: Direct Run (Easiest)

Just copy `CricketSim.exe` to any Windows computer and double-click it. It opens in a Command Prompt window.

If double-clicking closes the window immediately:
- Right-click `CricketSim.exe`
- Select "Open with" > "Command Prompt"
- Or open Command Prompt / PowerShell first, navigate to the folder, and type:

```
.\CricketSim.exe
```

### Method 2: From Command Prompt

```
cd C:\path\to\game\folder
CricketSim.exe
```

### Method 3: From PowerShell

```powershell
cd C:\path\to\game\folder
.\CricketSim.exe
```

### Windows Compatibility

| Windows Version | Supported |
|----------------|-----------|
| Windows 11     | Yes       |
| Windows 10     | Yes       |
| Windows 8.1    | Yes       |
| Windows 7 SP1  | Yes (may need Visual C++ Redistributable 2019) |

If you get a "VCRUNTIME140.dll not found" error on older Windows:
- Download "Microsoft Visual C++ Redistributable for Visual Studio 2019" from:
  https://aka.ms/vs/16/release/vc_redist.x64.exe
- Install it, then run the game again

### Windows SmartScreen Warning

If Windows shows "Windows protected your PC":
1. Click "More info"
2. Click "Run anyway"

This happens because the exe is not digitally signed. It is safe to run.

---

## Game Menus and Options

When you start the game, you see a text menu:

```
  ========================================
       CRICKET GAME - Console Edition
  ========================================

Select match format:
  1. Test Match (90 overs per innings)
  2. ODI (50 overs)
>
```

Type the number and press Enter for each choice:

1. **Match Format** - Test or ODI
2. **Home Team** - England (1) or Australia (2)
3. **Stadium** - Pick from 8 venues (4 in England, 4 in Australia)
4. **Review Squads** - Both team lineups are shown with player roles and ratings
5. **Press Enter** - Match begins

The match then simulates ball-by-ball with live commentary. You watch it unfold like a live text scorecard.

---

## Match Formats

### Test Match

- 4 innings total (each team bats twice)
- 90 overs maximum per innings
- Ball does significantly more in the first 40-50 overs:
  - Overs 1-20: New ball, heavy swing and seam (+12% difficulty)
  - Overs 20-40: Ball still assisting bowlers (+6% difficulty)
  - Overs 40-50: Ball getting older, less movement (+3% difficulty)
  - Overs 50-80: Flat period, good for batting
  - Over 80: New ball available, swing returns
- Pitch deteriorates between innings (helps spinners in 3rd and 4th innings)
- Reverse swing kicks in when ball shine drops below 25%
- Team can declare if score exceeds 500 after 60 overs
- 4th innings target is calculated automatically

### ODI (One Day International)

- 2 innings total (each team bats once)
- 50 overs maximum per innings
- **Two balls used from both ends** (alternating each over):
  - Ball 1 used from one end on odd overs
  - Ball 2 used from the other end on even overs
  - Both balls wear independently
  - After 40 overs, both balls are significantly worn
  - Reverse swing possible from either end if ball is old enough
- Powerplay (overs 1-10): New ball advantage (+8% difficulty)
- Death overs (overs 40-50): Batsmen play more aggressively
- Maximum 10 overs per bowler
- Second innings has a target to chase

---

## Teams and Players

### England Squad

| Player       | Role | Bat | Bowl | Style           |
|-------------|------|-----|------|-----------------|
| Z Crawley    | BAT  | 78  | 15   | Aggressive opener |
| B Duckett    | BAT  | 80  | 10   | Power hitter     |
| J Root       | BAT  | 92  | 20   | Elite technique, patient |
| H Brook      | BAT  | 85  | 12   | Power + flair    |
| J Bairstow   | BAT  | 76  | 10   | Aggressive middle order |
| B Stokes     | AR   | 82  | 72   | Power all-rounder |
| J Foakes     | WK   | 68  | 5    | Defensive keeper-bat |
| M Wood       | RF   | 25  | 85   | Express pace     |
| J Anderson   | RMF  | 15  | 90   | Swing master     |
| S Broad      | RMF  | 20  | 86   | Seam and bounce  |
| J Leach      | OS   | 18  | 78   | Off-spin         |

### Australia Squad

| Player        | Role | Bat | Bowl | Style           |
|--------------|------|-----|------|-----------------|
| U Khawaja     | BAT  | 84  | 10   | Patient, high technique |
| D Warner      | BAT  | 82  | 15   | Explosive opener |
| M Labuschagne | BAT  | 88  | 25   | Elite technique  |
| S Smith       | BAT  | 93  | 18   | Best technique in game |
| T Head        | BAT  | 79  | 15   | Aggressive middle order |
| C Green       | AR   | 72  | 70   | Tall all-rounder |
| A Carey       | WK   | 70  | 5    | Attacking keeper |
| P Cummins     | RF   | 22  | 92   | Best fast bowler |
| M Starc       | RF   | 30  | 88   | Left-arm pace    |
| J Hazlewood   | RMF  | 15  | 87   | Relentless seam  |
| N Lyon        | OS   | 20  | 82   | Off-spin veteran |

### Role Key
- BAT = Batsman
- AR = All-Rounder
- WK = Wicket-Keeper
- RF = Right-arm Fast
- RMF = Right-arm Medium Fast
- OS = Off-Spin
- LS = Leg-Spin

### How Player Skills Affect the Game

Each player has hidden attributes that determine their behavior:

- **Batting Skill (0-100)**: Overall batting ability. Higher = more runs, fewer dismissals
- **Bowling Skill (0-100)**: Overall bowling ability. Higher = more wickets, fewer runs conceded
- **Power Hitting (0-1)**: Ability to hit sixes and big fours. Warner (0.85) vs Khawaja (0.45)
- **Technique (0-1)**: Ability to survive tough conditions. Smith (0.96) vs Warner (0.60)
- **Temperament (0-1)**: Patience and shot selection. Smith (0.95) plays defensive more often; Warner (0.45) attacks more
- **Running Speed (0-1)**: Affects singles and twos conversion

A player like Steve Smith (technique 0.96, temperament 0.95) will:
- Leave more balls outside off stump
- Play defensive shots in tough conditions
- Rarely get out to rash shots
- Score slowly but survive long

A player like David Warner (power 0.85, temperament 0.45) will:
- Attack from ball one
- Play more lofted drives and pulls
- Score faster but risk more dismissals
- Thrive on flat pitches

---

## Stadiums

### England Venues

| Stadium     | City       | Boundary | Pitch    | Grass | Weather   |
|------------|------------|----------|----------|-------|-----------|
| Lord's      | London     | 68m      | Medium   | 50%   | Overcast  |
| The Oval    | London     | 65m      | Med-Hard | 35%   | Cloudy    |
| Headingley  | Leeds      | 64m      | Soft     | 65%   | Overcast  |
| Edgbaston   | Birmingham | 66m      | Medium   | 45%   | Cloudy    |

**England characteristics**: Overcast skies boost swing bowling. Green pitches at Headingley offer seam movement. Smaller boundaries mean more fours. High humidity helps Anderson and Broad.

### Australia Venues

| Stadium      | City      | Boundary | Pitch | Grass | Weather |
|-------------|-----------|----------|-------|-------|---------|
| MCG          | Melbourne | 75m      | Hard  | 25%   | Sunny   |
| SCG          | Sydney    | 70m      | Med-Hard | 30% | Sunny   |
| The Gabba    | Brisbane  | 72m      | Hard  | 35%   | Humid   |
| Adelaide Oval| Adelaide  | 71m      | Med-Hard | 28% | Sunny   |

**Australia characteristics**: Hard, dry pitches favor batting. Bigger boundaries mean fewer fours (balls that would be 4 at Headingley might only be 3 at the MCG). Low humidity means less swing. Pitches deteriorate more in Tests, helping Lyon's spin in the 4th innings.

### How Stadium Affects Gameplay

- **Boundary Size**: Larger boundaries (MCG 75m) reduce fours. A shot that's a boundary at The Oval (65m) might only be 3 runs at the MCG.
- **Pitch Hardness**: Hard pitches (Gabba 0.75) = more bounce, faster scoring. Soft pitches (Headingley 0.50) = lower bounce, helps seamers.
- **Grass Coverage**: Green pitches (Headingley 65%) = more seam movement. Bare pitches (MCG 25%) = better for batting.
- **Moisture**: Damp pitches help seam bowlers grip the ball. Dry pitches help spinners.
- **Weather**: Overcast = more swing. Sunny = less swing, more spin later. Humid = enhanced swing factor.

---

## Cricket Simulation Mechanics

### How Each Ball is Simulated

Every delivery goes through this calculation:

1. **Bowler chooses delivery type**: Good length, short, full, yorker, bouncer
   - Fast bowlers prefer pace and bounce
   - Spinners prefer flight and turn
   - Death overs = more yorkers

2. **Ball movement calculated**:
   - Swing (air movement) based on: bowler skill, ball shine, weather humidity
   - Seam (off the pitch) based on: bowler skill, grass coverage, pitch moisture
   - Spin based on: bowler skill, pitch deterioration, dryness
   - Bounce based on: pitch hardness + random variation

3. **Difficulty score computed** (0.0 to 0.85):
   - Sum of swing, seam, spin, bounce contributions
   - Modified by delivery type (yorker +15%, bouncer +10%)
   - Modified by match phase (new ball period, death overs)
   - Modified by weather conditions

4. **Batsman chooses shot** based on:
   - Difficulty (high technique batsmen leave/defend more)
   - Temperament (patient batsmen play safer)
   - Match situation (chasing target = more aggressive)
   - Innings phase (death overs = more lofted shots)

5. **Dismissal check**:
   - Base chance = difficulty x 8%
   - Reduced by batting skill and technique
   - Increased by risky shots (lofted drive x2.5, pull x1.8)
   - New batsmen more vulnerable (first 10 balls x1.5)
   - Capped between 0.5% and 15% per ball

6. **Runs scored** if not out:
   - Shot power = batting skill x shot type modifier x contact quality
   - High power + good contact = boundary
   - Medium = 1-3 runs
   - Low = dot ball

### Shot Types

| Shot          | Risk  | Reward | Best For           |
|--------------|-------|--------|--------------------|
| Defensive     | Low   | 0-2    | Survival           |
| Drive         | Medium| 1-4    | Full balls         |
| Cut           | Medium| 1-4    | Short wide balls   |
| Pull          | High  | 1-6    | Short balls        |
| Sweep         | Medium| 1-4    | Spin bowling       |
| Flick         | Medium| 1-4    | On the pads        |
| Lofted Drive  | Very High | 4-6 | Going big         |
| Upper Cut     | Very High | 4-6 | Short balls       |
| Leave         | None  | 0      | Tough conditions   |

### Dismissal Types

| Type          | How It Happens                    | Typical Bowler |
|--------------|-----------------------------------|----------------|
| Bowled        | Ball hits stumps                  | Any            |
| Caught        | Fielder catches the ball          | Any            |
| Caught Behind | Wicket-keeper catches edge        | Fast bowlers   |
| LBW           | Ball hits pad, would hit stumps   | Any            |
| Caught & Bowled| Bowler catches return catch      | Any            |
| Stumped       | Keeper breaks stumps, bat out     | Spinners       |
| Run Out       | Fielder hits stumps while running | Any            |

---

## Weather and Pitch System

### Weather Effects

The weather is determined by the stadium's typical conditions with slight random variation:

**Overcast/Cloudy** (typical England):
- Swing factor boosted by +40%
- Humidity usually 68-78%
- Favors: Anderson, Broad, Stokes (swing bowlers)
- Ball moves more in the air throughout the innings

**Sunny** (typical Australia):
- Spin factor boosted by +20%
- Humidity usually 40-50%
- Favors: Lyon, Leach (spinners, especially later)
- Less swing, more bounce off hard pitches

**Humid** (Brisbane):
- Swing factor boosted by +30% when humidity > 70%
- Combined with hard pitch = challenging for batsmen
- Favors: Cummins, Starc (pace + swing)

### Pitch Deterioration (Test Matches)

The pitch changes over the course of a Test match:

- **Innings 1**: Fresh pitch, consistent bounce, some grass
- **Between innings**: Deterioration +8% per innings break
- **Innings 2**: Slightly worn, grass flattened
- **Innings 3**: Cracks forming, variable bounce starting
- **Innings 4**: Rough patches, significant spin, unpredictable bounce

This means:
- Fast bowlers dominate innings 1 and 2
- Spinners become more dangerous in innings 3 and 4
- Batting gets harder as the match progresses
- 4th innings chases are the hardest

---

## Ball Condition and Wear

### Test Match Ball

One ball is used per innings:
- **New (overs 1-20)**: Shine 100-94%, heavy swing, sharp seam
- **Semi-new (overs 20-40)**: Shine 94-82%, reducing swing, seam still works
- **Old (overs 40-60)**: Shine 82-64%, minimal swing, flat period
- **Worn (overs 60-80)**: Shine below 50%, reverse swing possible when shine < 25%
- **New ball (over 80)**: Fresh ball taken, swing returns

Ball condition report appears every 20 overs:
```
Ball condition: shine=67%, age=240 deliveries
Ball condition: shine=23%, age=480 deliveries (REVERSING)
```

### ODI Ball (Two-Ball System)

Two balls are used simultaneously, one from each end:
- Ball 1 bowled from one end (odd overs: 1, 3, 5...)
- Ball 2 bowled from the other end (even overs: 2, 4, 6...)
- Each ball ages independently
- By over 40, both balls have ~150 deliveries each
- Both balls can develop reverse swing independently

Ball condition report appears every 10 overs:
```
Ball (end 1): shine=45%, age=150
Ball (end 2): shine=52%, age=140
```

After 40 overs:
- Both balls are significantly worn
- Less conventional swing
- Reverse swing possible from either end
- Batting generally easier (worn ball = less movement)

---

## Reading the Scorecard

### During the Match

Live commentary shows ball-by-ball action:
```
  FOUR! J Root drives through covers for FOUR! 45/0 (8.3 ov)
  WICKET! Z Crawley lbw b P Cummins 52/1 (12.4 ov)
  1 run. H Brook nudges for a single.
  SIX! B Stokes launches a magnificent lofted drive for SIX! 156/3 (28.2 ov)
  Wide ball! 157/3 (28.2 ov)
```

Every 5 overs, a summary appears:
```
  After 25 overs: 134/2 (25 ov) (RR: 5.36)
```

When chasing:
```
  After 30 overs: 156/4 (30 ov) (RR: 5.20) Need 95 from 20.0 ov (RRR: 4.75)
```

### End of Innings Scorecard

```
  ---- Batting ----
  Z Crawley         c †wk b P Cummins    23 (34b, 3x4, 0x6) SR:67.6
  B Duckett         b M Starc             45 (52b, 7x4, 1x6) SR:86.5
  J Root            not out               89 (124b, 10x4, 1x6) SR:71.8
  H Brook           lbw b J Hazlewood    12 (28b, 1x4, 0x6) SR:42.9
  ...
  Extras: 8 (w:3 nb:2)
  Total: 267/6 (50 ov)

  ---- Bowling ----
  P Cummins         2/48 (10 ov) Econ:4.80 M:1
  M Starc           1/56 (10 ov) Econ:5.60
  J Hazlewood       2/42 (10 ov) Econ:4.20 M:2
  N Lyon            1/52 (10 ov) Econ:5.20

  Fall of wickets:
  1-34 (Z Crawley, 8.3 ov), 2-89 (B Duckett, 18.1 ov), ...
```

### Reading Player Stats

- **23 (34b, 3x4, 0x6)**: 23 runs from 34 balls, 3 fours, 0 sixes
- **SR:67.6**: Strike rate (runs per 100 balls)
- **2/48 (10 ov)**: 2 wickets for 48 runs in 10 overs
- **Econ:4.80**: Economy rate (runs conceded per over)
- **M:1**: 1 maiden over (over with 0 runs)

---

## Building from Source

If you want to compile the game yourself (not needed to play):

### Prerequisites
- Any C++17 compiler (Visual Studio 2019+, g++ 9+, clang 10+)
- No external libraries needed for the console simulation

### Windows with Visual Studio (already installed)

Open "Developer Command Prompt for VS 2019" from Start menu, then:
```
cd C:\path\to\cricket-game
cl /std:c++17 /O2 /EHsc /Isrc src\cricket\cricket_main.cpp src\cricket\MatchEngine.cpp src\cricket\CricketTypes.cpp src\cricket\TeamData.cpp /Fe:CricketSim.exe
```

### Windows with g++ (MinGW/MSYS2)

```bash
g++ -std=c++17 -O2 -Isrc src/cricket/cricket_main.cpp src/cricket/MatchEngine.cpp src/cricket/CricketTypes.cpp src/cricket/TeamData.cpp -o CricketSim.exe
```

### Linux / macOS

```bash
g++ -std=c++17 -O2 -Isrc src/cricket/cricket_main.cpp src/cricket/MatchEngine.cpp src/cricket/CricketTypes.cpp src/cricket/TeamData.cpp -o CricketSim
./CricketSim
```

### Using CMake

```bash
cmake -B build_sim -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build_sim --target CricketSim
./build_sim/bin/CricketSim
```

### Using the Build Script

Windows: Double-click `build_sim.bat`
Linux/macOS: `bash build_sim.sh`

---

## Troubleshooting

### "CricketSim.exe is not recognized"
Make sure you are in the correct directory. Use `cd` to navigate to where the exe is.

### Window closes immediately after double-click
Open Command Prompt first, then run the exe from there. The game needs keyboard input.

### "VCRUNTIME140.dll not found"
Install Visual C++ Redistributable 2019: https://aka.ms/vs/16/release/vc_redist.x64.exe

### Game seems stuck / no output
The game is simulating. Test matches are long (up to 4 innings x 90 overs). Wait for it to complete or choose ODI for a shorter game.

### Scores seem unrealistic
Each match is randomly simulated. Occasionally you'll see very high or very low scores, just like real cricket. Run it again for a different result.

### Want to pipe output to a file
```
.\CricketSim.exe > match_log.txt
```
Note: You'll need to provide inputs (format, team, stadium) before the redirect, or use:
```
echo 2 1 5 | .\CricketSim.exe > match_log.txt
```
(This selects ODI, England home, MCG)
