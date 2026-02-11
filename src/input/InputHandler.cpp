#include "InputHandler.h"

InputHandler::InputHandler() {
    currentKeys.fill(false);
    previousKeys.fill(false);
}

void InputHandler::Update() {
    previousKeys = currentKeys;
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        currentKeys[i] = state[i];
    }
}

bool InputHandler::IsKeyPressed(SDL_Scancode key) const {
    return currentKeys[key];
}

bool InputHandler::IsKeyJustPressed(SDL_Scancode key) const {
    return currentKeys[key] && !previousKeys[key];
}
