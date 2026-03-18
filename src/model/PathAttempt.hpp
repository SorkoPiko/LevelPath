#pragma once
#include "AttemptTick.hpp"

struct PathAttempt {
    int recordingRate;
    std::vector<AttemptTick> p1Ticks;
    std::vector<AttemptTick> p2Ticks;
};