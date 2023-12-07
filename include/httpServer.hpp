#ifndef PROTEI_COV_HTTPSERVER_HPP
#define PROTEI_COV_HTTPSERVER_HPP
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <spdlog/async.h>
#include "builder.hpp"

/**
 * @file httpServer.hpp
 * @brief Содержит объявление класса HttpServer
 */

namespace net {
namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;
/**
 * @brief Класс HttpServer для обработки HTTP-запросов.
 */
class HttpServer {
public:
    /**
     * @brief Конструктор HttpServer.
     * @param port Порт для прослушивания.
     * @param path Путь для обработки запросов.
     */
    HttpServer(short unsigned port, std::filesystem::path path);
    /**
     * @brief Запускает сервер.
     */
    void run();

private:
    /**
     * @brief Обрабатывает HTTP-запросы.
     * @param socket Сокет для обработки запроса.
     */
    void handleHttpRequest(tcp::socket socket);
    /**
     * @brief Обрабатывает HTTP-запрос по указанному пути.
     * @param path Путь запроса.
     * @param res Ответ на запрос.
     */
    void handleRequest(const std::string& path, beast::http::response<beast::http::string_body>& res);

    /**
     * @brief Обрабатывает телефонный вызов.
     * @param res Ответ на запрос.
     * @param phone Номер телефона.
     */
    void processPhoneCall(beast::http::response<beast::http::string_body>& res, std::string_view phone);

    /**
     * @brief Обрабатывает статус вызова, доп функция для HttpServer@processPhoneCall
     * @param res Ответ на запрос.
     * @param status Статус вызова.
     */
    void workWithCallStatus(beast::http::response<beast::http::string_body>& res, CallStatus status);

    /**
     * @brief Обрабатывает запрос на обновление.
     * @param res Ответ на запрос.
     */
    void processUpdate(beast::http::response<beast::http::string_body>& res);

private:
    asio::io_context io_context; ///< Контекст ввода-вывода для асинхронных операций.
    tcp::acceptor acceptor; ///< Акцептор для прослушивания входящих соединений.
    std::shared_ptr<Manager> manager; ///< Указатель на объект Manager для обработки вызовов.
    std::shared_ptr<spdlog::logger> logger_; ///< Указатель на объект логгера.
};
}

#endif // PROTEI_COV_HTTPSERVER_HPP
