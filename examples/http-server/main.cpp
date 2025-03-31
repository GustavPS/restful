#include "restful/http_request.hpp"
#include "restful/http_response.hpp"
#include "restful/http_router.hpp"
#include "restful/http_server.hpp"
#include <exception>
#include <iostream>
#include <memory>

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
      response.set_body("Router MW said nonono sir please stop");

      return true;
    });

    server.start();
  } catch (const std::exception &e) {
    std::cerr << "Error" << e.what() << std::endl;
  }

  return 0;
}
