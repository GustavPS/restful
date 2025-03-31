#include <exception>
#include <functional>
#include <nlohmann/json.hpp>
namespace restful {
class HttpResponse;
class HttpRequest;

using json = nlohmann::json;
using RequestHandler = std::function<void(const HttpRequest &, HttpResponse &)>;
using Middleware = std::function<bool(const HttpRequest &, HttpResponse &)>;
using ErrorHandler = std::function<void(const std::exception &err,
                                        const HttpRequest &, HttpResponse &)>;

} // namespace restful