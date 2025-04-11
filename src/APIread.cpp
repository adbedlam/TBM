#include <APIread.h>


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