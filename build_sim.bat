@echo off
echo Building Cricket Simulation...
echo.

REM Try g++ first (MinGW/MSYS2)
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using g++...
    g++ -std=c++17 -O2 -Isrc src/cricket/cricket_main.cpp src/cricket/MatchEngine.cpp src/cricket/CricketTypes.cpp src/cricket/TeamData.cpp -o CricketSim.exe
    if %ERRORLEVEL% EQU 0 (
        echo Build successful! Run: CricketSim.exe
    ) else (
        echo Build failed.
    )
    goto :end
)

REM Try cl.exe (MSVC)
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using MSVC...
    cl /std:c++17 /O2 /EHsc /Isrc src\cricket\cricket_main.cpp src\cricket\MatchEngine.cpp src\cricket\CricketTypes.cpp src\cricket\TeamData.cpp /Fe:CricketSim.exe
    if %ERRORLEVEL% EQU 0 (
        echo Build successful! Run: CricketSim.exe
    ) else (
        echo Build failed.
    )
    goto :end
)

echo No C++ compiler found. Install MinGW/MSYS2 or Visual Studio.
echo Or use CMake: cmake -B build_sim -S . & cmake --build build_sim

:end
pause
