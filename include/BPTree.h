//
// Created by Wen Jiang on 6/15/18.
//

#ifndef MINISQL_BPTREE_H
#define MINISQL_BPTREE_H

#include <string>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

// Template B+ tree node.
// For coding convenience, we define an unified node class both represent
// internal node and leaf node.
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
    bool delete_key_with_value(int value);

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

template<typename T>
class BPTree {
private:
    typedef Node<T> *Tree;
    struct search_info {
        Tree pNode;
        int value;
        bool is_found;
    };
    static const int PAGESIZE = 4096;
    // name of index
    std::string m_name;
    // Pointer to root
    Tree root;
    // Pointer to the head of leaf node
    Tree p_leaf_head;
    // Number of keys
    unsigned int key_num;
    // Number of levels
    unsigned int level;
    // Number of nodes.
    unsigned int node_num;
    int key_size;
    // Degree of this B+ tree
    int degree;
    // min number of keys.
    int min_key_num;
public:

    explicit BPTree(std::string &name);

    ~BPTree();

    // Search by key
    // @key: key to search
    // @return the value stored in b+ tree.
    //  -1 if not find
    offset search_by_key(const T &key);

    // Insert key:value into B+ tree
    // @return true if success inserted.
    bool insert(const T &key, int value);

    // Delete key:value in B+ tree
    // @return true if delte success
    bool delete_by_key(const T &key);

    // Destroy this tree.
    // @tree: root of this tree
    // Can also be a node.
    void destroy_tree(Tree tree);

    // Search all value in range (key1, key2)
    // @start_key start key to search
    // @end_key terminate key to searh
    // @return: vector to store all value found
    std::vector<offset> search_between(const T &begin_key, const T &end_key);

    std::vector<offset> search_smaller(const T &end_key);

    std::vector<offset> search_greater(const T &begin_key);

    // Load from disk
    void load_all_node();

    // Dump to disk
    void dump_to_disk();

    // load a page
    void load_from_disk(char *p, char *end);

    void print_leaf();

private:
    // Init B+ tree
    void initialize();

    // Adjust to avoid overflow
    bool adjust_after_insert(Tree pNode);

    // Adjust to avoid underflow
    bool adjust_after_delete(Tree pNode);

    // Search where the leaf stored
    void find_by_key(Tree pNode, const T &key, search_info &info);

    // Create or open file.
    void get_file(const std::string &file_name);

    int count_block_num(const std::string &index_name);
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
            //TODO: raise exception
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
        // TODO: Raise exception of visited by internal nodes.
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
            // TODO: raise exception of duplicate key
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
bool Node<T>::delete_key_with_value(int value) {
    if (value > key_num) {
        // TODO: raise exception of values
    } else {
        if (is_leaf) {
            // For leaf node also move values
            for (int i = value; i < key_num - 1; i++) {
                keys[i] = keys[i + 1];
                values[i] = values[i + 1];
            }
            keys[key_num - 1] = T();
            values[key_num - 1] = int();
        } else {
            // As for internal node.
            for (int i = value; i < key_num - 1; i++)
                keys[i] = keys[i + 1];
            // Update child pointers
            for (int i = value + 1; i < key_num; i++)
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
bool Node<T>::find_in_range(int start_index, const T &terminate_key, std::vector<int> &values) {
    int i;
    for (i = start_index; i < key_num && keys[i] <= terminate_key; i++)
        values.push_back(values[i]);

    return !(keys[i] < terminate_key);
}

template<class T>
bool Node<T>::find_greater_than(int start_index, std::vector<int> &values) {
    int i;
    for (i = start_index; i < key_num; i++)
        values.push_back(values[i]);

    return false;
}

template<class T>
void Node<T>::print_node() {
    for (int i = 0; i < key_num; i++)
        std::cout << "->" << keys[i];
    std::cout << std::endl;

}

template<class T>
BPTree<T>::BPTree(std::string &name):
        m_name(name),
        key_num(0),
        level(0),
        node_num(0),
        root(NULL),
        p_leaf_head(NULL) {

    key_size = sizeof(T);
    degree = (PAGESIZE - sizeof(int)) / (sizeof(T) + sizeof(int));
    min_key_num = (degree - 1) / 2;
    // Initialize the keys.
    initialize();

    load_all_node();

}


template<class T>
BPTree<T>::~BPTree() {
    destroy_tree(root);
    key_num = 0;
    root = NULL;
    level = 0;
}


template<class T>
void BPTree<T>::initialize() {
    root = new Node<T>(degree, true);
    key_num = 0;
    level = 1;
    node_num = 1;
    p_leaf_head = root;
}


template<class T>
void BPTree<T>::find_by_key(Tree pNode, const T &key, search_info &info) {
    int key_index = 0; // The index storing the key in this node.
    // Search if key exist in this node
    if (pNode->find_by_key(key, key_index)) {
        // This is leaf node and key in this node.
        if (pNode->is_leaf) {
            info.pNode = pNode;
            info.value = key_index;
            info.is_found = true;
        } else {
            // Not leaf node, search recursively.
            pNode = pNode->child[key_index + 1];
            while (!pNode->is_leaf) {
                pNode = pNode->child[0];
            }
            // Since key appears in non leaf node.
            // Key must be the first value in this node.
            info.pNode = pNode;
            info.value = 0;
            info.is_found = true;
        }

    } else {
        // Key not appears in this node.
        if (pNode->is_leaf) {
            // Still not find
            info.pNode = pNode;
            info.value = key_index;
            info.is_found = false;
        } else {
            // Search recursively
            find_by_key(pNode->child[key_index], key, info);
        }
    }
}

template<class T>
bool BPTree<T>::insert(const T &key, const int value) {
    search_info info;
    // Init in the first insert;
    if (!root)
        initialize();
    // Check if exist
    find_by_key(root, key, info);
    if (info.is_found) {
        // TODO: raise duplicate key exception
        return false;
    } else {
        info.pNode->insert_key(key, value);
        // Adjust after insertion
        if (info.pNode->key_num == degree) {
            adjust_after_insert(info.pNode);
        }
        key_num++;
        return true;
    }
}

template<class T>
bool BPTree<T>::adjust_after_insert(Tree pNode) {
    T key;
    Tree newNode = pNode->split_node(key);
    node_num++;

    if (pNode->is_root()) {
        // If just have root node.
        auto root = new Node<T>(degree, false);
        level++;
        node_num++;
        this->root = root;
        pNode->father = root;
        newNode->father = root;
        root->insert_key(key);
        root->child[0] = pNode;
        root->child[1] = newNode;
        return true;
    } else {
        // Not root
        Tree father = pNode->father;
        int index = father->insert_key(key);

        father->child[index + 1] = newNode;
        newNode->father = father;
        // Adjust recursively
        if (father->key_num == degree)
            return adjust_after_insert(father);

        return true;
    }
}

template<class T>
offset BPTree<T>::search_by_key(const T &key) {
    if (!root)
        return -1;
    search_info info;
    find_by_key(root, key, info);

    if (!info.is_found)
        return -1;
    else
        return info.pNode->values[info.value];
}

template<class T>
bool BPTree<T>::delete_by_key(const T &key) {
    search_info info;
    // Root node not exist.
    if (!root) {
        // TODO: raise exception of null root
        return false;
    } else {
        find_by_key(root, key, info);
        if (!info.is_found) {
            // TODO: raise exception of deleting the key not exists.
            return false;
        } else {
            if (info.pNode->is_root()) {
                // This tree just contain an root node.
                info.pNode->delete_key_with_value(info.value);
                key_num--;
                return adjust_after_delete(info.pNode);
            } else {
                if (info.value == 0 && p_leaf_head != info.pNode) {
                    // Update internal node
                    int index = 0;

                    Tree cur_father = info.pNode->father;
                    bool if_found_inBranch = cur_father->find_by_key(key, index);
                    while (!if_found_inBranch) {
                        if (cur_father->father)
                            cur_father = cur_father->father;
                        else
                            break;
                        if_found_inBranch = cur_father->find_by_key(key, index);
                    }

                    cur_father->keys[index] = info.pNode->keys[1];

                    info.pNode->delete_key_with_value(info.value);
                    key_num--;
                    return adjust_after_delete(info.pNode);

                } else {
                    // Update lead node.
                    info.pNode->delete_key_with_value(info.value);
                    key_num--;
                    return adjust_after_delete(info.pNode);
                }
            }
        }
    }
}

template<class T>
bool BPTree<T>::adjust_after_delete(Tree pNode) {
    // Not necessary to adjust:
    if (((pNode->is_leaf) && (pNode->key_num >= min_key_num)) ||
        ((degree != 3) && (!pNode->is_leaf) && (pNode->key_num >= min_key_num - 1)) ||
        ((degree == 3) && (!pNode->is_leaf) && (pNode->key_num < 0))) {
        return true;
    }
    if (pNode->is_root()) {
        // For root node
        if (pNode->key_num > 0)
            return true;
        else {
            // None leaf node
            if (root->is_leaf) {
                // If root is also a leaf node, we shall set root = NULL
                delete pNode;
                root = NULL;
                p_leaf_head = NULL;
                level--;
                node_num--;
            } else {
                // son of root node become leaf
                root = pNode->child[0];
                root->father = NULL;
                delete pNode;
                level--;
                node_num--;
            }
        }
    } else {
        // Non root
        Tree father = pNode->father, brother = NULL;
        if (pNode->is_leaf) {
            // delete in leaf
            int index = 0;
            father->find_by_key(pNode->keys[0], index);
            // Borrow from left siblings
            if ((father->child[0] != pNode) && (index + 1 == father->key_num)) {
                brother = father->child[index];
                if (brother->key_num > min_key_num) {
                    for (int i = pNode->key_num; i > 0; i--) {
                        pNode->keys[i] = pNode->keys[i - 1];
                        pNode->values[i] = pNode->values[i - 1];
                    }
                    pNode->keys[0] = brother->keys[brother->key_num - 1];
                    pNode->values[0] = brother->values[brother->key_num - 1];
                    brother->delete_key_with_value(brother->key_num - 1);

                    pNode->key_num++;
                    father->keys[index] = pNode->keys[0];
                    return true;

                } else {
                    father->delete_key_with_value(index);

                    for (int i = 0; i < pNode->key_num; i++) {
                        brother->keys[i + brother->key_num] = pNode->keys[i];
                        brother->values[i + brother->key_num] = pNode->values[i];
                    }
                    brother->key_num += pNode->key_num;
                    brother->sibling = pNode->sibling;

                    delete pNode;
                    node_num--;

                    return adjust_after_delete(father);
                }

            } else {
                if (father->child[0] == pNode)
                    brother = father->child[1];
                else
                    brother = father->child[index + 2];
                if (brother->key_num > min_key_num) {
                    pNode->keys[pNode->key_num] = brother->keys[0];
                    pNode->values[pNode->key_num] = brother->values[0];
                    pNode->key_num++;
                    brother->delete_key_with_value(0);
                    if (father->child[0] == pNode)
                        father->keys[0] = brother->keys[0];
                    else
                        father->keys[index + 1] = brother->keys[0];
                    return true;

                } else {
                    for (int i = 0; i < brother->key_num; i++) {
                        pNode->keys[pNode->key_num + i] = brother->keys[i];
                        pNode->values[pNode->key_num + i] = brother->values[i];
                    }
                    if (pNode == father->child[0])
                        father->delete_key_with_value(0);
                    else
                        father->delete_key_with_value(index + 1);
                    pNode->key_num += brother->key_num;
                    pNode->sibling = brother->sibling;
                    delete brother;
                    node_num--;

                    return adjust_after_delete(father);
                }
            }

        } else {
            int index = 0;
            father->find_by_key(pNode->child[0]->keys[0], index);
            if ((father->child[0] != pNode) && (index + 1 == father->key_num)) {
                brother = father->child[index];
                if (brother->key_num > min_key_num - 1) {
                    pNode->child[pNode->key_num + 1] = pNode->child[pNode->key_num];
                    for (int i = pNode->key_num; i > 0; i--) {
                        pNode->child[i] = pNode->child[i - 1];
                        pNode->keys[i] = pNode->keys[i - 1];
                    }
                    pNode->child[0] = brother->child[brother->key_num];
                    pNode->keys[0] = father->keys[index];
                    pNode->key_num++;

                    father->keys[index] = brother->keys[brother->key_num - 1];

                    if (brother->child[brother->key_num])
                        brother->child[brother->key_num]->father = pNode;
                    brother->delete_key_with_value(brother->key_num - 1);

                    return true;

                } else {
                    brother->keys[brother->key_num] = father->keys[index];
                    father->delete_key_with_value(index);
                    brother->key_num++;

                    for (int i = 0; i < pNode->key_num; i++) {
                        brother->child[brother->key_num + i] = pNode->child[i];
                        brother->keys[brother->key_num + i] = pNode->keys[i];
                        brother->child[brother->key_num + i]->father = brother;
                    }
                    brother->child[brother->key_num + pNode->key_num] = pNode->child[pNode->key_num];
                    brother->child[brother->key_num + pNode->key_num]->father = brother;

                    brother->key_num += pNode->key_num;

                    delete pNode;
                    node_num--;

                    return adjust_after_delete(father);
                }

            } else {
                if (father->child[0] == pNode)
                    brother = father->child[1];
                else
                    brother = father->child[index + 2];
                if (brother->key_num > min_key_num - 1) {

                    pNode->child[pNode->key_num + 1] = brother->child[0];
                    pNode->keys[pNode->key_num] = brother->keys[0];
                    pNode->child[pNode->key_num + 1]->father = pNode;
                    pNode->key_num++;

                    if (pNode == father->child[0])
                        father->keys[0] = brother->keys[0];
                    else
                        father->keys[index + 1] = brother->keys[0];

                    brother->child[0] = brother->child[1];
                    brother->delete_key_with_value(0);

                    return true;
                } else {

                    pNode->keys[pNode->key_num] = father->keys[index];

                    if (pNode == father->child[0])
                        father->delete_key_with_value(0);
                    else
                        father->delete_key_with_value(index + 1);

                    pNode->key_num++;

                    for (int i = 0; i < brother->key_num; i++) {
                        pNode->child[pNode->key_num + i] = brother->child[i];
                        pNode->keys[pNode->key_num + i] = brother->keys[i];
                        pNode->child[pNode->key_num + i]->father = pNode;
                    }
                    pNode->child[pNode->key_num + brother->key_num] = brother->child[brother->key_num];
                    pNode->child[pNode->key_num + brother->key_num]->father = pNode;

                    pNode->key_num += brother->key_num;

                    delete brother;
                    node_num--;

                    return adjust_after_delete(father);
                }

            }

        }

    }

    return false;
}

template<class T>
void BPTree<T>::destroy_tree(Tree tree) {
    // Check if is a empty tree
    if (!tree)
        return;
    // Destroy B+ tree recursively
    if (!tree->is_leaf) {
        for (unsigned int i = 0; i <= tree->key_num; i++) {
            destroy_tree(tree->child[i]);
            tree->child[i] = NULL;
        }
    }
    delete tree;
    node_num--;
}

template<class T>
std::vector<offset> BPTree<T>::search_between(const T &begin_key, const T &end_key) {
    std::vector<offset> results;
    if (!root)
        return results;
    search_info info1, info2;
    find_by_key(root, begin_key, info1);
    find_by_key(root, end_key, info2);
    bool finished;
    int index;

    if (begin_key <= end_key) {
        Tree pNode = info1.pNode;
        index = info1.value;
        do {
            finished = pNode->find_in_range(index, end_key, results);
            index = 0;
            if (pNode->sibling == NULL)
                break;
            else
                pNode = pNode->get_sibling_node();
        } while (!finished);
    } else {
        Tree pNode = info2.pNode;
        index = info2.value;
        do {
            finished = pNode->find_in_range(index, begin_key, results);
            index = 0;
            if (pNode->sibling == NULL)
                break;
            else
                pNode = pNode->get_sibling_node();
        } while (!finished);
    }
    std::sort(results.begin(), results.end());
    results.erase(unique(results.begin(), results.end()), results.end());
    return results;
}

template<class T>
std::vector<offset> BPTree<T>::search_smaller(const T &end_key) {
    std::vector<offset> results;
    if (!root)
        return results;
    search_info info2;
    find_by_key(root, end_key, info2);

    bool finished;
    Tree pNode = info2.pNode;
    int index = info2.value;

    do {
        finished = pNode->find_greater_than(index, results);
        index = 0;
        if (pNode->sibling == NULL)
            break;
        else
            pNode = pNode->get_sibling_node();
    } while (!finished);
    std::sort(results.begin(), results.end());
    results.erase(unique(results.begin(), results.end()), results.end());
    return results;
}

template<class T>
std::vector<offset> BPTree<T>::search_greater(const T &begin_key) {
    std::vector<offset> results;
    if (!root)
        return results;
    search_info info1;
    find_by_key(root, begin_key, info1);

    bool finished;
    Tree pNode = info1.pNode;
    int index = info1.value;

    do {
        finished = pNode->find_greater_than(index, results);
        index = 0;
        if (pNode->sibling == NULL)
            break;
        else
            pNode = pNode->get_sibling_node();
    } while (!finished);
    std::sort(results.begin(), results.end());
    results.erase(unique(results.begin(), results.end()), results.end());
    return results;
}

template<class T>
void BPTree<T>::print_leaf() {
    Tree p = p_leaf_head;
    while (p != NULL) {
        p->print_node();
        p = p->get_sibling_node();
    }
}

template<class T>
void BPTree<T>::get_file(const std::string &file_name) {
    FILE *f = fopen(file_name.c_str(), "r");
    if (f == nullptr) {
        f = fopen(file_name.c_str(), "w+");
        fclose(f);
        f = fopen(file_name.c_str(), "r");
    }
    fclose(f);
}

template<class T>
void BPTree<T>::load_from_disk(char *p, char *end) {
    //TODO: load B+ tree from disk
}

template<class T>
void BPTree<T>::dump_to_disk() {
    // TODO: dump B+ tree to disk
}

template<class T>
int BPTree<T>::count_block_num(const std::string &index_name) {
    // TODO: get block number:
    return 0;
}

template<class T>
void BPTree<T>::load_all_node() {
    //TODO: load from disk
}


#endif //MINISQL_BPTREE_H
