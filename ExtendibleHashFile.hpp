#ifndef EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP
#define EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP

#include <bitset>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>

/*
 * File I/O Macro definitions
 */

#define SAFE_FILE_OPEN(file, file_name, flags)            \
file.open(file_name, flags);                          \
    if (!file.is_open()) {                                \
        throw std::runtime_error("Could not open file."); \
}

#define SAFE_FILE_CREATE_IF_NOT_EXISTS(file, file_name)   \
file.open(file_name, std::ios::app);                  \
    if (!file.is_open()) {                                \
        throw std::runtime_error("Could not open file."); \
}                                                     \
    file.close();

#define SEEK_ALL(file, pos) \
file.seekg(pos);        \
    file.seekp(pos);

#define SEEK_ALL_RELATIVE(file, pos, relative) \
file.seekg(pos, relative);                 \
    file.seekp(pos, relative);

#define TELL(file) file.tellp()

/*
 * Debugging tools
 */

#define PRINT_FLAGS(file) \
std::cout << std::boolalpha << "Good: " << file.good() << " Eof: " << file.eof() << " Bad: " << file.bad() << " Fail: " << file.fail() << std::endl;

#define PRINT_SIZE(T) \
std::cout << "Size: " << sizeof(T) << std::endl;

#define PRINT_TELL(file) \
std::cout << "tellg: " << file.tellg() << " tellp: " << file.tellp() << std::endl;

/*
 * Definitions of constants related to Disk Space Management
 */

#define BLOCK_SIZE 256

/*
 * Each bucket should fit in RAM.
 * Thus, the equation for determining the maximum amount of records per bucket is given by the sum of the size of its attributes:
 * BLOCK_SIZE = sizeof(long) + (MAX_RECORDS_PER_BUCKET * sizeof(RecordType)) + sizeof(long)
 */

template<typename KeyType>
constexpr long MAX_RECORDS_PER_BUCKET = (BLOCK_SIZE - 2 * sizeof(long)) / sizeof(long);

#define MAX_RECORDS_PER_BUCKET MAX_RECORDS_PER_BUCKET<KeyType>


/*
 * Utils
 */

namespace func {

template<typename T>
void copy(T &a, const T &b) {
    std::memcpy((char *) &a, (char *) &b, sizeof(T));
}

template<typename T>
void copy(T &a, T &b) {
    std::memcpy((char *) &a, (char *) &b, sizeof(T));
}

template<typename T>
void copy(T &a, char *&b) {
    std::memcpy((char *) &a, b, sizeof(T));
}

void read_buffer(char buffer[], int size) {
    std::string temp;
    std::getline(std::cin >> std::ws, temp, '\n');
    std::cin.clear();

    for (int i = 0; i < size; ++i) {
        buffer[i] = (i < temp.size()) ? temp[i] : '\0';
    }

    buffer[size - 1] = '\0';
}
}// namespace func


/*
 * Class/Struct definitions
 */

template<typename KeyType>
struct BucketPair {
    KeyType key{};      // < Key that's being indexed
    long record_ref = 0;// < Physical position of the record in the raw data file

    BucketPair() = default;

    /*
     * Constructor.
     * Ensures that a KeyType of char* is properly copied.
     */
    BucketPair(KeyType _key, long _record_ref) : record_ref(_record_ref) {
        func::copy(key, _key);
    }

    /*
     * Copy constructor.
     * Ensures that a KeyType of char* is properly copied.
     */
    BucketPair &operator=(const BucketPair &bucket_pair) {
        func::copy(key, bucket_pair.key);
        record_ref = bucket_pair.record_ref;
        return *this;
    }
};

template<typename KeyType>
struct Bucket {
    long size = 0;                                      // < Stores the real amount of records the bucket holds
    BucketPair<KeyType> records[MAX_RECORDS_PER_BUCKET];// < Stores the data of the records themselves
    long next = -1;                                     // < Stores a reference to the next bucket in the chain (if it exists)
};

template<typename std::size_t D>
struct ExtendibleHashEntry {
    std::size_t local_depth = 1;// < Stores the local depth of the bucket
    char sequence[D + 1] = {};  // < Stores the binary hash sequence
    long bucket_ref = 0;        // < Stores a reference to a page in disk

    ExtendibleHashEntry() = default;

    /*
     * Copy constructor.
     */
    ExtendibleHashEntry(const ExtendibleHashEntry<D> &other) {
        local_depth = other.local_depth;
        std::memcpy(sequence, other.sequence, D + 1);
        bucket_ref = other.bucket_ref;
    }
};

template<typename std::size_t D>
class ExtendibleHash {
    std::vector<ExtendibleHashEntry<D>> hash_entries;// < Vector containing the entries of the hash

public:
    /*
     * Constructs an empty hash with 2 buckets for sequences ending with 0 or 1.
     * The position to the second bucket (bucket_1_ref) is required in order to initalize the second entry of the index.
     */
    explicit ExtendibleHash(long bucket_1_ref) {
        // Initialize an empty index with two entries (the sequences 0...0 and 1...1) at local depth 1 with a reference to the first two buckets of the file
        ExtendibleHashEntry<D> entry_0{};
        std::string empty_sequence_0 = std::bitset<D>(0).to_string();
        std::strcpy(entry_0.sequence, empty_sequence_0.c_str());
        ExtendibleHashEntry<D> entry_1{};
        std::string empty_sequence_1 = std::bitset<D>(1).to_string();
        std::strcpy(entry_1.sequence, empty_sequence_1.c_str());
        entry_1.bucket_ref = bucket_1_ref;
        hash_entries.push_back(entry_0);
        hash_entries.push_back(entry_1);
    }

    /*
     * Constructs a hash from a non-empty index file.
     * Reads the entire file to memory (should fit in RAM).
     * Accesses to disk: O(1)
     */
    explicit ExtendibleHash(std::fstream &index_file) {
        // Get the size of the index file
        SEEK_ALL_RELATIVE(index_file, 0, std::ios::end)
        std::size_t index_file_size = TELL(index_file);
        // Read the entire index file (should fit in RAM)
        SEEK_ALL(index_file, 0)
        char *buffer = new char[index_file_size];
        index_file.read(buffer, (long long) index_file_size);
        // Unpack the binary char buffer
        std::stringstream buf{std::string{buffer, index_file_size}};
        while (!buf.eof()) {
            ExtendibleHashEntry<D> newEntry;
            buf.read((char *) &newEntry, sizeof(newEntry));
            if (!buf.eof()) {
                hash_entries.push_back(newEntry);
            }
        }
        delete[] buffer;
    }

    /*
     * Writes the entire index to disk (overwrites the actual contents of the file).
     * Accesses to disk: O(1)
     */
    void write_to_disk(std::fstream &index_file) {
        const std::size_t index_size = hash_entries.size() * sizeof(ExtendibleHashEntry<D>);
        char *buffer = new char[index_size];
        // Pack the binary char buffer
        std::stringstream buf{std::string{buffer, index_size}};
        for (std::size_t i = 0; i < hash_entries.size(); ++i) {
            buf.write((char *) &(hash_entries[i]), sizeof(hash_entries[i]));
        }
        // Write buffer to disk
        index_file.write(buf.str().c_str(), (long long) index_size);
        delete[] buffer;
    }

    /*
     * Looks for an entry.
     * Returns a pair containing the position of the entry (first), and the bucket it references (second)
     */
    std::pair<std::size_t, long> lookup(const std::string &hash_sequence) {
        for (std::size_t i = 0; i < hash_entries.size(); ++i) {
            auto local_depth = hash_entries[i].local_depth;
            bool eq = true;
            for (int j = 0; j < local_depth; ++j) {
                // If the sequences are different given the local depth, this is not the bucket we're looking for
                if (hash_sequence[D - 1 - j] != hash_entries[i].sequence[D - 1 - j]) {
                    eq = false;
                    break;
                }
            }
            if (eq) {
                return std::make_pair(i, hash_entries[i].bucket_ref);
            }
        }
        throw std::runtime_error("Could not find given hash sequence on ExtendibleHash.");
    }

    void update_entry_bucket(const std::size_t &entry_index, const long &new_bucket_ref) {
        hash_entries[entry_index].bucket_ref = new_bucket_ref;
    }

    /*
     * Splits an entry.
     * Returns a pair containing a bool which indicates if the split was successful (first), and a number indicating the old local depth (second).
     */
    std::pair<bool, std::size_t> split_entry(const std::size_t &entry_index, const long &new_bucket_ref) {
        std::size_t local_depth = hash_entries[entry_index].local_depth;
        if (local_depth < D) {
            // Create a copy of the actual entry
            ExtendibleHashEntry<D> entry_1{hash_entries[entry_index]};
            // Update the sequence to have a 1 at the local_depth position (from right to left)
            entry_1.sequence[D - 1 - local_depth] = '1';
            // Reference the new bucket's position
            entry_1.bucket_ref = new_bucket_ref;
            // Increase the local depth
            hash_entries[entry_index].local_depth++;
            entry_1.local_depth++;
            // Add the new entry to the index
            hash_entries.push_back(entry_1);
            return std::make_pair(true, local_depth);
        } else {
            return std::make_pair(false, 0);
        }
    }
};


template<typename KeyType,
         typename RecordType,
         std::size_t global_depth = 16,                        // < Maximum depth of the binary index key (defaults to 16)
         typename Index = std::function<KeyType(RecordType &)>,// < Indexing function type
         typename Equal = std::equal_to<KeyType>,              // < Equal comparator type
         typename Hash = std::hash<KeyType>                    // < Hash type
         >
class ExtendibleHashFile {
    std::fstream raw_file;                                                                //< File object used to manage acces to the raw data file (not used if index is already created)
    std::string raw_file_name;                                                            //< Raw data file name
    std::fstream index_file;                                                              // < File object used to manage the index
    std::string index_file_name;                                                          //< Name of index raw_file to be created
    std::fstream hash_file;                                                               // < File object used to access hash-based indexed file
    std::string hash_file_name;                                                           // < Hash-based indexed file name
    std::string unique_id;                                                                // < Index unique identifier (allows to create indexes in more than 1 attribute per table)
    const std::ios_base::openmode flags = std::ios::in | std::ios::binary | std::ios::out;// < Flags used in all accesses to disk

    /*
     * Generic purposes member variables
     */
    bool primary_key;                        //< Is `true` when indexing a primary key and `false` otherwise
    Index index;                             //< Receives a `RecordType` and returns his `KeyType` associated
    Equal equal;                             //< Returns `true` if the first parameter is greater than the second and `false` otherwise
    Hash hash_function;                      // < Hash function
    ExtendibleHash<global_depth> *hash_index = nullptr; // < Extendible hash index (stored in RAM)

    /*
     * Returns a binary sequence of the hash key.
     */
    std::string get_hash_sequence(KeyType key) {
        auto hash_key = hash_function(key);
        auto bit_set = std::bitset<global_depth>{hash_key % (1 << global_depth)};
        return bit_set.to_string();
    }

    /*
     * Auxiliary method for ensuring primary key consistency.
     * Assumes necessary files are already open.
     * Finds if a given key already exists on the index.
     * Returns true if the key is found, and false otherwise.
     * Accesses to disk: O(k)
     */
    bool _find_if_exists(KeyType key) {
        std::string hash_sequence = get_hash_sequence(key);
        auto [entry_index, bucket_ref] = hash_index->lookup(hash_sequence);
        // Read bucket at position bucket_ref
        SEEK_ALL(hash_file, bucket_ref)
        Bucket<KeyType> bucket{};
        hash_file.read((char *) &bucket, sizeof(bucket));
        // Search in chain of buckets
        while (true) {
            for (int i = 0; i < bucket.size; ++i) {
                if (equal(key, bucket.records[i].key)) {
                    return true;
                }
            }
            // If there is a next bucket, explore it
            if (bucket.next != -1) {
                SEEK_ALL(hash_file, bucket.next)
                hash_file.read((char *) &bucket, sizeof(bucket));
            } else {
                break;
            }
        }
        return false;
    }


    /*
     * Insertion algorithm.
     * Auxiliary method that avoids excessive file opening and closing when inserting.
     * Assumes necessary files are already open.
     * When overflow happens, a new bucket is pushed to the front of the overflow chain and linked, to allow for more efficient insertions.
     * Throws an exception if the key of the record to be inserted is already present and the index is for a primary key.
     * Accesses to disk: O(k + global_depth) where k is the number of buckets in an overflow chain,
     * and global_depth is the maximum depth of the index (number of bits in the binary sequences).
     */
    void _insert(RecordType &record, const long &record_ref) {
        // If the attribute is a primary key, we must check whether a record with the given key already exists
        if (primary_key && _find_if_exists(index(record))) {
            throw std::runtime_error("Cannot insert a duplicate primary key.");
        }
        std::string hash_sequence = get_hash_sequence(index(record));
        auto [entry_index, bucket_ref] = hash_index->lookup(hash_sequence);
        // Insert record into bucket bucket_ref of the hash file
        SEEK_ALL(hash_file, bucket_ref)
        // Read and update bucket bucket_ref if it's not full
        Bucket<KeyType> bucket{};
        hash_file.read((char *) &bucket, sizeof(bucket));
        if (bucket.size < MAX_RECORDS_PER_BUCKET) {
            // Append record
            bucket.records[bucket.size++] = BucketPair<KeyType>{index(record), record_ref};
            // Write bucket bucket_ref
            SEEK_ALL(hash_file, bucket_ref)
            hash_file.write((char *) &bucket, sizeof(bucket));
        } else {
            // Create new buckets and split hash index if possible
            Bucket<KeyType> bucket_0{};
            Bucket<KeyType> bucket_1{};
            SEEK_ALL_RELATIVE(hash_file, 0, std::ios::end)
            // Split the current hash entry and save a pointer to the end of the file (new bucket position)
            auto [could_split, local_depth] = hash_index->split_entry(entry_index, TELL(hash_file));
            // Split was successful, rehash the current content of the bucket into the two new buckets
            if (could_split) {
                for (int i = 0; i < bucket.size; ++i) {
                    std::string ith_hash_seq = get_hash_sequence(bucket.records[i].key);
                    if (ith_hash_seq[global_depth - 1 - local_depth] == '0') {
                        bucket_0.records[bucket_0.size++] = bucket.records[i];
                    } else {
                        bucket_1.records[bucket_1.size++] = bucket.records[i];
                    }
                }
                if (bucket_0.size != MAX_RECORDS_PER_BUCKET && bucket_1.size != MAX_RECORDS_PER_BUCKET) {
                    // Insert the new record
                    if (hash_sequence[global_depth - 1 - local_depth] == '0') {
                        bucket_0.records[bucket_0.size++] = BucketPair<KeyType>{index(record), record_ref};
                    } else {
                        bucket_1.records[bucket_1.size++] = BucketPair<KeyType>{index(record), record_ref};
                    }
                    // Write the two new buckets to secondary storage
                    SEEK_ALL(hash_file, bucket_ref)
                    hash_file.write((char *) &bucket_0, sizeof(bucket_0));
                    SEEK_ALL_RELATIVE(hash_file, 0, std::ios::end)
                    hash_file.write((char *) &bucket_1, sizeof(bucket_1));
                } else {
                    // Write the two new buckets to secondary storage
                    SEEK_ALL(hash_file, bucket_ref)
                    hash_file.write((char *) &bucket_0, sizeof(bucket_0));
                    SEEK_ALL_RELATIVE(hash_file, 0, std::ios::end)
                    hash_file.write((char *) &bucket_1, sizeof(bucket_1));
                    // Insert new record recursively (could not insert it in the current split)
                    _insert(record, record_ref);
                    return;
                }
            }
            // Split was unsuccessful. Create a new bucket.
            else {
                // Create new bucket
                SEEK_ALL_RELATIVE(hash_file, 0, std::ios::end)
                bucket_0.records[bucket_0.size++] = BucketPair<KeyType>{index(record), record_ref};
                // Reference the parent (push front)
                bucket_0.next = bucket_ref;
                long new_bucket_ref = TELL(hash_file);
                hash_file.write((char *) &bucket_0, sizeof(bucket_0));
                // Put reference to the new bucket in the directory
                hash_index->update_entry_bucket(entry_index, new_bucket_ref);
            }
        }
    }

public:
    explicit ExtendibleHashFile(const std::string &fileName, const std::string &uniqueId, bool primaryKey, Index index, Equal equal = std::equal_to<KeyType>{}, Hash hash = std::hash<KeyType>{}) : raw_file_name(fileName), primary_key(primaryKey), unique_id(uniqueId), index(index), equal(equal), hash_function(hash) {
        hash_file_name = raw_file_name + "_" + unique_id + ".ehash";
        index_file_name = raw_file_name + "_" + unique_id + ".ehashdir";
        SAFE_FILE_CREATE_IF_NOT_EXISTS(index_file, index_file_name)
        SAFE_FILE_OPEN(index_file, index_file_name, flags)
        if (index_file.peek() != std::ifstream::traits_type::eof()) {
            hash_index = new ExtendibleHash<global_depth>{index_file};
        }
        index_file.close();
    }


    /*
     * Returns a bool that indicates whether the index has already been created.
     */
    explicit operator bool() {
        SAFE_FILE_OPEN(index_file, index_file_name, flags)
        bool is_created = false;
        if (index_file.peek() != std::ifstream::traits_type::eof()) {
            is_created = true;
        }
        index_file.close();
        return is_created;
    }


    /*
     * Constructs the hash index file from a fixed length binary data file.
     * It creates 2 files: The directory file (.ehashdir) and the hash index (.ehash).
     * Accesses to disk: O(n) where n is the total number of records in the data file.
     */
    void create_index() {
        SAFE_FILE_CREATE_IF_NOT_EXISTS(hash_file, hash_file_name)
        SAFE_FILE_OPEN(hash_file, hash_file_name, flags)
        SAFE_FILE_OPEN(raw_file, raw_file_name, flags)
        SAFE_FILE_OPEN(index_file, index_file_name, flags | std::ios::trunc)
        SEEK_ALL(hash_file, 0)
        SEEK_ALL(raw_file, 0)
        SEEK_ALL(index_file, 0)
        Bucket<KeyType> bucket_0{};
        Bucket<KeyType> bucket_1{};
        hash_index = new ExtendibleHash<global_depth>{sizeof(bucket_0)};
        hash_file.write((char *) &bucket_0, sizeof(bucket_0));
        hash_file.write((char *) &bucket_1, sizeof(bucket_1));
        // Construct hash file (.ehash)
        RecordType record{};
        while (!raw_file.eof()) {
            long record_ref = TELL(raw_file);
            raw_file.read((char *) &record, sizeof(record));
            if (!raw_file.eof()) {
                if (!record.removed) {
                    _insert(record, record_ref);
                }
            }
        }
        hash_index->write_to_disk(index_file);
        raw_file.close();
        hash_file.close();
        index_file.close();
    }


    /*
     * Searches a given key.
     * Returns a vector of elements that match the given key.
     * If the index was created for primary keys, it returns a single element.
     * If no element matches the given key, it returns an empty vector.
     * Accesses to disk: O(k) where k is the length of the bucket chain accessed
     */
    std::vector<RecordType> search(KeyType key) {
        SAFE_FILE_OPEN(hash_file, hash_file_name, flags)
        SAFE_FILE_OPEN(raw_file, raw_file_name, flags)
        std::vector<RecordType> result;
        std::string hash_sequence = get_hash_sequence(key);
        auto [entry_index, bucket_ref] = hash_index->lookup(hash_sequence);
        // Read bucket at position bucket_ref
        SEEK_ALL(hash_file, bucket_ref)
        Bucket<KeyType> bucket{};
        hash_file.read((char *) &bucket, sizeof(bucket));
        // Search in chain of buckets
        bool stop = false;
        while (!stop) {
            for (int i = 0; i < bucket.size; ++i) {
                if (equal(key, bucket.records[i].key)) {
                    // Found record. Add to result
                    SEEK_ALL(raw_file, bucket.records[i].record_ref)
                    RecordType record{};
                    raw_file.read((char *) &record, sizeof(record));
                    if (!record.removed) {
                        result.push_back(record);
                    }
                    // If primary key, stop searching
                    if (primary_key) {
                        stop = true;
                        break;
                    }
                }
            }
            // If there is a next bucket, explore it
            if (bucket.next != -1) {
                SEEK_ALL(hash_file, bucket.next)
                hash_file.read((char *) &bucket, sizeof(bucket));
            } else {
                break;
            }
        }
        hash_file.close();
        raw_file.close();
        return result;
    }


    /*
     * Inserts a given key in the hash index.
     * When overflow happens, a new bucket is pushed to the front of the overflow chain and linked, to allow for more efficient insertions.
     * Throws an exception if the key of the record to be inserted is already present and the index is for a primary key.
     * Accesses to disk: O(k + global_depth) where k is the number of buckets in an overflow chain,
     * and global_depth is the maximum depth of the index (number of bits in the binary sequences).
     */
    void insert(RecordType &record, const long &record_ref) {
        SAFE_FILE_OPEN(hash_file, hash_file_name, flags)
        SAFE_FILE_OPEN(index_file, hash_file_name, flags | std::ios::trunc)
        _insert(record, record_ref);
        hash_index->write_to_disk(index_file);
        hash_file.close();
        index_file.close();
    }


    /*
     * Removes every record that matches the given key by marking it as removed on the data file.
     * Does nothing if the key does not exist.
     * Accesses to disk: O(k) where k is the length of the bucket chain accessed.
     */
    void remove(KeyType key) {
        SAFE_FILE_OPEN(hash_file, hash_file_name, flags)
        SAFE_FILE_OPEN(raw_file, raw_file_name, flags)
        std::string hash_sequence = get_hash_sequence(key);
        auto [entry_index, bucket_ref] = hash_index->lookup(hash_sequence);
        // Read bucket at position bucket_ref
        SEEK_ALL(hash_file, bucket_ref)
        Bucket<KeyType> bucket{};
        hash_file.read((char *) &bucket, sizeof(bucket));
        // Search in chain of buckets
        long current_bucket_ref = bucket_ref;
        while (true) {
            for (int i = bucket.size - 1; i >= 0; --i) {
                if (equal(key, bucket.records[i].key)) {
                    // Mark record as deleted in the data file.
                    long record_ref = bucket.records[i].record_ref;
                    SEEK_ALL(raw_file, record_ref)
                    RecordType record{};
                    raw_file.read((char *) &record, sizeof(record));
                    record.removed = true;
                    SEEK_ALL(raw_file, record_ref)
                    raw_file.write((char *) &record, sizeof(record));
                    // If primary key, stop searching
                    if (primary_key) {
                        return;
                    }
                }
            }
            // If there is a next bucket, explore it
            if (bucket.next != -1) {
                current_bucket_ref = bucket.next;
                SEEK_ALL(hash_file, bucket.next)
                hash_file.read((char *) &bucket, sizeof(bucket));
            } else {
                break;
            }
        }
        hash_file.close();
        raw_file.close();
    }

    virtual ~ExtendibleHashFile() {
        if (hash_index != nullptr) {
            delete hash_index;
        }
    }
};


#endif//EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP
