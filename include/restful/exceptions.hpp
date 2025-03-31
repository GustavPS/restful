#include <stdexcept>
namespace restful {
class ServerException : public std::runtime_error {
public:
  explicit ServerException(const std::string &message, int code)
      : std::runtime_error(message), m_code(code) {}
  int get_code() const { return m_code; }

private:
  int m_code{500};
};

class SocketCreationException : public ServerException {
public:
  explicit SocketCreationException(const std::string &message)
      : ServerException(message, 500) {}
};

class SocketBindException : public ServerException {
public:
  explicit SocketBindException(const std::string &message)
      : ServerException(message, 500) {}
};

class SocketListenException : public ServerException {
public:
  explicit SocketListenException(const std::string &message)
      : ServerException(message, 500) {}
};

class ParseException : public ServerException {
public:
  explicit ParseException(const std::string &message)
      : ServerException(message, 500) {}
};

class HttpNotFound : public ServerException {
public:
  explicit HttpNotFound(const std::string &message)
      : ServerException(message, 404) {}
};

class BadRequest : public ServerException {
public:
  explicit BadRequest(const std::string &message)
      : ServerException(message, 400) {}
};
} // namespace restful