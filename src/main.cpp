#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <string>
#include <uuid/uuid.h>
namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;

void DoHttpServer(tcp::socket socket) {
    try {
        beast::tcp_stream stream(std::move(socket));

        beast::flat_buffer buffer;
        beast::http::request<beast::http::string_body> req;
        beast::http::read(stream, buffer, req);
        if (req.method() == beast::http::verb::get) {
            beast::http::response<beast::http::string_body> res;
            res.version(11);
            res.result(beast::http::status::ok);
            res.set(beast::http::field::server, "Boost.Beast HTTP Server");
            res.body() = "Hello, World!";
            res.prepare_payload();
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
