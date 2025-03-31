#include "restful/http_request.hpp"
#include "restful/http_response.hpp"
#include "restful/http_router.hpp"
#include "restful/http_server.hpp"
#include <exception>
#include <iostream>
#include <memory>

class CustomException : public std::exception {
public:
  explicit CustomException(const std::string &message) : m_message(message) {}
  const char *what() const noexcept override { return m_message.c_str(); }

private:
  std::string m_message;
};

int main() {
  try {
    auto testLambda = [](const restful::HttpRequest &request,
                         restful::HttpResponse &response) {
      restful::json json_response{};
      json_response["hello"] = "world";
      response.set_body(json_response);
    };

    restful::HttpServer server{8093};
    auto router = std::make_shared<restful::HttpRouter>("/first/:mid");
    router->register_handler(restful::HttpRequest::HttpRequestType::Get,
                             "/testLambda", testLambda);
    router->register_handler(restful::HttpRequest::HttpRequestType::Get,
                             "/testLambda2", testLambda);
    router->register_handler(restful::HttpRequest::HttpRequestType::Get,
                             "/hello/:id",
                             [](const restful::HttpRequest &request,
                                restful::HttpResponse &response) {
                               restful::json json{};
                               std::cout << "hi from hello" << std::endl;
                               json["id"] = request.get_param("id")->get();
                               json["mid"] = request.get_param("mid")->get();
                               response.set_body(json);
                             });
    router->register_handler(restful::HttpRequest::HttpRequestType::Get,
                             "/hello2",
                             [](const restful::HttpRequest &request,
                                restful::HttpResponse &response) {
                               restful::json json{};
                               std::cout << "hi from hello2" << std::endl;
                               response.set_body(json);
                             });

    server.register_middleware([](const restful::HttpRequest &request,
                                  restful::HttpResponse &response) {
      std::cout << "HI FROM MW" << std::endl;
      return true;
    });

    server.register_handler(router);

    router->register_middleware([](const auto &request, auto &response) {
      std::cout << "Hi from router mw" << std::endl;
      // throw CustomException("No cookies for you :(");
      return true;
    });

    server.register_middleware(
        [](const std::exception &error, const auto &request, auto &response) {
          const std::string error_message = error.what();
          response.set_body("Something happened: " + error_message);
          response.set_status_code(500);
        });

    server.start();
  } catch (const std::exception &e) {
    std::cerr << "Error" << e.what() << std::endl;
  }

  return 0;
}
