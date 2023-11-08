#include <any>
#include <map>
#include <string>
#include "baseConfig.hpp"
#include <boost/property_tree/ptree.hpp>

class JsonPareser{
private:
    boost::property_tree::ptree data;

public:
    JsonPareser();

    void parse(std::string&);

    std::string output();

    std::map<std::string, int> outputConfig();
};