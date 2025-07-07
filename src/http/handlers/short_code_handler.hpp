#pragma once

#include "encode/encoder.hpp"
#include "http/router.hpp" // For request_t and response_t
#include "storage/storage_service.hpp"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>

namespace http::handler
{

/**
 * @brief Handles short code redirects and fallback 404 responses.
 *
 * This handler first attempts to process the request as a short code redirect.
 * If the path looks like a valid short code and exists in storage, it performs the redirect.
 * Otherwise, it returns a 404 Not Found response.
 */
class ShortCodeHandler
{
  public:
    ShortCodeHandler(boost::asio::any_io_executor executor, encode::Encoder encoder, storage::StorageService storage);

    auto operator()(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>;

  private:
    boost::asio::any_io_executor executor_;
    encode::Encoder encoder_;
    storage::StorageService storage_;

    // Check if path looks like a short code and handle redirect
    [[nodiscard]] auto try_redirect(std::string_view path, response_t *res) const -> boost::asio::awaitable<bool>;

    // Extract short code from path (e.g., "/abc123" -> "abc123")
    [[nodiscard]] auto extract_short_code(std::string_view path) const -> std::string_view;
};

} // namespace http::handler