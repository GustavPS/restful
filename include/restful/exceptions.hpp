#include <stdexcept>
namespace restful {
class ServerException : public std::runtime_error {
public:
  explicit ServerException(const std::string &message)
      : std::runtime_error(message) {}
};

class SocketCreationException : public ServerException {
public:
  explicit SocketCreationException(const std::string &message)
      : ServerException(message) {}
};

class SocketBindException : public ServerException {
public:
  explicit SocketBindException(const std::string &message)
      : ServerException(message) {}
};

class SocketListenException : public ServerException {
public:
  explicit SocketListenException(const std::string &message)
      : ServerException(message) {}
};

class ParseException : public ServerException {
public:
  explicit ParseException(const std::string &message)
      : ServerException(message) {}
};

class HttpNotFound : public ServerException {
public:
  explicit HttpNotFound(const std::string &message)
      : ServerException(message) {}
};

class BadRequest : public ServerException {
public:
  explicit BadRequest(const std::string &message) : ServerException(message) {}
};
} // namespace restful