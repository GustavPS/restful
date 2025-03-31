#pragma once
#include "restful/http_router.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>
namespace restful {
class HttpResponse;
class HttpServer : public HttpRouter {
public:
  explicit HttpServer(int port);
  // Bring the base implementation in to scope
  using HttpRouter::register_middleware;
  void register_middleware(ErrorHandler &&handler);
  void start();
  void stop();

private:
  void listen_for_connections();
  void handle_request(int client_socket);
  // const HttpRoute &find_route(const HttpRequest &request);
  int m_port;
  int m_server_fd{-1};
  std::vector<std::thread> m_client_threads{};
  std::vector<ErrorHandler> m_error_handlers{};
};

} // namespace restful