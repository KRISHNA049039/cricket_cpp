#pragma once
#include <SDL2/SDL.h>
#include <array>

class InputHandler {
public:
    InputHandler();
    void Update();
    bool IsKeyPressed(SDL_Scancode key) const;
    bool IsKeyJustPressed(SDL_Scancode key) const;
private:
    std::array<bool, SDL_NUM_SCANCODES> currentKeys;
    std::array<bool, SDL_NUM_SCANCODES> previousKeys;
};
