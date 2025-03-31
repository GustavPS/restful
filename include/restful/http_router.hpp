#pragma once

#include "restful/http_path.hpp"
#include "restful/http_request.hpp"
#include "restful/http_route.hpp"
#include <memory>
#include <vector>

namespace restful {
class HttpServer;
class HttpRouter {
public:
  HttpRouter(std::string path, HttpRouter *parent = nullptr);
  // Move constructor and assignment operator
  HttpRouter(HttpRouter &&other) noexcept;
  HttpRouter &operator=(HttpRouter &&other) noexcept;
  // Delete copy constructor and assignment operator
  HttpRouter(const HttpRouter &) = delete;
  HttpRouter &operator=(const HttpRouter &) = delete;

  void register_handler(HttpRequest::HttpRequestType endpoint, std::string path,
                        RequestHandler &&handler);
  void register_handler(std::shared_ptr<HttpRouter> router);
  void register_middleware(Middleware &&middleware);

protected:
  std::vector<HttpRoute> m_get_routes{};
  std::vector<HttpRoute> m_post_routes{};
  std::vector<HttpRoute> m_put_routes{};
  std::vector<HttpRoute> m_delete_routes{};
  std::vector<HttpRoute> m_patch_routes{};
  std::vector<HttpRoute> m_options_routes{};
  std::vector<HttpRoute> m_head_routes{};
  std::vector<HttpRoute> m_trace_routes{};

  std::vector<std::shared_ptr<HttpRouter>> m_http_routers{};
  std::vector<Middleware> m_middlewares{};

private:
  void execute_middlewares() const;
  const HttpRoute *
  find_route(const std::string &full_path,
             const HttpRequest::HttpRequestType &request_type) const;
  const std::vector<HttpRoute> &
  get_routes(const HttpRequest::HttpRequestType &) const;

  HttpRouter *m_parent{nullptr};
  HttpPath m_path;

  friend HttpServer; // Friend so we don't expose find_route and path
  friend HttpRoute;  // I don't like this..
};
} // namespace restful