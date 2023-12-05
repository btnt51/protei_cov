#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <spdlog/async.h>
#include "builder.hpp"

namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;



std::shared_ptr<Manager> manager;


void HandleHttpRequest(const std::string& path, beast::http::response<beast::http::string_body>& res) {
    if (path.find("/phone=") == 0) {
        try {
            std::string phone = path.substr(7);
            std::cout << "Thread id: " << std::this_thread::get_id() << " phone: " << phone << std::endl;
            auto [callID, future] = manager->addTask(phone);


            auto result = future.get();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(result.callDuration);
            res.body() =
                "CallID: " + std::to_string(callID) + " call duration: " + std::to_string(seconds.count()) + "s";
            std::string status;
            if (result.status == CallStatus::Completed) {
                res.result(beast::http::status::ok);
                status = "completed";
            }
            if (result.status == CallStatus::Rejected) {
                status = "rejected";
            }
            if (result.status == CallStatus::Timeout) {
                status = "timeout";
                res.result(beast::http::status::request_timeout);
            }
            if (result.status == CallStatus::Duplication) {
                res.result(beast::http::status::conflict);
                status = "Duplication";
            }
            if (result.status == CallStatus::Overloaded) {
                res.result(beast::http::status::too_many_requests);
                status = "Overloaded";
            }
            res.body() += "\n Status: " + status + "\n";
        } catch (const std::future_error &e) {
            std::cerr << "Caught a future_error: " << e.what() << std::endl;
        }
    } else if(path.find("/update") == 0) {
        auto resUpd = manager->processRequestForUpdate();
        if(resUpd) {
            res.result(beast::http::status::ok);
            res.body() = "Updated";
        } else {
            res.result(beast::http::status::forbidden);
            res.body() = "Could not update";
        }

    }
    else{
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
            HandleHttpRequest(static_cast<std::string>(req.target()), res);
            beast::http::write(stream, res);
        }
    } catch (const beast::system_error& e) {
        std::cerr << "Boost.Beast Error: " << e.what() << ", Code: " << e.code() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, const char* argv[]) {
    spdlog::init_thread_pool(8192, 1);
    ManagerBuilder builder = ManagerBuilder();
    manager = builder.Construct("base.json");
    std::cout << argc << std::endl;
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
            boost::system::error_code ec;
            acceptor.accept(socket, ec);
            if (ec) {
                std::cerr << "Error accepting connection: " << ec.message() << std::endl;
                break;
            } else {
                std::thread(DoHttpServer, std::move(socket)).detach();
            }
        }

        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
