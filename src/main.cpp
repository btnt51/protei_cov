#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>
#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <iostream>
namespace http = boost::beast::http;
using std::string_literals::operator""s;
std::string getIp() {
    const std::string host = "ifconfig.co";
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);
    boost::asio::ip::tcp::socket socket(ioc);
    boost::asio::connect(socket, resolver.resolve(host, "80"));
    http::request<http::string_body> req(http::verb::get, "/", 11);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, "curl/7.88.1");
    http::write(socket, req);
    boost::beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(socket, buffer, res);   
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    return res.body().data();
}

int main([[maybe_unused]]int argc, [[maybe_unused]]char* argv[]) { 
    spdlog::info(boost::filesystem::system_complete(argv[0]).c_str());
    spdlog::info("My ip: "s+getIp());
    return 0;
}