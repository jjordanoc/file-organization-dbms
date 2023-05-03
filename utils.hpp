#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>

template<typename Return>
struct TimedResult {
    Return result;
    double duration;
    TimedResult(Return &_result, double &_duration) : result(_result), duration(_duration) {}
};

template<>
struct TimedResult<void> {
    double duration;
    TimedResult(double &_duration) : duration(_duration) {}
};

template<typename Return, typename Fun, typename ...Args>
TimedResult<Return> time_function(Fun &function, Args... args) {
    auto start = std::chrono::steady_clock::now();
    Return result = function(args...);
    auto end = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return {result, duration};
}

template<typename Fun, typename ...Args>
TimedResult<void> time_function(Fun &function, Args... args) {
    auto start = std::chrono::steady_clock::now();
    function(args...);
    auto end = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return {duration};
}

#endif
