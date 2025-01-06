#pragma once

#include <bits/stdc++.h>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

class HttpClient
{
  public:
    HttpClient(boost::asio::io_context&);
    std::string post(const std::string&, const std::string&,
                     const std::string&, const std::string&,
                     const std::string&);
    std::string get(const std::string&, const std::string&,
                    const std::string&);

  private:
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::tcp_stream stream_;
};

class HttpSession : public std::enable_shared_from_this<HttpSession>
{
  public:
    HttpSession(boost::asio::io_context&,
                boost::asio::ip::tcp::socket);
    void start();

  private:
    void do_read();
    void log_request();
    void handle_request();

    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body>
        request_;
};

class HttpServer : public std::enable_shared_from_this<HttpServer>
{
  public:
    HttpServer(boost::asio::io_context&, short);
    void run();

  private:
    void do_accept();
    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
};