/**
 * \file jsonParser.hpp
 * \brief Содержит объявление класса JsonParser, предназначенного для парсинга и обработки JSON данных.
 */
#include <any>
#include <filesystem>
#include <map>
#include <string>
#include <boost/property_tree/ptree.hpp>

#include "baseConfig.hpp"



/**
 * @namespace utility
 * @brief Пространство имен utility, содержащее функционал для обработки JSON данных.
 */
namespace utility {

/**
 * @class JsonParser
 * @brief Класс JsonParser, предназначенный для парсинга JSON данных.
 */
class JsonParser {
public:
    /// @brief Конструктор класса JsonParser.
    JsonParser();

    /**
     * @brief Метод для парсинга файла.
     * @param pathToFile Путь до файла, который необходимо распарсить.
     */
    void parse(const std::filesystem::path& pathToFile);

    /**
     *  @brief Метод для вывода JSON данных в виде строки.
     *  @return Строка, содержащая представление JSON данных.
     */
    std::string output();

    /**
     * @brief Метод для вывода конфигурационных данных в виде карты (map) строк и целых чисел.
     *  @return Карта, содержащая конфигурационные данные.
     */
    std::map<std::string, int> outputConfig();

private:
    boost::property_tree::ptree data_; ///< Объект property_tree для хранения и обработки данных.
};

} // namespace utility
