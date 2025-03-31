#include "restful/http_response.hpp"
namespace restful {
void HttpResponse::set_status_code(int status_code) {
  m_status_code = status_code;
}
void HttpResponse::set_body(json body) { m_body = body; }

std::string HttpResponse::get_status_message() {
  switch (m_status_code) {
  case 200:
    return "OK";
  case 404:
    return "Not Found";
  case 500:
    return "Internal Server Error";
  default:
    return "Unknown"; // TODO throw error, also m_status_code should not be an
                      // int? enum?
  }
}
} // namespace restful