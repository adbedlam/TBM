#include <APIread.h>

using json = nlohmann::json;

map<string, string> loadConf(const string& filename) {
    map<string, string> conf;

    ifstream file(filename);

    string line;

    while (std::getline(file, line)) {
        
        size_t pos = line.find('=');

        string key = line.substr(0, pos);

        string value = line.substr(pos+1, line.size());

        conf[key] = value;
    }
    // cout << "API: " << conf["API_KEY"] << endl;
    // cout << "Key: " << conf["SECRET_KEY"] << endl;
    return conf;
}

json loadJsonData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file: " + filename);
    }

    json data;
    file >> data;

    // Преобразуем строковые числа в числовые значения
    for (auto& candle : data) {
        if (candle["timestamp"].is_string()) {
            candle["timestamp"] = std::stoull(candle["timestamp"].get<string>());
        }
        if (candle["open"].is_string()) {
            candle["open"] = std::stod(candle["open"].get<string>());
        }
        if (candle["high"].is_string()) {
            candle["high"] = std::stod(candle["high"].get<string>());
        }
        if (candle["low"].is_string()) {
            candle["low"] = std::stod(candle["low"].get<string>());
        }
        if (candle["close"].is_string()) {
            candle["close"] = std::stod(candle["close"].get<string>());
        }
        if (candle["volume"].is_string()) {
            candle["volume"] = std::stod(candle["volume"].get<string>());
        }
    }

    return data;
}