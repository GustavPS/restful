#include "restful/http_request.hpp"
#include "restful/http_header.hpp"
#include <utility>
namespace restful {
HttpRequest::HttpRequest(std::string full_path, HttpRequestType request_type,
                         HttpHeader headers, json body)
    : full_path(full_path), request_type(request_type), m_headers(headers),
      body(body) {}

void HttpRequest::set_params(const std::map<std::string, std::string> &params) {
  m_params = params;
}

std::optional<std::reference_wrapper<const std::string>>
HttpRequest::get_header(const std::string &name) const {
  return m_headers.get(name);
}

std::optional<std::reference_wrapper<const std::string>>
HttpRequest::get_param(const std::string &name) const {
  auto it = m_params.find(name);
  if (it != m_params.end()) {
    return it->second;
  }
  return std::nullopt;
}
} // namespace restful