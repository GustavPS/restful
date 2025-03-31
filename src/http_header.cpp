#include "restful/http_header.hpp"
#include <optional>
#include <string>

namespace restful {
// Can these be references?
void HttpHeader::set(std::string name, std::string value) {
  m_headers[name] = value;
}
bool HttpHeader::has(const std::string &name) const {
  auto it = m_headers.find(name);
  return it != m_headers.end();
}

std::optional<std::reference_wrapper<const std::string>>
HttpHeader::get(const std::string &name) const {
  auto it = m_headers.find(name);
  if (it != m_headers.end()) {
    return it->second;
  }
  return std::nullopt;
}

void HttpHeader::remove(const std::string &name) {
  auto it = m_headers.find(name);
  if (it != m_headers.end()) {
    m_headers.erase(it);
  }
}
} // namespace restful