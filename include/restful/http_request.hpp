#pragma once

#include "restful/http_header.hpp"
#include "restful/restful.hpp"
#include <functional>
#include <optional>
#include <string>
namespace restful {

class HttpRequest {
public:
  enum HttpRequestType {
    Get,
    Post,
  };

  explicit HttpRequest(std::string full_path, HttpRequestType request_type,
                       HttpHeader headers, json body);
  void set_params(const std::map<std::string, std::string> &params);

  std::optional<std::reference_wrapper<const std::string>>
  get_param(const std::string &name) const;
  std::optional<std::reference_wrapper<const std::string>>
  get_header(const std::string &name) const;

  const HttpRequestType request_type;
  const json body{};
  const std::string full_path;

private:
  const HttpHeader m_headers;
  std::map<std::string, std::string> m_params{};
};
} // namespace restful