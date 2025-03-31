#include "restful/http_router.hpp"
#include "restful/exceptions.hpp"
#include "restful/http_path.hpp"
#include "restful/http_request.hpp"
#include <algorithm>
#include <memory>
#include <regex>
#include <utility>

namespace restful {

HttpRouter::HttpRouter(std::string path, HttpRouter *router)
    : m_path(path), m_parent(router) {};

HttpRouter::HttpRouter(HttpRouter &&other) noexcept
    : m_path(std::move(other.m_path)), m_routes(std::move(other.m_routes)),
      m_http_routers(std::move(other.m_http_routers)),
      m_middlewares(std::move(other.m_middlewares)), m_parent(other.m_parent) {
  other.m_parent = nullptr;
  for (auto &routes : m_routes) {
    for (auto &route : routes) {
      route.m_parent = this;
    }
  }
  for (auto &router : m_http_routers) {
    router->m_parent = this;
  }
};
HttpRouter &HttpRouter::operator=(HttpRouter &&other) noexcept {
  if (this != &other) {
    m_path = std::move(other.m_path);
    m_routes = std::move(other.m_routes);
    m_http_routers = std::move(other.m_http_routers);
    m_middlewares = std::move(other.m_middlewares);
    m_parent = other.m_parent;
    other.m_parent = nullptr;
  }
  for (auto &routes : m_routes) {
    for (auto &route : routes) {
      route.m_parent = this;
    }
  }
  for (auto &router : m_http_routers) {
    router->m_parent = this;
  }
  return *this;
}

void HttpRouter::register_handler(HttpRequest::HttpRequestType endpoint,
                                  std::string path, RequestHandler &&handler) {

  auto &routes = m_routes[static_cast<int>(endpoint)];
  // /my/path/:id/abc
  for (auto &entry : routes) {
    if (entry.path.get_original_path() == path) {
      entry.set_handler(std::forward<RequestHandler>(handler));
      return; // Replaced handler
    }
  }
  routes.emplace_back(path, std::forward<RequestHandler>(handler), this);
}

void HttpRouter::register_handler(std::shared_ptr<HttpRouter> router) {
  bool replaced = false;
  auto it = std::find_if(m_http_routers.begin(), m_http_routers.end(),
                         [&router](std::shared_ptr<HttpRouter> element) {
                           return element->m_path.get_original_path() ==
                                  router->m_path.get_original_path();
                         });
  router->m_parent = this;
  if (it != m_http_routers.end()) {
    *it = std::move(router);
  } else {
    m_http_routers.push_back(router);
  }
}

void HttpRouter::register_middleware(Middleware &&middleware) {
  m_middlewares.push_back(std::forward<Middleware>(middleware));
}

const HttpRoute *
HttpRouter::find_route(const std::string &full_path,
                       const HttpRequest::HttpRequestType &request_type) const {
  std::string working_path = full_path;
  // Get correct route
  const std::vector<HttpRoute> &routes =
      m_routes[static_cast<int>(request_type)];
  const auto it = std::find_if(
      routes.cbegin(), routes.cend(), [&full_path](const HttpRoute &route) {
        return std::regex_match(full_path, route.path.get_regex_path());
      });

  // If we found the correct route
  if (it != routes.end()) {
    return &(*it);
  }

  // Check in nested routers
  for (const std::shared_ptr<HttpRouter> router : m_http_routers) {
    std::smatch match;
    // If it matches the beginning of the routers path
    if (std::regex_search(working_path, match,
                          router->m_path.get_regex_path()) &&
        match.position() == 0) {
      // Remove the routers part of the path
      working_path =
          std::regex_replace(working_path, router->m_path.get_regex_path(), "");
      const HttpRoute *route = router->find_route(working_path, request_type);
      if (route != nullptr) {
        return route;
      }
    }
  }
  return nullptr;
}

} // namespace restful