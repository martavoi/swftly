#pragma once

#include "encode/encoder.hpp"
#include "http/router.hpp"
#include "storage/storage_service.hpp"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>

namespace http::handler
{

/**
 * @brief Handles requests to create a new short code.
 *
 * This handler is responsible for creating a new short code for a given URL.
 */
class NewShortCodeHandler
{
  public:
    NewShortCodeHandler(boost::asio::any_io_executor executor, encode::Encoder encoder,
                        storage::StorageService storage);
    auto operator()(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>;

  private:
    boost::asio::any_io_executor executor_;
    encode::Encoder encoder_;
    storage::StorageService storage_;
};

} // namespace http::handler