//
// Created by juandiego on 5/2/23.
//

#ifndef ISAM_PAGES_HPP
#define ISAM_PAGES_HPP

#include "utils.hpp"

#define SIZE(T) sizeof(T)

#define DISK_NULL (-1)

#define BUFFER_SIZE 239

#define SEEK_ALL(file, pos) \
    file.seekg(pos);        \
    file.seekp(pos);

#define SEEK_ALL_RELATIVE(file, pos, relative) \
    file.seekg(pos, relative);                 \
    file.seekp(pos, relative);

/* `M` stores the number of keys in an index page
*
* BUFFER = (M * key) + (M + 1) * long + int
* BUFFER = (M * key) + (M * long) + long + int
* BUFFER = M * (key + long) + long_size + int
* BUFFER - long - int = M * (key + long)
* M = (BUFFER - long - int) / (key + long) */
template<typename KeyType>
inline static constexpr int M = (BUFFER_SIZE - SIZE(int) - SIZE(int64_t))
                                / (SIZE(int64_t) + SIZE(KeyType));

template<typename KeyType>
struct IndexPage
{
    int n_keys{0};                      //< number of keys in the index page
    KeyType keys[M<KeyType>]{};         //< keys array
    int64_t children[M<KeyType> + 1]{}; //< children physical position array

    int64_t locate(KeyType key, std::function<bool(KeyType, KeyType)> greater)
    {
        int i = 0;
        for (; ((i < n_keys) && !greater(keys[i], key)); ++i)
            ;
        return children[i];
    }
};

//`N` stores the maximum number of records per database page
// It is calculated with the same logic as with `M`.
template<typename RecordType>
inline static constexpr int N = (BUFFER_SIZE - SIZE(int64_t) - SIZE(int)) / SIZE(RecordType);

template<typename RecordType>
struct DataPage
{
    RecordType records[N<RecordType>];
    int64_t next{-1};
    int n_records{0};
};

template<typename KeyType>
struct Pair
{
    KeyType key{KeyType()};
    int64_t data_pointer;

    Pair()
        : data_pointer(DISK_NULL)
    {}

    Pair(KeyType _key, int64_t pointer)
        : data_pointer(pointer)
    {
        func::copy(key, _key);
    }
};

#endif //ISAM_PAGES_HPP
