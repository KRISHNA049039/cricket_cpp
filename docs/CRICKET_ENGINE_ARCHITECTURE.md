# Cricket Simulation Engine - Technical Architecture

## Overview

The Cricket Simulation Engine (`src/cricket/`) is a standalone, zero-dependency match simulation system that models realistic cricket matches with ball-by-ball delivery simulation, environmental physics, player skill differentiation, and format-specific rules.

It compiles independently of the 3D engine (no SDL2, OpenGL, or any external library needed).

---

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    cricket_main.cpp                       │
│              (Menu UI, Team Selection, I/O)               │
└──────────────────────┬──────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────┐
│                    MatchEngine                            │
│                                                           │
│  SetupMatch() ──► SimulateMatch() ──► PrintScorecard()   │
│                        │                                  │
│              ┌─────────┼─────────┐                       │
│              ▼         ▼         ▼                        │
│      SimulateInnings  Weather   Pitch                    │
│              │        System    System                    │
│              ▼                                            │
│      SimulateOver                                        │
│              │                                            │
│              ▼                                            │
│      SimulateDelivery                                    │
│         │    │    │    │                                  │
│         ▼    ▼    ▼    ▼                                  │
│      Swing Seam Spin Bounce                              │
│      Calc  Calc Calc  Calc                               │
│              │                                            │
│         ┌────┼────┐                                      │
│         ▼    ▼    ▼                                      │
│      Choose Choose Check                                 │
│      Shot   Delivery Dismissal                           │
└─────────────────────────────────────────────────────────┘
                       │
          ┌────────────┼────────────┐
          ▼            ▼            ▼
┌──────────────┐ ┌──────────┐ ┌──────────────┐
│ CricketTypes │ │ TeamData │ │ Commentary   │
│ (Structs,    │ │ (Teams,  │ │ (Callback    │
│  Enums,      │ │ Stadiums,│ │  System)     │
│  Scoring)    │ │ Players) │ │              │
└──────────────┘ └──────────┘ └──────────────┘
```

---

## File Structure

```
src/cricket/
├── CricketTypes.h      # All data structures, enums, scoring types
├── CricketTypes.cpp    # Score string formatting
├── TeamData.h          # Team and Stadium database interface
├── TeamData.cpp        # England/Australia squads, 8 stadiums
├── MatchEngine.h       # Core simulation engine header
├── MatchEngine.cpp     # ~960 lines of match simulation logic
└── cricket_main.cpp    # Console UI entry point
```

---

## Core Data Structures

### Match Configuration

```
MatchFormat ──► Test (4 innings, 90 ov each)
            └─► ODI  (2 innings, 50 ov each)

Stadium ──► name, city, country
        ├─► boundarySize (65-75m)
        ├─► pitchHardness, grassCoverage, baseMoisture
        └─► typicalWeather, humidity, windSpeed, temperature
```

### Environmental State

```
Weather ──► type (Sunny/Overcast/Cloudy/Drizzle/Humid)
        ├─► humidity (0-1) ──► GetSwingFactor()
        ├─► windSpeed (km/h)
        ├─► temperature (°C)
        └─► GetSpinFactor()

PitchCondition ──► hardness (0-1)
               ├─► grassCoverage (0-1)
               ├─► moisture (0-1)
               └─► deterioration (0-1, increases over match)

BallCondition ──► shine (1.0 → 0.0 over time)
              ├─► seamIntegrity (1.0 → 0.1)
              ├─► ballAge (delivery count)
              └─► isReversing (true when shine < 0.25 and age > 150)
```

### Player Model

```
PlayerStats ──► name
            ├─► battingSkill (0-100)
            ├─► bowlingSkill (0-100)
            ├─► fieldingSkill (0-100)
            ├─► bowlType (Fast/MediumFast/Medium/OffSpin/LegSpin)
            ├─► isWicketKeeper
            ├─► powerHitting (0-1)    ──► Affects six-hitting ability
            ├─► technique (0-1)       ──► Affects survival in tough conditions
            ├─► temperament (0-1)     ──► Affects shot selection aggression
            └─► runningSpeed (0-1)    ──► Affects singles/twos conversion
```

---

## Simulation Pipeline

### Per-Delivery Flow

```
SimulateDelivery(bowler, batsman)
│
├─► 1. Choose Delivery Type
│       Fast bowler: Good(50%) / Short(20%) / Full(15%) / Bouncer(7%) / Yorker(8%)
│       Spinner:     Good(60%) / Full(20%) / Short(15%) / Yorker(5%)
│       Death overs: Yorker probability +30%
│
├─► 2. Calculate Ball Movement
│       ├─► Swing = f(bowlerSkill, ballShine, weather.humidity, ballCondition)
│       │     Conventional: shine > 0.5 → boost
│       │     Reverse: shine < 0.25 + age > 150 → 1.4x boost
│       │     Weather: overcast → +40%, humid → +30%
│       │
│       ├─► Seam = f(bowlerSkill, pitch.grass, pitch.moisture, ball.seamIntegrity)
│       │     Green pitch (grass > 0.6) → +50% seam
│       │     Damp pitch → +30% seam
│       │
│       ├─► Spin = f(bowlerSkill, pitch.deterioration, pitch.dryness, weather)
│       │     Deteriorated pitch → up to +150% spin
│       │     Dry conditions → +30% spin
│       │     Sunny weather → +20% spin
│       │
│       └─► Bounce = f(pitch.hardness, pitch.deterioration, random)
│             Variable bounce on worn pitch
│
├─► 3. Compute Difficulty Score (0.05 - 0.85)
│       difficulty = swing*0.25 + seam*0.2 + spin*0.2 + bounce*0.15 + bowlerSkill*0.2
│       + delivery modifiers (yorker +0.15, bouncer +0.10)
│       + format modifiers:
│           Test overs 1-20:  +0.12 (new ball period)
│           Test overs 20-40: +0.06
│           Test overs 40-50: +0.03
│           ODI overs 1-10:   +0.08 (powerplay)
│           ODI overs 40+:    -0.05 (worn ball)
│       × weather swing factor
│
├─► 4. Check Wide / No-Ball
│       Wide chance: 3% - (bowlerSkill/100 × 2%)
│       No-ball chance: 1.5%
│
├─► 5. Batsman Shot Selection
│       Based on: difficulty, technique, temperament, match situation, phase
│       High technique + high difficulty → Leave/Defend
│       Low temperament → More attacking shots
│       Chasing high RRR → More lofted shots
│       Death overs → +25% aggression
│
├─► 6. Dismissal Check
│       Base: difficulty × 8%
│       × (1 - battingSkill/150)     skill reduction
│       × shot risk multiplier        (lofted drive ×2.5, defensive ×0.5)
│       × (1 - technique × 0.3)      technique reduction
│       × new batsman penalty         (first 10 balls ×1.5)
│       Clamped to [0.5%, 15%]
│       If dismissed → determine type (bowled/caught/lbw/etc.)
│
└─► 7. Calculate Runs
        shotValue = battingPower × contactQuality(random)
        > 0.92 + power shot → SIX
        > 0.70            → FOUR (reduced if boundary > 72m)
        > 0.45            → 1-3 runs
        > 0.20            → single
        else              → dot ball
```

### Bowler Selection Algorithm

```
SelectBowler()
│
├─► Build eligible list (bowlingSkill >= 40, not last over's bowler)
│   └─► ODI: exclude bowlers who've bowled 10 overs
│
├─► Early phase (overs 1-20%):
│   └─► Prefer fast bowlers (Fast, MediumFast)
│
├─► Middle/Late phase + deteriorated pitch:
│   └─► 60% chance to prefer spinners (OffSpin, LegSpin)
│
└─► Default: random from eligible list
```

### Ball Wear Model

```
Per delivery:
  shine -= 0.003        (reaches 0 after ~333 deliveries / ~55 overs)
  seamIntegrity -= 0.001 (reaches 0.1 minimum)
  ballAge++
  if shine < 0.25 AND ballAge > 150 → isReversing = true

Test: single ball, new ball at over 80
ODI:  two balls alternating ends, no replacement
```

---

## Design Patterns Used

### Strategy Pattern (Implicit)
Shot selection and delivery choice vary based on player stats and match context without explicit strategy classes - the logic branches on player attributes.

### State Machine
Match progresses through: Setup → Innings → Over → Delivery → Over End → Innings End → Match End.
Innings phase (Early/Middle/Late/Death) drives bowler selection and batting aggression.

### Callback Pattern
`CommentaryCallback` allows the engine to output text through any system (console, GUI, network):
```cpp
using CommentaryCallback = std::function<void(const std::string&)>;
```

### Data-Driven Design
All player stats, stadium properties, and weather parameters are data, not code. Adding a new team or stadium requires only adding data to `TeamData.cpp`.

---

## Random Number Generation

Uses `std::mt19937` (Mersenne Twister) seeded from `std::chrono::steady_clock` for:
- Non-deterministic match outcomes
- Uniform float distribution for probability checks
- Uniform int distribution for discrete choices

Every match produces different results. The same teams at the same venue will play differently each time.

---

## Integration with 3D Engine

The cricket simulation engine is designed to integrate with the existing 3D engine:

```
3D Engine (src/core/, src/rendering/, etc.)
    │
    ├─► Application creates MatchEngine
    ├─► MatchEngine.SetCommentaryCallback() → feeds UI/HUD
    ├─► Each SimulateDelivery() result drives:
    │     ├─► Ball physics animation
    │     ├─► Player animation (shot type)
    │     ├─► Camera mode (follow ball, batsman view)
    │     └─► Audio (bat hit, crowd, commentary)
    └─► InningsScore feeds scoreboard rendering
```

The simulation engine is pure logic with no rendering dependencies, making it testable and portable.

---

## Performance

- Single match simulation: < 100ms for ODI, < 500ms for Test
- Memory: ~50KB per match (player data + scoring arrays)
- Zero heap allocations during delivery simulation (all stack-based math)
- No external dependencies, no file I/O during simulation
