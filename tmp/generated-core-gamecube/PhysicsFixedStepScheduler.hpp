#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_exceptions.hpp"

class PhysicsFixedStepScheduler
{
public:
    virtual ~PhysicsFixedStepScheduler() = default;

    double get_AccumulatedSeconds();

    double StepSeconds;

    double get_StepSeconds();

    void AddElapsedSeconds(double elapsedSeconds);

    PhysicsFixedStepScheduler(double stepSeconds);

    void Reset();

    bool TryConsumeStep();
private:
    double AccumulatedSecondsValue;
};
