//
// Created by juandiego on 5/1/23.
//

#ifndef PROJECT_DATASET_ISAM_HPP
#define PROJECT_DATASET_ISAM_HPP

#include "pages.hpp"

#define INT_POW(BASE, EXP) (static_cast<int>(std::pow(BASE, EXP)))

#define OPEN_FILES(MODE)                        \
    index_file1.open(index_file_name(1), MODE); \
    index_file2.open(index_file_name(2), MODE); \
    index_file3.open(index_file_name(3), MODE); \
    data_file.open(data_file_name, MODE)

#define CLOSE_FILES      \
    index_file1.close(); \
    index_file2.close(); \
    index_file3.close(); \
    data_file.close()

template<bool PrimaryKey,
        typename KeyType,
        typename RecordType,
        typename Index = std::function<KeyType(RecordType &)>,
        typename Greater = std::greater<KeyType>>
class ISAM {
private:

    /* Data pages are stored @ the last level */
    std::fstream data_file;
    std::string data_file_name; //< The name of the last level file
    std::string heap_file_name; //< The name of the heap file
    std::string attribute;      //< The name of the indexing attribute

    /* Three levels of index files that stores index pages */
    std::fstream index_file1;
    std::fstream index_file2;
    std::fstream index_file3;

    std::function<std::string(int)> index_file_name = [&](int i) {
        return heap_file_name + "_" + attribute + "_index_" + std::to_string(i) + ".isam";
    }; //< Gets the name of the ith index file

    std::ios::openmode flags = std::ios_base::in | std::ios_base::out | std::ios_base::binary;  //< Open mode flags

    Index index;        //< Receives a `RecordType` and returns its `KeyType` associated
    Greater greater;    //< Receives two `RecordTypes` and returns `true` if a.key > b.key and `false` otherwise

    long locate(KeyType key) {
        /************************************** third (top) index level  **********************************************/
        IndexPage<KeyType> index1;
        index_file1.seekg(std::ios::beg);
        index_file1.read((char *) &index1, SIZE(IndexPage<KeyType>));//< loads the unique index1 page in RAM
        long descend_to_index_2 = index1.locate(key, greater);
        //^ searches the physical pointer to descend to the second level

        /************************************** second index level ****************************************************/
        IndexPage<KeyType> index2;
        index_file2.seekg(descend_to_index_2);
        index_file2.read((char *) &index2, SIZE(IndexPage<KeyType>));//< loads an index2 page in RAM
        long descend_to_index_3 = index2.locate(key, greater);
        //^ searches the pointer to descend to the last index level

        /************************************** first (base) index level **********************************************/
        IndexPage<KeyType> index3;
        index_file3.seekg(descend_to_index_3);
        index_file3.read((char *) &index3, SIZE(IndexPage<KeyType>));//< loads an index3 page in RAM
        long page_position = index3.locate(key, greater);
        //^ searches the physical pointer to descend to a leaf page
        return page_position;
    }

    void init_data_pages(std::vector<std::pair<RecordType, long>> &sorted_records, int n_pages) {
        int l3 = 0, l2 = 0, l1 = 0;
        int k3 = 0, k2 = 0, k1 = 0;

        IndexPage<KeyType> level3_index_page;
        IndexPage<KeyType> level2_index_page;
        IndexPage<KeyType> level1_index_page;

        for (int i = 0; i < n_pages; ++i) {
            DataPage<Pair<KeyType>> data_page;
            for (int j = 0; j < N<Pair<KeyType>>; ++j) {
                std::pair<RecordType, long> pair = sorted_records[(i * N<Pair<KeyType>>) + j];
                data_page.records[j] = Pair<KeyType>(index(pair.first), pair.second);
            }
            data_page.n_records = N<Pair<KeyType>>;
            data_file.write((char *) &data_page, SIZE(DataPage<Pair<KeyType>>));

            /* This part of the code stores the key of the first record of each database page in vectors in order to  */
            /* assign the keys in each index level later in the process of ISAM-tree initialization.                  */

            if (i == 0) {//< Skips the first database page (not belongs to any key level)
                continue;
            }

            auto key = data_page.records[0].key;

            if (l3 < M<KeyType>) {
                func::copy(level3_index_page.keys[l3], key);
                level3_index_page.children[l3] = (k3++) * SIZE(DataPage<Pair<KeyType>>);
                ++l3;
            } else {
                level3_index_page.children[l3] = (k3++) * SIZE(DataPage<Pair<KeyType>>);
                level3_index_page.n_keys = M<KeyType>;
                index_file3.write((char *) &level3_index_page, SIZE(IndexPage<KeyType>));
                l3 = 0;

                if (l2 < M<KeyType>) {
                    func::copy(level2_index_page.keys[l2], key);
                    level2_index_page.children[l2] = (k2++) * SIZE(IndexPage<KeyType>);
                    ++l2;
                } else {
                    level2_index_page.children[l2] = (k2++) * SIZE(IndexPage<KeyType>);
                    level2_index_page.n_keys = M<KeyType>;
                    index_file2.write((char *) &level2_index_page, SIZE(IndexPage<KeyType>));
                    l2 = 0;

                    func::copy(level1_index_page.keys[l1], key);
                    level1_index_page.children[l1] = (k1++) * SIZE(IndexPage<KeyType>);
                    ++l1;
                }
            }
        }

        level3_index_page.children[l3] = k3 * SIZE(DataPage<Pair<KeyType>>);
        level3_index_page.n_keys = M<KeyType>;
        index_file3.write((char *) &level3_index_page, SIZE(IndexPage<KeyType>));

        level2_index_page.children[l1] = k2 * SIZE(IndexPage<KeyType>);
        level2_index_page.n_keys = M<KeyType>;
        index_file2.write((char *) &level2_index_page, SIZE(IndexPage<KeyType>));

        level1_index_page.children[l1] = k1 * SIZE(IndexPage<KeyType>);
        level1_index_page.n_keys = M<KeyType>;
        index_file1.write((char *) &level1_index_page, SIZE(IndexPage<KeyType>));
    }

    void locate_insertion_pk(long seek, KeyType key, long &non_full, long &prev) {
        DataPage<Pair<KeyType>> page;

        do {
            data_file.seekg(seek);
            data_file.read((char *) &page, SIZE(DataPage<Pair<KeyType>>));

            for (int i = 0; i < page.n_records; ++i) {
                if (!greater(page.records[i].key, key) && !greater(key, page.records[i].key)) {
                    CLOSE_FILES;
                    throw std::invalid_argument("Error: ISAM cannot insert a repeated primary key.");
                }
            }

            non_full = (page.n_records < N<Pair<KeyType>> && non_full == DISK_NULL) ? seek : non_full;
            prev = seek;
            seek = page.next;
        } while (seek != DISK_NULL);
    }

    void locate_insertion_non_pk(long seek, long &non_full, long &prev) {
        DataPage<Pair<KeyType>> page;

        do {
            data_file.seekg(seek);
            data_file.read((char *) &page, SIZE(DataPage<Pair<KeyType>>));

            if (page.n_records < N<Pair<KeyType>>) {
                non_full = seek;
                return;
            }

            prev = seek;
            seek = page.next;
        } while (seek != DISK_NULL);
    }

    void search(KeyType key, std::vector<long> &pointers) {
        // locates the physical position of the database page where the record to be searched is
        long seek = this->locate(key);
        DataPage<Pair<KeyType>> page;
        do {
            data_file.seekg(seek);
            data_file.read((char *) &page, SIZE(DataPage<Pair<KeyType>>));
            for (int i = 0; i < page.n_records; ++i) {  //< iterates the leaf records in the current page
                if (!greater(page.records[i].key, key) && !greater(key, page.records[i].key)) {
                    // if the `record.key` equals `key`, pushes to the `vector`
                    pointers.push_back(page.records[i].data_pointer);
                    if (PrimaryKey) {// if indexing a primary-key, it breaks the loop (the unique record was found).
                        return;
                    }
                }
            }
            seek = page.next; //< `page.next` probably points to an overflow page (or to nothing)
        } while (seek != DISK_NULL);
    }

    void range_search(KeyType lower_bound, KeyType upper_bound, std::vector<long> &pointers) {
        // locates the physical position of the database page where the `lower_bound` is located
        long seek = this->locate(lower_bound);
        long n_static_pages = INT_POW(M<KeyType> + 1, 3);

        DataPage<Pair<KeyType>> page;
        bool any_found;

        do {
            any_found = false;
            data_file.seekg(seek);
            data_file.read((char *) &page, SIZE(DataPage<Pair<KeyType>>));

            for (int i = 0; i < page.n_records; ++i) {
                if (!greater(lower_bound, page.records[i].key) && !greater(page.records[i].key, upper_bound)) {
                    pointers.push_back(page.records[i].data_pointer);
                    any_found = true;
                }
            }

            DataPage<Pair<KeyType>> overflow;
            long seek_overflow = page.next;
            while (seek_overflow != DISK_NULL) {
                data_file.seekg(seek_overflow);
                data_file.read((char *) &overflow, SIZE(DataPage<Pair<KeyType>>));

                for (int j = 0; j < overflow.n_records; ++j) {
                    if (!greater(lower_bound, overflow.records[j].key) &&
                        !greater(overflow.records[j].key, upper_bound)) {
                        pointers.push_back(overflow.records[j].data_pointer);
                        any_found = true;
                    }
                }
                seek_overflow = overflow.next;
            }

            seek += SIZE(DataPage<Pair<KeyType>>);
        } while (any_found && (seek != n_static_pages * SIZE(DataPage<Pair<KeyType>>)));
    }

    void search_records(std::vector<long> &pointers, std::vector<RecordType> &records) {
        records.reserve(pointers.size());
        std::fstream heap_file(heap_file_name, flags);
        for (long pointer: pointers) {
            RecordType record;
            heap_file.seekg(pointer);
            heap_file.read((char *) &record, SIZE(RecordType));
            if (!record.removed) {
                records.push_back(record);
            }
        }
        heap_file.close();
    }

    void remove_records(std::vector<long> &pointers) {
        std::fstream heap_file(heap_file_name, flags);
        for (long pointer: pointers) {
            RecordType record;
            heap_file.seekg(pointer);
            heap_file.read((char *) &record, SIZE(RecordType));
            if (!record.removed) {
                record.removed = true;
                heap_file.seekp(pointer);
                heap_file.write((char *) &record, SIZE(RecordType));
            }
        }
        heap_file.close();
    }

public:

    explicit ISAM(const std::string &heap_file_name, std::string _attribute, Index index, Greater greater = Greater())
            : heap_file_name(heap_file_name), index(index),
              greater(greater), attribute(std::move(_attribute)) {
        data_file_name = heap_file_name + "_" + attribute + ".isam";
        OPEN_FILES(std::ios::app | std::ios::binary);
        CLOSE_FILES;
    }

    ISAM() = default;

    /******************************************************************************************************************/
    /* This member function meets the following requirements                                                          */
    /*   • It must be called once, since it initializes the tree structure levels, which are static.                  */
    /*   • It assumes that the `sorted_file` has exactly N * (M+1)^3 records, in order to generate a full ISAM-tree   */
    /******************************************************************************************************************/
    void create_index() {
        drop_index(); // removes all the previous information in the index

        std::fstream heap_file(heap_file_name, flags);
        if (!heap_file.is_open()) {
            throw std::runtime_error("Cannot open the file: " + heap_file_name);
        }

        SEEK_ALL_RELATIVE(heap_file, 0, std::ios::end)
        long n_bytes = heap_file.tellg();
        SEEK_ALL(heap_file, 0);

        int n_total_records = n_bytes / SIZE(RecordType); // Total number of records in the heap file

        int max_n_children = M<KeyType> + 1;        //< Maximum number of children per index page
        int n_pages = INT_POW(max_n_children, 3);   //< Total number of record pages in full ISAM-tree
        int n_records = N<Pair<KeyType>> * n_pages; //< Total number of records in full ISAM-tree

        if (n_total_records < n_records) {
            throw std::runtime_error("The #N of records in " + heap_file_name + " are less than the minimum required.");
        }

        std::vector<std::pair<RecordType, long>> sorted_data;
        sorted_data.reserve(n_records);

        long seek = 0;

        for (int i = 0; i < n_records; ++i) {
            RecordType record;
            heap_file.read((char *) &record, SIZE(RecordType));
            sorted_data.push_back(std::make_pair(record, seek));
            seek = heap_file.tellg();
        }

        std::sort(sorted_data.begin(), sorted_data.end(),
                  [&](std::pair<RecordType, long> &a, std::pair<RecordType, long> &b) {
                      return !greater(index(a.first), index(b.first));
                  });

        OPEN_FILES(std::ios::app);

        // Creates the index pages for each level and fills them with his correspondents keys
        init_data_pages(sorted_data, n_pages);
        /* index1:                      [k_1          k_2           ...         k_{M-1}             k_M]
         *                             /               |            ...            |                    \
         * index2:           [x < k_1]          [k_1 <= x < k_2]    ...     [k_{M-2} <= x < k_{M-1}]    [x >= k_M]
         *                 /     ...    \        /    ...    \      ...    /    ...    \               /    ...   \
         * index3:      [.]      ...     [.]   [.]    ...     [.]   ...  [.]    ...     [.]         [.]     ...    [.]
         *             /  \      ...    /  \  /  \    ...    /  \       /  \    ...    /  \        /  \     ...   /  \
         * database:  [*][*]     ...   [*][*][*][*]   ...   [*][*]     [*][*]   ...   [*][*]      [*][*]    ...  [*][*]
         */
        CLOSE_FILES;

        // Inserts the remaining records
        RecordType record;
        while (heap_file.read((char *) &record, SIZE(RecordType))) {
            insert(index(record), seek);
            seek = heap_file.tellg();
        }

        /* index1:                      [k_1          k_2           ...         k_{M-1}             k_M]
         *                             /               |            ...            |                    \
         * index2:           [x < k_1]          [k_1 <= x < k_2]    ...     [k_{M-2} <= x < k_{M-1}]    [x >= k_M]
         *                 /     ...    \        /    ...    \      ...    /    ...    \               /    ...   \
         * index3:      [.]      ...     [.]   [.]    ...     [.]   ...  [.]    ...     [.]         [.]     ...    [.]
         *             /  \      ...    /  \  /  \    ...    /  \       /  \    ...    /  \        /  \     ...   /  \
         * database:  [*][*]     ...   [*][*][*][*]   ...   [*][*]     [*][*]   ...   [*][*]      [*][*]    ...  [*][*]
         * overflow  ^[*][*]          ^[*][*]                         ^[*][*]                                   ^[*][*]
         * pages:    ^[*][*]                                          ^[*][*]
         */
        heap_file.close();
        CLOSE_FILES;
    }

    void drop_index() {
        OPEN_FILES(std::ios::out);
        CLOSE_FILES;
    }

    explicit operator bool() {
        OPEN_FILES(std::ios::in);
        SEEK_ALL_RELATIVE(index_file1, 0, std::ios::end)
        long size = index_file1.tellg();
        CLOSE_FILES;
        return (size > 0);
    }

    std::vector<RecordType> search(KeyType key) {
        std::vector<long> pointers;

        OPEN_FILES(flags);
        this->search(key, pointers);
        CLOSE_FILES;

        std::vector<RecordType> records;
        search_records(pointers, records);
        return records;
    }

    std::vector<RecordType> range_search(KeyType lower_bound, KeyType upper_bound) {
        std::vector<long> pointers;

        OPEN_FILES(flags);
        this->range_search(lower_bound, upper_bound, pointers);
        CLOSE_FILES;

        std::vector<RecordType> records;
        search_records(pointers, records);
        return records;
    }

    void remove(KeyType key) {
        std::vector<long> pointers;

        OPEN_FILES(flags);
        this->search(key, pointers);
        CLOSE_FILES;

        remove_records(pointers);
    }

    /******************************************************************************************************************/
    /* This member function makes the following assumption                                                            */
    /*   • The three indexing levels are already created, so the criteria for descending in the tree is well-defined. */
    /*                                                                                                                */
    /* In addition, by ISAM definition we have                                                                        */
    /*   • The three indexing levels are totally static, so the tree do not admit the split of indexing pages.        */
    /*   • If the selected insertion page is full, a database page split occurs.                                          */
    /******************************************************************************************************************/
    void insert(KeyType key, long pointer) {
        OPEN_FILES(flags);//< open the files

        // locates the physical position of the database page where the new record should be inserted
        long seek = this->locate(key);

        long non_full = DISK_NULL;
        long prev = DISK_NULL;

        PrimaryKey ? locate_insertion_pk(seek, key, non_full, prev) : locate_insertion_non_pk(seek, non_full, prev);
        //^ At the end of this call, `non_full` contains the physical position of the first DataPage
        //  found whose number of records does not exceed the maximum allowed (`N`)
        //  and `prev` contains the physical position of the last DataPage before DISK_NULL was found (if is required).

        if (non_full != DISK_NULL) {
            DataPage<Pair<KeyType>> non_full_page;
            data_file.seekg(non_full);
            data_file.read((char *) &non_full_page, SIZE(DataPage<Pair<KeyType>>));
            non_full_page.records[non_full_page.n_records++] = Pair<KeyType>(key, pointer);
            data_file.seekp(non_full);
            data_file.write((char *) &non_full_page, SIZE(DataPage<Pair<KeyType>>));
        } else {
            data_file.close();
            data_file.open(data_file_name, std::ios::app);
            DataPage<Pair<KeyType>> new_page;
            new_page.records[new_page.n_records++] = Pair<KeyType>(key, pointer);
            long pos = data_file.tellp();
            data_file.write((char *) &new_page, SIZE(DataPage<Pair<KeyType>>));
            data_file.close();

            data_file.open(data_file_name, flags);
            DataPage<Pair<KeyType>> prev_page;
            data_file.seekg(prev);
            data_file.read((char *) &prev_page, SIZE(DataPage<Pair<KeyType>>));

            prev_page.next = pos;
            data_file.seekp(prev);
            data_file.write((char *) &prev_page, SIZE(DataPage<Pair<KeyType>>));
        }

        CLOSE_FILES;//< closes the files
    }
};

#endif //PROJECT_DATASET_ISAM_HPP
