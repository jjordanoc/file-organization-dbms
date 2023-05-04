//
// Created by juandiego on 4/14/23.
//

#ifndef AVL_FILE_NODE_HPP
#define AVL_FILE_NODE_HPP

#include <sstream>

#include "utils.hpp"

/// nullptr file representation
#define DISK_NULL (-1)
#define INITIAL_RECORD (0)
// for remove method purposes
#define DETACH (-2)
#define NOT_DETACH (-3)

template<typename KeyType>
struct Node {
    KeyType key{};
    long data_pointer = DISK_NULL;
    long left = DISK_NULL;
    long right = DISK_NULL;
    long height = 0;
    long next = DISK_NULL;

    explicit Node() = default;

    explicit Node(KeyType key_, long physical_position) : data_pointer(physical_position) {
        func::copy(key, key_);
    }

    Node<KeyType> &operator=(const Node<KeyType> &other) {
        func::copy(key, other.key);
        next = other.next;
        data_pointer = other.data_pointer;
        return *this;
    }

    std::string to_string() {
        std::stringstream ss;
        ss << "<key: " << key << ", pointer: " << data_pointer << ", height: " << height << ", left: " << left
           << ", right: " << right << ", next: " << next << ">";
        return ss.str();
    }
};

template<typename RecordType>
std::ostream &operator<<(std::ostream &os, Node<RecordType> &node) {
    os.write((char *) &node, sizeof(Node<RecordType>));
    return os;
}

template<typename RecordType>
std::istream &operator>>(std::istream &is, Node<RecordType> &node) {
    is.read((char *) &node, sizeof(Node<RecordType>));
    return is;
}

#endif //AVL_FILE_NODE_HPP