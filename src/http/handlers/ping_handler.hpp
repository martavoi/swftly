#pragma once

#include "http/router.hpp" // For request_t and response_t
#include <boost/asio/awaitable.hpp>

namespace http::handler
{

/**
 * @brief Handles health-check requests to the /ping endpoint.
 *
 * This handler populates the response with a simple JSON object to indicate
 * that the server is alive and responding to requests.
 */
class PingHandler
{
  public:
    auto operator()(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>;
};

} // namespace http::handler