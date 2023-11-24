#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <uuid/uuid.h>
#include "config.hpp"
#include "manager.hpp"
#include "threadpool.hpp"
namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;


auto config = std::make_shared<utility::ThreadSafeConfig>("base.json");
auto pool = std::make_shared<TP::ThreadPool>(config->getAmountOfOperators());
auto manager = std::make_shared<Manager>(config, pool);


void HandleHttpRequest(const std::string& path, beast::http::response<beast::http::string_body>& res) {
    if (path.find("/phone=") == 0) {
        // Извлечь значение "name" из параметра запроса
        try {
            std::string phone = path.substr(7); // 11 - длина "/user?name="
            std::cout << "Thread id: " << std::this_thread::get_id() << " phone: " << phone << std::endl;
            auto [callID, future] = manager->addTask(phone);
            // Далее вы можете использовать значение "name" в ответе

            res.result(beast::http::status::ok);
            auto result = future.get();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(result.callDuration);
            res.body() =
                "CallID: " + std::to_string(callID) + " call duration: " + std::to_string(seconds.count()) + "s";
            std::string status;
            if (result.status == CallStatus::completed)
                status = "completed";
            if (result.status == CallStatus::rejected)
                status = "rejected";
            if (result.status == CallStatus::awaiting)
                status = "awaiting";
            res.body() += "\n Status: " + status + "\n";
        } catch (const std::future_error &e) {
            // Ловим исключение, если произошла ошибка с future
            std::cerr << "Caught a future_error: " << e.what() << std::endl;
        }
    } else {
        res.result(beast::http::status::not_found);
        res.body() = "Not Found";
    }
    res.version(11);
    res.set(beast::http::field::server, "Boost.Beast HTTP Server");
    res.prepare_payload();
}


void DoHttpServer(tcp::socket socket) {
    try {
        beast::tcp_stream stream(std::move(socket));

        beast::flat_buffer buffer;
        beast::http::request<beast::http::string_body> req;
        beast::http::read(stream, buffer, req);
        beast::http::response<beast::http::string_body> res;
        if (req.method() == beast::http::verb::get) {
            HandleHttpRequest(req.target(), res);
            beast::http::write(stream, res);
        }
    } catch (const beast::system_error& e) {
        std::cerr << "Boost.Beast Error: " << e.what() << ", Code: " << e.code() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, const char* argv[]) {
    std::cout << argc << std::endl;
    config->setManager(manager);
    manager->startThreadPool();
    if(argc == 2) {
        if(!strcmp(argv[1], "test")) {
            std::cout << "Normal test run!" << std::endl;
            return 0;
        }
    }
    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, {tcp::v4(), 8080});

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(DoHttpServer, std::move(socket)).detach();
        }

        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
