#include "restful/http_router.hpp"
#include "restful/exceptions.hpp"
#include "restful/http_path.hpp"
#include "restful/http_request.hpp"
#include <algorithm>
#include <iostream>
#include <regex>
#include <utility>

namespace restful {
HttpRouter::HttpRouter(std::string path, HttpRouter *router)
    : m_path(path), m_parent(router) {};

HttpRouter::HttpRouter(HttpRouter &&other) noexcept
    : m_path(std::move(other.m_path)),
      m_get_routes(std::move(other.m_get_routes)),
      m_http_routers(std::move(other.m_http_routers)),
      m_middlewares(std::move(other.m_middlewares)), m_parent(other.m_parent) {
  other.m_parent = nullptr;
  // Set m_get_routes parents to this
  for (auto &route : m_get_routes) {
    route.m_parent = this;
  }
  // Set m_http_routers parents to this
  for (auto &router : m_http_routers) {
    router.m_parent = this;
  }
};
HttpRouter &HttpRouter::operator=(HttpRouter &&other) noexcept {
  if (this != &other) {
    m_path = std::move(other.m_path);
    m_get_routes = std::move(other.m_get_routes);
    m_http_routers = std::move(other.m_http_routers);
    m_middlewares = std::move(other.m_middlewares);
    m_parent = other.m_parent;
    other.m_parent = nullptr;
  }
  for (auto &route : m_get_routes) {
    route.m_parent = this;
  }
  for (auto &router : m_http_routers) {
    router.m_parent = this;
  }
  return *this;
}

typename std::enable_if<!std::is_lvalue_reference<RequestHandler>::value,
                        void>::type
HttpRouter::register_handler(HttpRequest::HttpRequestType endpoint,
                             std::string path, RequestHandler &&handler) {
  // /my/path/:id/abc
  for (auto &entry : m_get_routes) {
    if (entry.path.get_original_path() == path) {
      entry.set_handler(std::move(handler));
      std::cout << "Replaced route: " << entry << std::endl;
      return; // Replaced handler
    }
  }
  // New path
  // HttpRoute route{path, handler, this};
  m_get_routes.emplace_back(path, handler, this);
  std::cout << "Registered route: " << m_get_routes.back() << "in "
            << m_get_routes.back().m_parent->m_path << std::endl;
}

typename std::enable_if<!std::is_lvalue_reference<HttpRouter>::value,
                        void>::type
HttpRouter::register_handler(HttpRouter &&router) {
  bool replaced = false;
  auto it = std::find_if(m_http_routers.begin(), m_http_routers.end(),
                         [&router](HttpRouter &element) {
                           return element.m_path.get_original_path() ==
                                  router.m_path.get_original_path();
                         });
  router.m_parent = this;
  if (it != m_http_routers.end()) {
    *it = std::move(router);
    std::cout << "Replaced HttpRouter" << std::endl;
  } else {
    m_http_routers.push_back(std::move(router));
    std::cout << "Inserted HttpRouter" << std::endl;
  }
}

typename std::enable_if<!std::is_lvalue_reference<Middleware>::value,
                        void>::type
HttpRouter::register_middleware(Middleware &&middleware) {
  std::cout << "insterted mw" << std::endl;
  m_middlewares.push_back(std::move(middleware));
}

const HttpRoute *
HttpRouter::find_route(const std::string &full_path,
                       const HttpRequest::HttpRequestType &request_type) const {
  // Make copy of string
  std::string working_path = full_path;

  auto search_in_handlers = [&full_path](
                                const std::vector<HttpRoute> &handlers) {
    return std::find_if(handlers.cbegin(), handlers.cend(),
                        [&full_path](const HttpRoute &route) {
                          return std::regex_match(full_path,
                                                  route.path.get_regex_path());
                        });
  };

  switch (request_type) {
  case restful::HttpRequest::HttpRequestType::Get: {
    auto it = search_in_handlers(m_get_routes);
    if (it != m_get_routes.end()) {
      return &(*it);
    }
    break;
  }
  case restful::HttpRequest::HttpRequestType::Post: {
    throw HttpNotFound("POST NOT IMPLEMENTED");
  }
  }
  for (const HttpRouter &router : m_http_routers) {
    std::smatch match;
    // If it matches the beginning of the routers path
    if (std::regex_search(working_path, match,
                          router.m_path.get_regex_path()) &&
        match.position() == 0) {
      // Remove the routers part of the path
      working_path =
          std::regex_replace(working_path, router.m_path.get_regex_path(), "");
      std::cout << "new working path " << working_path << std::endl;
      auto route = router.find_route(working_path, request_type);
      if (route != nullptr) {
        return route;
      }
    }
  }
  return nullptr;
}
} // namespace restful