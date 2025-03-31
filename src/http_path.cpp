#include "restful/http_path.hpp"
#include "detail/utils.hpp"
#include <cassert>

namespace restful {
HttpPath::HttpPath(std::string path) : m_original_path(path) {
  m_regex_string = detail::path_to_regexp(path);
  m_regex_path = std::regex(m_regex_string);
  m_path_params = detail::path_to_param_keys(path);
}

std::map<std::string, std::string>
HttpPath::extract_path_parameters(const std::string &path,
                                  const std::regex &reg) const {
  std::smatch matches;
  std::map<std::string, std::string> params{};
  if (std::regex_match(path, matches, reg)) {
    // Should this really be an assert?
    assert(matches.size() - 1 == m_path_params.size());
    // First match is the full match so skip it
    for (size_t i = 1; i < matches.size(); ++i) {
      params[m_path_params[i - 1]] = matches[i];
    }
  }
  return params;
}
} // namespace restful