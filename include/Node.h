//
// Created by Wen Jiang on 6/21/18.
//

#ifndef MINISQL_NODE_H
#define MINISQL_NODE_H

#include "exceptions.h"
#include <string>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

typedef int offset;

template<typename T>
class Node {
private:
    int min_node_num;
public:
    // Indicator of node attributes.
    bool is_leaf;
    // degree of this node.
    int degree;
    // Number of keys stored in this node.
    int key_num;
    // Pointer to its father
    Node *father;
    // child pointer. Only used in internal node.
    std::vector<Node *> child;
    // Values's vector. Only used in leaf node.
    std::vector<int> values;
    // Pointer to next lead node. Only used in leaf node.
    Node *sibling;
    // Keys in this node
    std::vector<T> keys;

public:
    explicit Node(int in_degree, bool is_leaf_node = false);

    // Constructor
    ~Node();

    //
    bool is_root();

    // Find keys
    //Input:
    //  @key: key to find
    //  @value: reference of value,
    //Return:
    //  return true if exist, and will store the result in value
    //  otherwise false
    bool find_by_key(const T &key, int &value);

    // Split Node into two nodes.
    // New node will be sibling of this node.
    // And key will become this key of its father
    // @key:
    Node *split_node(T &key);

    // Insert into internal node.
    // @key: key to insert
    // @return: the offset to insert the key.
    int insert_key(const T &key);

    // Insert into leaf node
    // @key: key to insert.
    // @val: value to insert.
    int insert_key(const T &key, const int &val);

    // Delete the key with value of @value
    // @value value to delete associate with key.
    // @retrun true if success
    bool delete_key_start_by(int start_index);

    // @return: return the pointer Node* sbling
    Node *get_sibling_node();

    // Find from @start_index until reach @terminate_key
    // @start_index: start of i-th index in this node.
    // @terminate_key: stop find if reach terminate_key
    // @values: container to store the search answer.
    // @return true if successfully reach terminate_key.
    bool find_in_range(int start_index, const T &terminate_key, std::vector<int> &values);

    // Find from @start_index until reach @terminate_key
    // @start_index: start of i-th index in this node.
    // @values: container to store the search answer.
    // @return true if successfully find elements
    bool find_greater_than(int start_index, std::vector<int> &values);

    void print_node();
};

template<class T>
Node<T>::Node(int in_degree, bool is_leaf_node):
        key_num(0),
        father(NULL),
        sibling(NULL),
        is_leaf(is_leaf_node),
        degree(in_degree) {
    min_node_num = (degree - 1) / 2;
    for (unsigned i = 0; i < degree + 1; i++) {
        child.push_back(NULL);
        keys.push_back(T());
        values.push_back(int());
    }

    child.push_back(NULL);
}

// Default de-constructor
template<class T>
Node<T>::~Node() = default;

// To check if is a leaf node.
template<class T>
bool Node<T>::is_root() {
    return father == nullptr;
}

// Find keys
//Input:
//  @key: key to find
//  @value: reference of value,
//Return:
//  return true if exist, and will store the result in value
//  otherwise false
template<class T>
bool Node<T>::find_by_key(const T &key, int &value) {
    if (key_num == 0) {
        // This is an empty node.
        value = 0;
        return false;
    } else {
        // key is nor in this node since it is greater than the last key
        // in this node.
        if (keys[key_num - 1] < key) {
            value = key_num;
            return false;
        } else if (keys[0] > key) {
            // key is nor in this node since it is smaller than the last key
            // in this node.
            value = 0;
            return false;
        } else {
            // scan all the value to find the key.
            for (int i = 0; i < key_num; i++) {
                if (keys[i] == key) {
                    value = i;
                    return true;
                } else if (keys[i] < key)
                    continue;
                else if (keys[i] > key) {
                    value = i;
                    return false;
                }
            }
        }
    }

    return false;
}

template<class T>
Node<T> *Node<T>::split_node(T &key) {
    auto *new_node = new Node(degree, this->is_leaf);

    // When is leaf node, operate on value.
    if (is_leaf) {
        key = keys[min_node_num + 1];
        // Copy keys:values to new node.
        for (int i = min_node_num + 1; i < degree; i++) {
            new_node->keys[i - min_node_num - 1] = keys[i];
            keys[i] = T();
            new_node->values[i - min_node_num - 1] = values[i];
            values[i] = int();
        }
        // Update sibling and father pointers
        new_node->sibling = this->sibling;
        this->sibling = new_node;
        new_node->father = this->father;

        // Adjust key number.
        new_node->key_num = min_node_num;
        this->key_num = min_node_num + 1;
    } else if (!is_leaf) {
        // for internal node, do not operate on value
        key = keys[min_node_num];
        // copy keys to new node
        for (int i = min_node_num + 1; i < degree + 1; i++) {
            new_node->child[i - min_node_num - 1] = this->child[i];
            new_node->child[i - min_node_num - 1]->father = new_node;
            this->child[i] = NULL;
        }
        // Copy value to new node.
        for (int i = min_node_num + 1; i < degree; i++) {
            new_node->keys[i - min_node_num - 1] = this->keys[i];
            this->keys[i] = T();
        }
        // Update father pointer
        this->keys[min_node_num] = T();
        new_node->father = this->father;

        // Adjust key_num of each node.
        new_node->key_num = min_node_num;
        this->key_num = min_node_num;
    }

    return new_node;
}

template<class T>
int Node<T>::insert_key(const T &key) {
    // Empty node:
    if (key_num == 0) {
        keys[0] = key;
        key_num++;
        return 0;
    } else {
        // Check is key exists
        int index = 0;
        bool exist = find_by_key(key, index);
        if (exist) {
            throw DuplicateKey();
        } else {
            // Insert key into keys
            for (int i = key_num; i > index; i--)
                keys[i] = keys[i - 1];
            keys[index] = key;

            //Adjust pointers
            for (int i = key_num + 1; i > index + 1; i--)
                child[i] = child[i - 1];
            child[index + 1] = NULL;
            // Key number increment
            key_num++;

            return index;
        }
    }

    return 0;
}


template<class T>
int Node<T>::insert_key(const T &key, const int &val) {
    if (!is_leaf) {
        throw BPTreeInnerException("This method is not allowed to be visted by internal nodes.");
        return -1;
    }

    // Empty node:
    if (key_num == 0) {
        keys[0] = key;
        values[0] = val;
        key_num++;
        return 0;
    } else {
        int index = 0;
        bool exist = find_by_key(key, index);
        if (exist) {
            throw DuplicateKey();
        } else {
            // Adjust key & values.
            for (int i = key_num; i > index; i--) {
                keys[i] = keys[i - 1];
                values[i] = values[i - 1];
            }
            keys[index] = key;
            values[index] = val;
            key_num++;
            return index;
        }
    }

    return 0;
}

template<class T>
bool Node<T>::delete_key_start_by(int start_index) {
    if (start_index > key_num) {
        throw BPTreeInnerException("Start index to delete is bigger than number of keys in this node");
    } else {
        if (is_leaf) {
            // For leaf node also move values
            for (int i = start_index; i < key_num - 1; i++) {
                keys[i] = keys[i + 1];
                values[i] = values[i + 1];
            }
            keys[key_num - 1] = T();
            values[key_num - 1] = int();
        } else {
            // As for internal node.
            for (int i = start_index; i < key_num - 1; i++)
                keys[i] = keys[i + 1];
            // Update child pointers
            for (int i = start_index + 1; i < key_num; i++)
                child[i] = child[i + 1];

            keys[key_num - 1] = T();
            child[key_num] = NULL;
        }

        key_num--;
        return true;
    }

    return false;
}

template<class T>
Node<T> *Node<T>::get_sibling_node() {
    return sibling;
}


template<class T>
bool Node<T>::find_in_range(int start_index, const T &terminate_key, std::vector<int> &results) {
    int i;
    for (i = start_index; i < key_num && keys[i] <= terminate_key; i++)
        results.push_back(values[i]);

    return !(keys[i] < terminate_key);
}

template<class T>
bool Node<T>::find_greater_than(int start_index, std::vector<int> &results) {
    int i;
    for (i = start_index; i < key_num; i++)
        results.push_back(values[i]);

    return false;
}

template<class T>
void Node<T>::print_node() {
    for (int i = 0; i < key_num; i++)
        std::cout << "->" << keys[i];
    std::cout << std::endl;

}

#endif //MINISQL_NODE_H
