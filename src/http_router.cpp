#include "restful/http_router.hpp"
#include "restful/exceptions.hpp"
#include "restful/http_path.hpp"
#include "restful/http_request.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <regex>
#include <utility>

namespace restful {

static void move_routes_to_parent(std::vector<HttpRoute> &routes,
                                  HttpRouter *parent) {
  for (auto &route : routes) {
    route.m_parent = parent;
  }
}

HttpRouter::HttpRouter(std::string path, HttpRouter *router)
    : m_path(path), m_parent(router) {};

HttpRouter::HttpRouter(HttpRouter &&other) noexcept
    : m_path(std::move(other.m_path)),
      m_get_routes(std::move(other.m_get_routes)),
      m_post_routes(std::move(other.m_post_routes)),
      m_put_routes(std::move(other.m_put_routes)),
      m_delete_routes(std::move(other.m_delete_routes)),
      m_patch_routes(std::move(other.m_patch_routes)),
      m_options_routes(std::move(other.m_options_routes)),
      m_http_routers(std::move(other.m_http_routers)),
      m_head_routes(std::move(other.m_head_routes)),
      m_trace_routes(std::move(other.m_trace_routes)),
      m_middlewares(std::move(other.m_middlewares)), m_parent(other.m_parent) {
  other.m_parent = nullptr;
  // Is this needed? Can I make this happen in the respective move thing? They
  // need access to "this"
  move_routes_to_parent(m_get_routes, this);
  move_routes_to_parent(m_post_routes, this);
  move_routes_to_parent(m_put_routes, this);
  move_routes_to_parent(m_delete_routes, this);
  move_routes_to_parent(m_patch_routes, this);
  move_routes_to_parent(m_options_routes, this);
  move_routes_to_parent(m_head_routes, this);
  move_routes_to_parent(m_trace_routes, this);
  // Set m_http_routers parents to this
  for (auto &router : m_http_routers) {
    router->m_parent = this;
  }
};
HttpRouter &HttpRouter::operator=(HttpRouter &&other) noexcept {
  if (this != &other) {
    m_path = std::move(other.m_path);
    m_get_routes = std::move(other.m_get_routes);
    m_post_routes = std::move(other.m_post_routes);
    m_put_routes = std::move(other.m_put_routes);
    m_delete_routes = std::move(other.m_delete_routes);
    m_patch_routes = std::move(other.m_patch_routes);
    m_options_routes = std::move(other.m_options_routes);
    m_head_routes = std::move(other.m_head_routes);
    m_trace_routes = std::move(other.m_trace_routes);
    m_http_routers = std::move(other.m_http_routers);
    m_middlewares = std::move(other.m_middlewares);
    m_parent = other.m_parent;
    other.m_parent = nullptr;
  }
  move_routes_to_parent(m_get_routes, this);
  move_routes_to_parent(m_post_routes, this);
  move_routes_to_parent(m_put_routes, this);
  move_routes_to_parent(m_delete_routes, this);
  move_routes_to_parent(m_patch_routes, this);
  move_routes_to_parent(m_options_routes, this);
  move_routes_to_parent(m_head_routes, this);
  move_routes_to_parent(m_trace_routes, this);
  for (auto &router : m_http_routers) {
    router->m_parent = this;
  }
  return *this;
}

void HttpRouter::register_handler(HttpRequest::HttpRequestType endpoint,
                                  std::string path, RequestHandler &&handler) {
  // /my/path/:id/abc
  for (auto &entry : m_get_routes) {
    if (entry.path.get_original_path() == path) {
      entry.set_handler(std::forward<RequestHandler>(handler));
      return; // Replaced handler
    }
  }
  m_get_routes.emplace_back(path, std::forward<RequestHandler>(handler), this);
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
  const std::vector<HttpRoute> &routes = get_routes(request_type);
  const auto it = std::find_if(
      routes.cbegin(), routes.cend(), [&full_path](const HttpRoute &route) {
        return std::regex_match(full_path, route.path.get_regex_path());
      });

  // If we found the correct route
  if (it != m_get_routes.end()) {
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

const std::vector<HttpRoute> &
HttpRouter::get_routes(const HttpRequest::HttpRequestType &request_type) const {
  switch (request_type) {
  case restful::HttpRequest::HttpRequestType::Get:
    return m_get_routes;
  case restful::HttpRequest::HttpRequestType::Post:
    return m_post_routes;
  case restful::HttpRequest::HttpRequestType::Put:
    return m_put_routes;
  case restful::HttpRequest::HttpRequestType::Delete:
    return m_delete_routes;
  case restful::HttpRequest::HttpRequestType::Patch:
    return m_patch_routes;
  case restful::HttpRequest::HttpRequestType::Options:
    return m_options_routes;
  case restful::HttpRequest::HttpRequestType::Head:
    return m_head_routes;
  case restful::HttpRequest::HttpRequestType::Trace:
    return m_trace_routes;
  }
  throw HttpNotFound("Route not found");
}

} // namespace restful