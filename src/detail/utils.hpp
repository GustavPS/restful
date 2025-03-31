#pragma once

#include "restful/http_request.hpp"
#include <string>
#include <utility>
namespace restful {
namespace detail {
std::string &to_uppercase(std::string &str);
std::pair<HttpRequest::HttpRequestType, std::string>
parse_request_line(const std::string &request_line);
std::string read_request(int client_socket);
HttpRequest parse_request(const std::string &request_string);
std::string path_to_regexp(const std::string &path);
std::vector<std::string> path_to_param_keys(const std::string &path);
} // namespace detail
} // namespace restful