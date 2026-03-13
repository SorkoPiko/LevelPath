#pragma once
#include "GameMode.hpp"

struct AttemptTick {
    float x;
    float y;
    float rotation;
    GameMode gameMode;
    bool gravityFlipped;
    bool mini;
};
