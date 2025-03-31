#pragma once

#include <map>
#include <optional>
#include <string>
namespace restful {
class HttpHeader {
public:
  std::optional<std::reference_wrapper<const std::string>>
  get(const std::string &name) const;
  bool has(const std::string &name) const;
  void set(const std::string name, const std::string value);
  void remove(const std::string &name);

private:
  std::map<std::string, std::string> m_headers{};
};
} // namespace restful