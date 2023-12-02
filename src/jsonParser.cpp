#include "jsonParser.hpp"
#include "boost/property_tree/ptree.hpp"
#include <sstream>
#include <iostream>
#include <string>
#include <boost/property_tree/json_parser.hpp>

using namespace utility;

JsonParser::JsonParser(std::shared_ptr<spdlog::logger> logger) : logger_(logger) {}

void JsonParser::parse(const std::filesystem::path& pathToFile) {
    try {
        if(logger_)
            logger_->debug("Parsing JSON file: {}", pathToFile.string());
        std::ifstream file(pathToFile);
        std::stringstream ss;
        ss << file.rdbuf();
        std::string str = ss.str();
        boost::property_tree::read_json(ss, data_);
    } catch (std::exception& e) {
        if(logger_)
            logger_->critical("Critical error parsing JSON file {}: {}", pathToFile.string(), e.what());
        throw e;
    }
}

std::string JsonParser::output() {
    if(logger_)
        logger_->debug("Generating output string from JSON data");
    std::string res("");
    res += "RMin: " + std::to_string(data_.get<int>("RMin")) + "\n";
    res += "RMax: " + std::to_string(data_.get<int>("RMax"))+ "\n";
    res += "AmountOfOperators: " + std::to_string(data_.get<int>("AmountOfOperators"))+ "\n";
    res += "SizeOfQueue: " + std::to_string(data_.get<int>("SizeOfQueue"))+ "\n";
    return res;
}

std::map<std::string, int> JsonParser::outputConfig() {
    if(logger_)
        logger_->debug("Generating output string from JSON data");
    std::map<std::string, int> res;
    try {
        boost::property_tree::basic_ptree<std::string, std::string>::const_iterator iter = data_.begin(),
                                                                                    iterEnd = data_.end();
        for (; iter != iterEnd; ++iter) {
            res[iter->first] = iter->second.get_value<int>();
        }
    } catch (std::exception& e) {
        if(logger_)
            logger_->error("Error generating output configuration map: {}", e.what());
        throw e;
    }
    return res;
}

void JsonParser::setLogger(std::shared_ptr<spdlog::logger> logger) {
    logger_ = logger;
}