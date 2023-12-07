#include "httpServer.hpp"

/**
 * @file httpServer.cpp
 * @brief Содержит определение класса HttpServer
 */


using namespace net;

HttpServer::HttpServer(short unsigned port = 8080, std::filesystem::path path = "base.json"):
    acceptor(io_context, {tcp::v4(), port}) {
    ManagerBuilder managerBuilder;
    manager = managerBuilder.Construct(path);
    logger_ = managerBuilder.BuildLogger();
    manager->startThreadPool();
}

void HttpServer::run() {
    try {
        while (true) {
            tcp::socket socket(io_context);
            boost::system::error_code ec;
            acceptor.accept(socket, ec);
            if (ec) {
                if (logger_)
                    logger_->error("Error accepting connection: {}", ec.message());
                break;
            } else {
                std::thread(&HttpServer::handleHttpRequest, this, std::move(socket)).detach();
            }
        }

        io_context.run();
    } catch (const std::exception& e) {
        if (logger_)
            logger_->critical("Error: {}", e.what());
    }
}

void HttpServer::handleHttpRequest(tcp::socket socket) {
    try {
        beast::tcp_stream stream(std::move(socket));
        beast::flat_buffer buffer;
        beast::http::request<beast::http::string_body> req;
        beast::http::read(stream, buffer, req);
        beast::http::response<beast::http::string_body> res;

        if (req.method() == beast::http::verb::get) {
            handleRequest(static_cast<std::string>(req.target()), res);
            beast::http::write(stream, res);
        }
    } catch (const beast::system_error& e) {
        if (logger_)
            logger_->critical("Boost.Beast Error: {} , Code: {}", e.what(), e.code().to_string());
    } catch (const std::exception& e) {
        if (logger_)
            logger_->critical("Error: {}", e.what());
    }
}

void HttpServer::handleRequest(const std::string& path, beast::http::response<beast::http::string_body>& res) {
    if (path.find("/phone=") == 0) {
        std::string phone = path.substr(7);
        processPhoneCall(res, phone);
    } else if (path.find("/update") == 0) {
        processUpdate(res);
    } else {
        res.result(beast::http::status::not_found);
        res.body() = "Not Found";
    }
    res.version(11);
    res.set(beast::http::field::server, "Boost.Beast HTTP Server");
    res.prepare_payload();
}

void HttpServer::processPhoneCall(beast::http::response<beast::http::string_body>& res, std::string_view phone) {
    try {
        logger_->debug("Thread id: {}, phone: {}", std::hash<std::thread::id>{}(std::this_thread::get_id()), phone);
        auto [callID, future] = manager->addTask(phone);

        auto result = future.get();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(result.callDuration);
        res.body() = "CallID: " + std::to_string(callID) + " call duration: " + std::to_string(seconds.count()) + "s";

        workWithCallStatus(res, result.status);
    } catch (const std::future_error& e) {
        if (logger_)
            logger_->error("Caught a future_error: ", e.what());
    }
}

void HttpServer::workWithCallStatus(beast::http::response<beast::http::string_body>& res, CallStatus status) {
    std::string statusStr;
    switch (status) {
    case CallStatus::Awaiting:
        res.result(beast::http::status::ok);
        statusStr = "Awaiting";
        break;
    case CallStatus::Completed:
        res.result(beast::http::status::ok);
        statusStr = "Completed";
        break;
    case CallStatus::Duplication:
        res.result(beast::http::status::conflict);
        statusStr = "Duplication";
        break;
    case CallStatus::Overloaded:
        res.result(beast::http::status::too_many_requests);
        statusStr = "Overloaded";
        break;
    case CallStatus::Rejected:
        res.result(beast::http::status::not_acceptable);
        statusStr = "rejected";
        break;
    case CallStatus::Timeout:
        res.result(beast::http::status::request_timeout);
        statusStr = "timeout";
        break;
    }
    res.body() += "\n Status: " + statusStr + "\n";
}

void HttpServer::processUpdate(beast::http::response<beast::http::string_body>& res) {
    auto resUpd = manager->processRequestForUpdate();
    if (resUpd) {
        res.result(beast::http::status::ok);
        res.body() = "Updated";
    } else {
        res.result(beast::http::status::forbidden);
        res.body() = "Could not update";
    }
}