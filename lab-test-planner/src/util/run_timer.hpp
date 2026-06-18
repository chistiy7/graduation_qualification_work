#pragma once

#include <chrono>
#include <iostream>

namespace lab {

class RunTimer {
public:
    [[nodiscard]] double elapsedMs() const {
        using Clock = std::chrono::steady_clock;
        return std::chrono::duration<double, std::milli>(Clock::now() - start_).count();
    }

private:
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
};

inline void printRunTimeMs(double ms) {
    std::cerr << "Время работы: " << ms << " мс\n";
}

}  // namespace lab
