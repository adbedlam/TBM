#ifndef APIREAD_H
#define APIREAD_H
#include "common.hpp"

using std::map;
using std::string;
using std::cout;
using std::ifstream;
using std::getline;
using std::endl;


map<string, string> loadConf(const string& filename);
nlohmann::json loadJsonData(const string& filename);  // Добавлено
#endif //APIREAD_H
