#include "restful/http_route.hpp"
#include "restful/http_request.hpp"
#include "restful/http_response.hpp"
#include <cassert>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace restful {
HttpRoute::HttpRoute(std::string path, RequestHandler handler,
                     HttpRouter *router)
    : path(path), m_handler(handler), m_parent(router) {}

HttpRoute::HttpRoute(HttpRoute &&other) noexcept
    : path(std::move(other.path)), m_handler(std::move(other.m_handler)),
      m_parent(other.m_parent) {
  other.m_parent = nullptr;
};

void HttpRoute::set_handler(RequestHandler handler) { m_handler = handler; }

void HttpRoute::execute(const HttpRequest &request,
                        HttpResponse &response) const {
  if (!execute_middlewares(request, response)) {
    return;
  }
  m_handler(request, response);
}

bool HttpRoute::execute_middlewares(const HttpRequest &request,
                                    HttpResponse &response) const {
  HttpRouter *parent = m_parent;
  while (parent != nullptr) {
    for (auto mw : parent->m_middlewares) {
      bool result = mw(request, response);
      if (!result) {
        return false;
      }
    }
    parent = parent->m_parent;
  }
  return true;
}

std::regex HttpRoute::get_full_path_regex() const {
  std::string regex_string;
  // Reserve enough space for the regex string, expecting 5 layers of nesting
  regex_string.reserve(5 * path.get_regex_string().size());
  regex_string.insert(0, path.get_regex_string());

  HttpRouter *parent = m_parent;
  while (parent != nullptr) {
    regex_string.insert(0, parent->m_path.get_regex_string());
    parent = parent->m_parent;
  }
  // Replace double slashes with a single slash
  regex_string = std::regex_replace(std::string(regex_string),
                                    std::regex(R"(//)"), R"(/)");
  std::cout << "full regex: " << regex_string << std::endl;
  return std::regex(regex_string);
}

std::map<std::string, std::string>
HttpRoute::extract_path_params(const std::string &full_path) const {
  const std::regex regex = std::regex(path.get_regex_string() + "$");
  std::string working_path = full_path;

  std::smatch matches;
  std::map<std::string, std::string> params{};

  if (std::regex_search(working_path, matches, regex)) {
    assert(matches.size() - 1 == path.m_path_params.size());

    // First match is the full match so skip it
    for (size_t i = 1; i < matches.size(); ++i) {
      params[path.m_path_params[i - 1]] = matches[i];
    }
  }
  // Consume the matched path
  working_path =
      std::regex_replace(working_path, regex, ""); // Remove the matched part

  HttpRouter *parent = m_parent;
  while (parent != nullptr) {
    const std::regex regex =
        std::regex(parent->m_path.get_regex_string() + "$");
    if (std::regex_search(working_path, matches, regex)) {
      assert(matches.size() - 1 == parent->m_path.m_path_params.size());
      // First match is the full match so skip it
      for (size_t i = 1; i < matches.size(); ++i) {
        params[parent->m_path.m_path_params[i - 1]] = matches[i];
      }
    }
    // Consume the matched path
    working_path =
        std::regex_replace(working_path, regex, ""); // Remove the matched part
    parent = parent->m_parent;
  }

  return params;
}

/*
std::map<std::string, std::string>
HttpRoute::extract_path_parameters(const std::string &path) const {
  std::smatch matches;
  std::map<std::string, std::string> params{};
  if (std::regex_match(path, matches, m_regex_path)) {
    std::cout << "Found matches:" << std::endl;
    // Should this really be an assert?
    assert(matches.size() - 1 == m_path_params.size());
    // First match is the full match so skip it
    for (size_t i = 1; i < matches.size(); ++i) {
      params[m_path_params[i - 1]] = matches[i];
    }
  }
  return params;
}
*/
} // namespace restful