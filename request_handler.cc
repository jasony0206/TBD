#include "iostream"
#include "sstream"
#include "request_handler.h"

// Request Implementations

std::unique_ptr<Request> Request::Parse(const std::string & raw_request) {
    std::unique_ptr<Request> new_request(new Request());
    new_request.raw_request_ = raw_request;
    
    Headers headers = new Headers();
    std::vector<std::string> lines = split_lines(raw_request);
    for (auto i = 0; i < lines.size() - 1; i++) {
        if (i == 0) {
        // parse first line of the request
            auto first_space = lines[i].find(" ");
            new_request.method_ = lines[i].substr(0, first_space);
            std::string temp = lines[i].substr(first_space + 1);
            auto second_space = temp.find(" ");
            new_request.uri_ = temp.substr(0, second_space);
            new_request.version_ = temp.substr(second_space + 1);
        } else {
        // parse header fields of the request
            auto first_space = lines[i].find(" ");
            std::string header = lines[i].substr(0, first_space - 1);
            std::string field = lines[i].substr(first_space + 1);
            headers.push_back(std::make_pair(header, field));
        }
    }
    new_request.headers_ = headers;
    
    return new_request;
}

std::vector<std::string> Request::split_lines(std::string request) {
    std::stringstream ss(request);
    std::string line;
    std::vector<std::string> lines;

    if (!request.empty()) {
        // tokenize each line by newline
        while(std::getline(ss,line,'\n')){
            // remove carriage return from line
            lines.push_back(line.substr(0, line.size()-1));
        }
    }
    return lines;
}

const std::string Request::raw_request() {
    return raw_request_;
}

const std::string Request::method() {
    return method_;
}

const std::string Request::uri() {
    return uri_;
}

const std::string Request::version() {
    return version_;
}

const Headers Request::headers() {
    return headers_;
}

const std::string body() {
    return body_;
}


// Response Implementations

std::string Response::to_string() {
    std::string response = build_status_line(status_code_);
    response += build_header("Content-Type", mime_type_);
    response += "\r\n" + response_body_;
    return response;
}

std::string Response::build_status_line() {
    std::string status_line = "HTTP/1.1";

    if (status_code_ == 200) {
        status_line += " 200 OK";
    } else if (status_code_ == 404) {
        status_line += " 404 Not Found";
    }

    return status_line + "\r\n";
}

std::string Response::build_header(std::string field, std::string value) {
    return field + ": " + value + "\r\n";
}

void Response::set_response_body(const std::string& response_body) {
    response_body_ = response_body;
}

void Response::set_status_code(ResponseCode status_code) {
    status_code_ = status_code;
}

void Response::set_mime_type(const std::string& mime_type) {
    mime_type_ = mime_type;
}


// RequestHandler Implementations

RequestHandler::RequestHandler() {}

RequestHandler::~RequestHandler() {}

std::string RequestHandler::build_response() {
    return "";
}

std::string RequestHandler::build_status_line(int status_code) {
    std::string status_line = "HTTP/1.1";

    if (status_code == 200) {
        status_line += " 200 OK";
    } else if (status_code == 404) {
        status_line += " 404 Not Found";
    }

    return status_line + "\r\n";
}

std::string RequestHandler::build_header(std::string field, std::string value) {
    return field + ": " + value + "\r\n";
}
