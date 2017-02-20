/* Source:
    http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/example/echo/blocking_tcp_echo_server.cpp
*/

#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <utility>
#include "server.h"
#include "echo_handler.h"
#include "static_file_handler.h"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, NginxConfig* config)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), std::stoi(config->getConfigValue("port")))),
      config_(config) {
    create_handler_map(config_);
}

Server::~Server() {}

// Start accepting requests
void Server::run() {
    while (true) {
        tcp::socket socket(io_service_);
        acceptor_.accept(socket);

        char data[MAX_LENGTH];
        memset(data, 0, MAX_LENGTH);
        boost::system::error_code error;
        socket.read_some(boost::asio::buffer(data), error);

        if (error) {
            std::cout << "Error reading request" << std::endl;
        } else {
            std::string message = handle_read(data);
            boost::asio::write(socket, boost::asio::buffer(message, message.size()));
        }
        socket.close();
    }
}

std::string Server::handle_read(const char* data) {
    std::string data_string = std::string(data);
    auto request = Request::Parse(data_string);
    Response response;
    std::string longest_prefix = find_uri_prefix(request->uri());
    RequestHandler::Status status = handler_map_[longest_prefix]->HandleRequest(*request, &response);

    // TODO: check status
    if (status == RequestHandler::OK) {
        std::cout << "handle request OK!" << std::endl;
    }
    return response.ToString();
}

void Server::create_handler_map(NginxConfig* config) {
    std::vector<std::shared_ptr<NginxConfigStatement>> statements = config->statements_;
    
    for (size_t i = 0; i < statements.size(); i++) {
        if (statements[i]->tokens_[0] == "path") {
            std::string uri_prefix = statements[i]->tokens_[1];

            if (statements[i]->tokens_[2] == "EchoHandler") {
                handler_map_[uri_prefix] = std::unique_ptr<RequestHandler>(new EchoHandler());
            } else if (statements[i]->tokens_[2] == "StaticHandler") {
                handler_map_[uri_prefix] = std::unique_ptr<RequestHandler>(new StaticFileHandler());
            } else {
                continue;
            }

            // TODO: Check status
            handler_map_[uri_prefix]->Init(uri_prefix, *(statements[i]->child_block_));
        }
    }
}

// Finds the correct handler using longest uri prefix matching
std::string Server::find_uri_prefix(std::string request_uri) {
    std::string longest_prefix_match = "";
    for (auto it = handler_map_.begin(); it != handler_map_.end(); it++) {
        std::string prefix = it->first;
        if (is_uri_prefix(prefix, request_uri) &&
            prefix.size() > longest_prefix_match.size()) {
            longest_prefix_match = prefix;
        }
    }
    return longest_prefix_match;
}

// Returns true if short_str is a uri prefix of long_str
// e.g. /foo is a uri prefix of /foo/boo, but not of /fooo
bool Server::is_uri_prefix(std::string short_str, std::string long_str) {
    if (short_str.size() > long_str.size()) return false;

    if ((long_str.substr(0, short_str.size()) == short_str) &&
        (short_str == long_str || long_str[short_str.size()] == '/')) {
        return true;
    } else {
        return false;
    }
}
