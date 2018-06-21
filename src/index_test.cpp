#define INDEX_TEST
#ifdef INDEX_TEST

#include "IndexManager.h"
#include <random>

int main() {
    IndexManager indexManager;
    std::string name = "fuck";
    indexManager.create_index(name, IndexManager::type_int);
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(-1000, 1000);
    for (int j = 0; j < 10000; j++) {
        try {
            auto i = dis(gen);
            indexManager.insert_index(name, i, i);
        } catch (std::exception &e) { ;
        }
    }
    for (int j = 0; j < 10000; j++) {
        try {
            auto i = dis(gen);
            indexManager.delete_index(name, i);
        } catch (std::exception &e) { ;
        }
    }
    for (int j = 0; j < 10000; j++) {
        try {
            auto i = dis(gen);
            auto k = dis(gen);
            indexManager.search_equal(name, i);
            indexManager.search_between(name, i, k);
            indexManager.search_smaller(name, i);
            indexManager.search_greater(name, i);

        } catch (std::exception &e) { ;
        }
    }
    return 0;
}

#endif