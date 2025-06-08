#pragma once
#include <optional>
#include <functional>

struct Timer {
    float elapsed = 0.0f;
    float duration;
    unsigned int iterations = 1;
    bool repeat = false;
    bool justCompleted = false;
    bool completed = false;
    bool infinite = false;

    explicit Timer(float duration_) : duration(duration_)
    {

    }

    void Update(float deltaTime) {
        elapsed += deltaTime;

        if (infinite) return;
        if (justCompleted) justCompleted = false;
        if (completed) return;
        if (elapsed >= duration) {
            justCompleted = true;
            if (repeat) {
                elapsed -= duration;
                return;
            }
            if (iterations > 1) {
                iterations--;
                elapsed -= duration;
            } else {
                completed = true;
            }
        }
    }

    bool JustCompleted() const {
        return justCompleted;
    }

    bool Completed() const {
        return elapsed >= duration;
    }

    Timer &SetDuration(float duration_) {
        duration = duration_;
        return *this;
    }

    Timer &SetIterations(unsigned int iterations_) {
        iterations = iterations_;
        return *this;
    }

    Timer &SetRepeat(bool repeat_) {
        repeat = repeat_;
        return *this;
    }

    Timer &SetInfinite(bool infinite_) {
        infinite = infinite_;
        return *this;
    }
};