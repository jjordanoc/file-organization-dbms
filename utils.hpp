#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace func {

template<typename T>
void copy(T &a, const T &b)
{
    std::memcpy((char *) &a, (char *) &b, sizeof(T));
}

template<typename T>
void copy(T &a, T &b)
{
    std::memcpy((char *) &a, (char *) &b, sizeof(T));
}

template<typename T>
void copy(T &a, char *&b)
{
    std::memcpy((char *) &a, b, sizeof(T));
}

} // namespace func

template<typename Return>
struct TimedResult
{
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

template<typename KeyType, typename RecordType, typename Index, typename Equal = std::equal_to<KeyType>>
std::vector<RecordType> linear_search(std::string filename,
                                      KeyType key,
                                      Index index,
                                      Equal equal = std::equal_to<KeyType>{})
{
    std::vector<RecordType> result;
    std::fstream file(filename, std::ios::in | std::ios::binary);
    while (!file.eof()) {
        RecordType tmp{};
        file.read((char *) &tmp, sizeof(tmp));
        if (!file.eof() && equal(index(tmp), key) && !tmp.removed) {
            result.push_back(tmp);
        }
    }
    file.close();
    return result;
}

template<typename KeyType, typename RecordType, typename Index, typename Greater = std::greater<KeyType>>
std::vector<RecordType> range_search(std::string filename,
                                     KeyType start,
                                     KeyType end,
                                     Index index,
                                     Greater greater = std::greater<KeyType>{})
{
    std::vector<RecordType> result;
    std::fstream file(filename, std::ios::in | std::ios::binary);
    while (!file.eof()) {
        RecordType tmp{};
        file.read((char *) &tmp, sizeof(tmp));
        if (!file.eof() && !greater(start, index(tmp)) && !greater(index(tmp), end)
            && !tmp.removed) {
            result.push_back(tmp);
        }
    }
    file.close();
    return result;
}

template<typename RecordType>
long insert_register_on_file(std::string filename, RecordType &record)
{
    std::fstream file(filename, std::ios::out | std::ios::binary | std::ios::in);
    file.seekp(0, std::ios::end);
    long pos = file.tellp();
    file.write((char *) &record, sizeof(record));
    file << std::flush;
    file.close();
    return pos;
}

template<typename KeyType, typename RecordType, typename Index, typename Equal = std::equal_to<KeyType>>
void linear_delete(std::string filename,
                   KeyType key,
                   Index index,
                   Equal equal = std::equal_to<KeyType>{})
{
    std::fstream file(filename, std::ios::in | std::ios::binary | std::ios::out);
    while (!file.eof()) {
        RecordType tmp{};
        long pos = file.tellg();
        file.read((char *) &tmp, sizeof(tmp));
        if (!file.eof() && equal(index(tmp), key)) {
            file.seekp(pos);
            tmp.removed = true;
            file.write((char *) &tmp, sizeof(tmp));
        }
    }
    file.close();
}

#endif
