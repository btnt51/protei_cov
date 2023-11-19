#include "jsonParser.hpp"
#include "boost/property_tree/ptree.hpp"
#include <sstream>
#include <iostream>
#include <string>
#include <boost/property_tree/json_parser.hpp>

using namespace utility;

JsonParser::JsonParser() {}

void JsonParser::parse(std::string& pathToFile) {
    std::ifstream file(pathToFile);
    std::stringstream ss;
    ss << file.rdbuf();
    boost::property_tree::read_json(ss, data_);
}

std::string JsonParser::output() {
    std::string res("");
    res += "RMin: " + std::to_string(data_.get<int>("RMin")) + "\n";
    res += "RMax: " + std::to_string(data_.get<int>("RMax"))+ "\n";
    res += "AmountOfOperators: " + std::to_string(data_.get<int>("AmountOfOperators"))+ "\n";
    res += "SizeOfQueue: " + std::to_string(data_.get<int>("SizeOfQueue"))+ "\n";
    return res;
}

std::map<std::string, int> JsonParser::outputConfig() {
    std::map<std::string, int> res;
    boost::property_tree::basic_ptree<std::string, std::string>::const_iterator iter=data_.begin(), iterEnd = data_.end();
    for(;iter != iterEnd;++iter) {
        res[iter->first] = iter->second.get_value<int>();
    }
    return res;
}