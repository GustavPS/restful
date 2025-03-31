#include "restful/http_server.hpp"
#include "detail/utils.hpp"
#include "restful/exceptions.hpp"
#include "restful/http_request.hpp"
#include "restful/http_response.hpp"
#include "restful/http_route.hpp"
#include "restful/http_router.hpp"
#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

using json = nlohmann::json;

namespace restful {
HttpServer::HttpServer(int port) : HttpRouter("/"), m_port(port) {}
void handle_client(int client_socket);

void HttpServer::start() {
  m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_server_fd == -1) {
    throw SocketCreationException("Failed to create socket");
  }

  int opt = 1;
  if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) !=
      0) {
    std::cerr << "Error setting SO_REUSEADDR" << std::endl;
    throw SocketBindException("Failed setting reusable");
  }

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(m_port);

  if (bind(m_server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    close(m_server_fd);
    throw SocketBindException("Failed to bind");
  }

  if (listen(m_server_fd, 5) < 0) {
    close(m_server_fd);
    throw SocketListenException("Failed to listen");
  }

  std::cout << "Server listening on port " << m_port << "\n";

  listen_for_connections();
  close(m_server_fd);
}

void HttpServer::stop() {
  for (auto &client_thread : m_client_threads) {
    if (client_thread.joinable()) {
      client_thread.join();
    }
  }
}
void HttpServer::register_middleware(ErrorHandler &&handler) {
  m_error_handlers.push_back(std::forward<ErrorHandler>(handler));
}

void HttpServer::listen_for_connections() {
  if (m_server_fd == -1) {
    std::cerr << "Server not running\n";
    return; // Throw error?
  }
  while (true) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_len{sizeof(client_addr)};
    client_socket =
        accept(m_server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
      std::cerr << "Failed to accept client\n";
      continue;
    }
    std::cout << "New client connected\n";

    std::thread client_thread(&HttpServer::handle_request, this, client_socket);
    m_client_threads.push_back(std::move(client_thread));
  }
}

// TODO: status_text enum?
void send_response(int client_socket, HttpResponse &response) {
  // Convert JSON object to string
  std::string json_response = response.m_body.dump(4);

  std::stringstream client_response{};
  client_response << "HTTP/1.1 " + std::to_string(response.m_status_code) +
                         " " + response.get_status_message() + "\r\n";
  client_response << "Content-Type: application/json\r\n";
  client_response << "Content-Length: " << json_response.size() << "\r\n";
  client_response << "\r\n"; // End of headers

  client_response << json_response;

  send(client_socket, client_response.str().c_str(),
       client_response.str().size(), 0);
}

void HttpServer::handle_request(int client_socket) {
  std::string request_data{};
  try {
    request_data = restful::detail::read_request(client_socket);
  } catch (BadRequest error) {
    HttpResponse response{};
    response.set_status_code(400);
    send_response(client_socket, response);
    return;
  }

  HttpResponse response{};
  std::optional<HttpRequest> request;
  const HttpRoute *route;

  // Parse request, can fail. In that case emplace should fail sending us to the
  // catch
  try {
    request.emplace(detail::parse_request(request_data));
    route = find_route(request->full_path, request->request_type);
    if (route == nullptr) {
      response.set_status_code(404);
      send_response(client_socket, response);
      return;
    }
    request->set_params(route->extract_path_params(request->full_path));
  } catch (ServerException &error) {
    response.set_status_code(error.get_code());
    send_response(client_socket, response);
    return;
  } catch (...) {
    response.set_status_code(500);
    send_response(client_socket, response);
    return;
  }

  // Handle the endpoint, can fail because of user defined bugs
  try {
    route->execute(request.value(), response);
  } catch (ServerException &error) {
    response.set_status_code(error.get_code());
    for (const auto &handler : m_error_handlers) {
      handler(error, request.value(), response);
    }
  } catch (std::exception &error) {
    response.set_status_code(500);
    for (const auto &handler : m_error_handlers) {
      handler(error, request.value(), response);
    }
  }
  send_response(client_socket, response);
}

} // namespace restful
