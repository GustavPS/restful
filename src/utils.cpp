#include "restful/exceptions.hpp"
#include "restful/http_header.hpp"
#include "restful/http_request.hpp"
#include "restful/http_route.hpp"
#include "restful/http_router.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unordered_map>
#include <utility>
namespace restful {
namespace detail {

std::string &to_uppercase(std::string &str) {
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  return str;
}

std::pair<HttpRequest::HttpRequestType, std::string>
parse_request_line(std::istringstream &stream) {
  static std::unordered_map<std::string, HttpRequest::HttpRequestType> const
      endpoint_table{{"GET", HttpRequest::HttpRequestType::Get},
                     {"POST", HttpRequest::HttpRequestType::Post}};
  std::string method, path, version;
  stream >> method >> path >> version;

  auto it = endpoint_table.find(to_uppercase(method));
  if (it != endpoint_table.end()) {
    return {it->second, path};
  }
  throw ParseException("Couldn't parse request type");
}

HttpHeader parse_request_headers(std::istringstream &stream) {
  HttpHeader headers{};
  std::string line{};
  std::getline(stream, line); // Skip the first line (request_line)
  while (std::getline(stream, line) && line != "\r") {
    size_t colon_pos = line.find(": ");
    if (colon_pos != std::string::npos) {
      std::string key = line.substr(0, colon_pos);
      std::string value = line.substr(colon_pos + 2);
      headers.set(std::move(key), std::move(value));
    }
  }
  return headers;
}

json parse_request_body(std::istringstream &stream, int content_length) {
  std::string body(content_length, '\0');
  stream.read(&body[0], content_length);

  try {
    return json::parse(body);
  } catch (const std::exception &e) {
    std::cerr << "JSON Parse error: " << e.what() << std::endl;
    throw ParseException(e.what());
  }
}

std::string read_request(int client_socket) {
  char buffer[8192];
  std::string request_data{};
  ssize_t bytes_recieved;

  while ((bytes_recieved = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) >
         0) {
    buffer[bytes_recieved] = '\0'; // null terminate the string
    request_data.append(buffer, bytes_recieved);
    if (request_data.find("\r\n\r\n") != std::string::npos) {
      // We have found the full string
      break;
    }
  }
  if (bytes_recieved <= 0) {
    throw BadRequest("Couldn't read client request");
  }
  return request_data;
}

HttpRequest parse_request(const std::string &request_string) {
  std::istringstream stream(request_string);
  std::string line{};

  const auto [request_type, path] = parse_request_line(stream);
  HttpHeader headers = parse_request_headers(stream);
  json body{};
  if (headers.has("Content-Length")) {
    body = parse_request_body(stream,
                              std::stoi(headers.get("Content-Length")->get()));
  }

  HttpRequest request{path, request_type, headers, body};
  return request;
}

std::string path_to_regexp(const std::string &path) {
  return std::regex_replace(path, std::regex(":(([^/]+))"), R"(([^/]+))");
  // return std::regex("^" + regexPath + "$");
}

std::vector<std::string> path_to_param_keys(const std::string &path) {
  std::vector<std::string> keys = {};
  std::smatch matches;
  auto begin = path.cbegin();
  while (
      std::regex_search(begin, path.cend(), matches, std::regex(":([^/]+)"))) {
    // TODO: check for duplicates?
    keys.push_back(matches[1].str());
    // Move the begin iterator to the end of the current match
    begin = matches[0].second;
  }
  return keys;
}

} // namespace detail
} // namespace restful