#include <vector>
#include <string>
#include <iostream>
#include <SicHash.h>

int main() {
    std::vector<std::string> keys = {"abc", "def", "123", "456"};
    sichash::SicHashConfig config;
    config.silent = false;
    sichash::SicHash<true> hashFunc(keys, config);
    std::cout << hashFunc("abc") << std::endl;
}
