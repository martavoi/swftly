#pragma once

#include "http/router.hpp" // For request_t and response_t
#include <boost/asio/awaitable.hpp>

namespace http::handler
{

/**
 * @brief Handles requests to the root (/) endpoint.
 *
 * This handler populates the response with a simple JSON object containing
 * basic server information, such as its name and version.
 */
class RootHandler
{
  public:
    auto operator()(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>;
};

} // namespace http::handler