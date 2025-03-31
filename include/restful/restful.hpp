#include <functional>
#include <nlohmann/json.hpp>
namespace restful {
class HttpResponse;
class HttpRequest;

using json = nlohmann::json;
using RequestHandler = std::function<void(const HttpRequest &, HttpResponse &)>;
using Middleware = std::function<bool(const HttpRequest &, HttpResponse &)>;

} // namespace restful