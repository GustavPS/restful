#pragma once

#include "restful/http_header.hpp"
#include "restful/http_server.hpp"
#include <map>
#include <string>
namespace restful {
class HttpResponse {
public:
  void set_status_code(int status_code);
  void set_body(json body);
  friend void send_response(int client_socket, HttpResponse &response);
  HttpHeader headers{};

private:
  std::string get_status_message();
  int m_status_code{200};
  std::map<std::string, std::string> m_headers{};
  json m_body{};
};
} // namespace restful