#include "static_file_handler.h"

StaticFileHandler::StaticFileHandler() {}

StaticFileHandler::~StaticFileHandler() {}

std::string StaticFileHandler::GetMimeType() {
    size_t pos = file_path_.find_last_of(".");
    if (pos == std::string::npos) {
        return "text/plain";
    }

    std::string extension = file_path_.substr(pos);

    if (extension == ".html")
        return "text/html";
    else if (extension == ".txt")
        return "text/plain";
    else if (extension == ".jpeg" || extension == ".jpg")
        return "image/jpeg";
    else
        return "";
}

bool StaticFileHandler::ReadFile(const std::string file_path, std::string& file_content) {
    boost::filesystem::ifstream fs(file_path);
    std::string line;
    if (fs.is_open()) {
        while (std::getline(fs, line)) {
            file_content += line + '\n';
        }
        fs.close();
        return true;
    } else {
        return false;
    }
}

Status StaticFileHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    uri_prefix_ = uri_prefix;
    root_path_ = config.statements_[0]->tokens_[1];
    return Status::OK;
}

Status StaticFileHandler::HandleRequest(const Request& request, Response* response) {
    std::string request_uri = request.uri();
    std::string file_path = root_path_ + request_uri.substr(uri_prefix_.size());

    std::string file_content = "";
    if (ReadFile(file_path, file_content)) {
        response.SetStatus(ResponseCode::OK);

        std::string content_type = GetMimeType();
        response.AddHeader("Content-Type", content_type);

        response.SetBody(file_content);
        return Status::OK;
    } else
        return Status::READ_FILE_ERROR;
}
