#pragma once
#include "restful/http_path.hpp"
#include "restful/http_request.hpp"
#include <ostream>
#include <regex>
#include <string>
namespace restful {
class HttpResponse;
class HttpRouter;
class HttpRoute {
public:
  HttpRoute(std::string path, RequestHandler handler, HttpRouter *router);
  HttpRoute(HttpRoute &&other) noexcept;
  HttpRoute(const HttpRoute &) = delete;
  void set_handler(RequestHandler handler);
  void execute(const HttpRequest &request, HttpResponse &response) const;
  std::map<std::string, std::string>
  extract_path_params(const std::string &path) const;
  std::regex get_full_path_regex() const;

  HttpPath path{};
  HttpRouter *m_parent{nullptr};

private:
  bool execute_middlewares(const HttpRequest &request,
                           HttpResponse &response) const;
  friend std::ostream &operator<<(std::ostream &os, const HttpRoute &route);

  RequestHandler m_handler;
};

inline std::ostream &operator<<(std::ostream &os, const HttpRoute &route) {
  os << route.path;
  return os;
}
} // namespace restful