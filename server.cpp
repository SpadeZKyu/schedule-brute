#include "server.h"
#include "schedule.h"

#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;
using namespace nlohmann;

HttpClient::HttpClient(asio::io_context& ioc)
    : resolver_(ioc), stream_(ioc)
{}

std::string HttpClient::post(const std::string& host,
                             const std::string& port,
                             const std::string& target,
                             const std::string& body,
                             const std::string& token)
{
    try
    {
        auto const results = resolver_.resolve(host, port);
        asio::connect(stream_.socket(), results);

        http::request<http::string_body> req{http::verb::post, target,
                                             11};
        req.set(http::field::host, host);
        req.set(http::field::content_type, "application/json");
        req.set(http::field::content_length,
                std::to_string(body.size()));
        req.set(http::field::authorization, "Bearer " + token);

        req.body() = body;
        req.prepare_payload();

        http::write(stream_, req);

        boost::beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream_, buffer, res);

        boost::beast::error_code ec;
        stream_.socket().shutdown(
            asio::ip::tcp::socket::shutdown_both, ec);

        return res.body();
    }
    catch (const std::exception& e)
    {
        return "Error: " + std::string(e.what());
    }
}

std::string HttpClient::get(const std::string& host,
                            const std::string& port,
                            const std::string& target)
{
    try
    {
        auto const results = resolver_.resolve(host, port);
        asio::connect(stream_.socket(), results);

        http::request<http::empty_body> req{http::verb::get, target,
                                            11};
        req.set(http::field::host, host);
        req.set(http::field::accept, "application/json");

        http::write(stream_, req);

        boost::beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream_, buffer, res);

        boost::beast::error_code ec;
        stream_.socket().shutdown(
            asio::ip::tcp::socket::shutdown_both, ec);

        return res.body();
    }
    catch (const std::exception& e)
    {
        return "Error: " + std::string(e.what());
    }
}

HttpSession::HttpSession(asio::io_context& ioc, tcp::socket socket)
    : ioc_(ioc), socket_(std::move(socket))
{}

void HttpSession::start()
{
    return do_read();
}

void HttpSession::do_read()
{
    auto self = shared_from_this();
    http::async_read(
        socket_, buffer_, request_,
        [self](beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if (!ec)
            {
                self->log_request();
                self->handle_request();
            }
            else
            {
                std::cerr << "Read error: " << ec.message()
                          << std::endl;
            }
        });
}

void HttpSession::log_request()
{
    std::cout << "Received request:\n" << request_ << std::endl;
    return;
}

void HttpSession::handle_request()
{

    HttpClient client(ioc_);
    try
    {
        std::stringstream ss;
        std::string id;
        ss << request_.target();
        ss >> id;
        std::string token =
            nlohmann::json::parse(request_.body())["access_token"];
        Schedule schedule(json::parse(
            client.get("localhost", "8080",
                       "/api/programs" + id))["data"]["data"]);
        auto gen = schedule.solve();
        while (gen.next())
        {
            std::ostringstream oss;
            for (auto& c : gen.value())
            {
                oss << c.size() << '\n';
                for (auto& [s, t] : c)
                    oss << s << ' ' << t << '\n';
            }
            json j;
            j["data"] = oss.str();
            j["program_id"] = stoi(id.substr(1));
            client.post("localhost", "8080", "/api/schemas", j.dump(),
                        token);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    auto response =
        std::make_shared<http::response<http::string_body>>(
            http::status::ok, request_.version());
    response->set(http::field::content_type, "application/json");
    response->body() = "";
    response->prepare_payload();

    auto self = shared_from_this();
    http::async_write(
        socket_, *response,
        [self, response](beast::error_code ec, std::size_t)
        {
            if (ec)
            {
                std::cerr << "Write error: " << ec.message()
                          << std::endl;
            }
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        });
    return;
}

HttpServer::HttpServer(asio::io_context& ioc, short port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "HTTP server created on port " << port << std::endl;
}

void HttpServer::run()
{
    return do_accept();
}

void HttpServer::do_accept()
{
    acceptor_.async_accept(
        [self = shared_from_this()](beast::error_code ec,
                                    tcp::socket socket)
        {
            if (!ec)
            {
                std::cout
                    << "Accepted new connection from: "
                    << socket.remote_endpoint().address().to_string()
                    << std::endl;
                std::make_shared<HttpSession>(self->ioc_,
                                              std::move(socket))
                    ->start();
            }
            else
            {
                std::cerr << "Accept error: " << ec.message()
                          << std::endl;
            }
            self->do_accept();
        });
    return;
}