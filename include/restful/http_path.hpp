#pragma once

#include <regex>
#include <string>
namespace restful {
class HttpPath {
public:
  HttpPath(std::string path = "/");
  const std::string &get_original_path() const { return m_original_path; }
  const std::regex &get_regex_path() const { return m_regex_path; }
  std::map<std::string, std::string>
  extract_path_parameters(const std::string &path, const std::regex &reg) const;

  std::string get_regex_string() const { return m_regex_string; };
  std::vector<std::string> m_path_params; // Ordered

private:
  std::string m_original_path;
  std::string m_regex_string;
  std::regex m_regex_path;

  friend std::ostream &operator<<(std::ostream &os, const HttpPath &path);
};

inline std::ostream &operator<<(std::ostream &os, const HttpPath &path) {
  os << "HttpRoute: { path: " << path.m_original_path << ", params: [";

  for (size_t i = 0; i < path.m_path_params.size(); ++i) {
    os << path.m_path_params[i];
    if (i != path.m_path_params.size() - 1) {
      os << ", ";
    }
  }
  os << "] }";
  return os;
}
} // namespace restful