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
  void start();
  void stop();

private:
  void listen_for_connections();
  void handle_request(int client_socket);
  // const HttpRoute &find_route(const HttpRequest &request);
  int m_port;
  int m_server_fd{-1};
  std::vector<std::thread> m_client_threads{};
};

} // namespace restful