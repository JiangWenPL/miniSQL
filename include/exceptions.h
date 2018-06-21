//
// Created by Wen Jiang on 6/19/18.
//

#ifndef MINISQL_EXCEPTIONS_H
#define MINISQL_EXCEPTIONS_H

#include <exception>
#include <string>
#include <iostream>

class DuplicateKey : public std::exception {
public:
    explicit DuplicateKey(const char *ptr = "Duplicate key exception") : ptr(ptr) {}

    char const *what() const noexcept override {
        std::cout << this->ptr << std::endl;
        return ptr;
    }

private:
    const char *ptr;
};

class TypeDisaccord : public std::exception {
public:
    explicit TypeDisaccord(const char *ptr = "type disaccord") : ptr(ptr) {}

    char const *what() const noexcept override {
        std::cout << this->ptr << std::endl;
        return ptr;
    }

private:
    const char *ptr;
};

class DuplicateIndex : public std::exception {
public:
    explicit DuplicateIndex(const char *ptr = "Already have index of same name") : ptr(ptr) {}

    char const *what() const noexcept override {
        std::cout << this->ptr << std::endl;
        return ptr;
    }

private:
    const char *ptr;
};


class IndexNotExist : public std::exception {
public:
    explicit IndexNotExist(const char *ptr = "Index to search is not exist") : ptr(ptr) {}

    char const *what() const noexcept override {
        std::cout << this->ptr << std::endl;
        return ptr;
    }


private:
    const char *ptr;
};

class KeyNotExist : public std::exception {
public:
    explicit KeyNotExist(const char *ptr = "Key to delete is not exist in B+ tree") : ptr(ptr) {}

    char const *what() const noexcept override {
        std::cout << this->ptr << std::endl;
        return ptr;
    }


private:
    const char *ptr;
};

class BatchSizeNotEqual : public std::exception {
public:
    explicit BatchSizeNotEqual(const char *ptr = "Vector size not equal in batch insertion") : ptr(ptr) {}

    char const *what() const noexcept override {
        std::cout << this->ptr << std::endl;
        return ptr;
    }


private:
    const char *ptr;
};

class BPTreeInnerException : public std::exception {
public:
    explicit BPTreeInnerException(const char *ptr = "Inner erro in B+ tree")
            : ptr(ptr) {}

    char const *what() const noexcept override {
        std::cout << this->ptr << std::endl;
        return ptr;
    }


private:
    const char *ptr;
};

#endif //MINISQL_EXCEPTIONS_H
