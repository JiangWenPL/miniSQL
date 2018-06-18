#define INDEX_TEST
#ifdef INDEX_TEST

#include "IndexManager.h"


int main() {
    char *h = const_cast<char *>("Hello");
    m_string str(h);
    auto s2 = h;
    char h1[] = "H";
    m_string s1(h1);
    IndexManager indexManager;
    std::string name = "fuck";
    indexManager.create_index(name, IndexManager::max_var_char);
    IndexManager::dtype d;
    d.type_indicator = IndexManager::max_var_char;
    d.var_char = "f";
    indexManager.insert_index(name, d, 10);
    indexManager.delete_index(name, d);
    return 0;
}

#endif