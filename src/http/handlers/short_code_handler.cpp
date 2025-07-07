#include "short_code_handler.hpp"
#include <boost/json.hpp>

namespace http::handler
{

namespace json = boost::json;

ShortCodeHandler::ShortCodeHandler(boost::asio::any_io_executor executor, encode::Encoder encoder,
                                   storage::StorageService storage)
    : executor_(std::move(executor)), encoder_(std::move(encoder)), storage_(std::move(storage))
{
}

auto ShortCodeHandler::operator()(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>
{
    try
    {
        if (co_await try_redirect(req->target(), res))
        {
            co_return;
        }
    }
    catch (const std::exception &)
    {
        res->result(http::status::internal_server_error);
        co_return;
    }

    res->result(http::status::not_found);
    res->set(http::field::content_type, "application/json");
    res->body() = json::serialize(json::object{{"error", "Not found"}});
    co_return;
}

auto ShortCodeHandler::try_redirect(std::string_view path, response_t *res) const -> boost::asio::awaitable<bool>
{
    // Extract short code from path (e.g., "/abc123" -> "abc123")
    auto short_code = extract_short_code(path);

    if (short_code.empty())
    {
        co_return false; // Not a valid short code format
    }

    // Try to decode the short code
    auto decode_result = encoder_.decode(short_code);
    if (!decode_result.has_value())
    {
        co_return false; // Invalid short code format
    }

    auto id = decode_result.value();

    // Look up the URL
    auto url = co_await storage_.get_url(id);
    if (!url.has_value())
    {
        co_return false; // Short code not found in storage
    }

    // Return redirect response
    res->result(http::status::found); // 302 redirect
    res->set(http::field::location, url.value());
    res->set(http::field::content_type, "text/html");

    co_return true; // Successfully handled
}

auto ShortCodeHandler::extract_short_code(std::string_view path) const -> std::string_view
{
    // Remove leading slash: "/abc123" -> "abc123"
    if (path.starts_with('/'))
    {
        path.remove_prefix(1);
    }

    // For now, assume the entire remaining path is the short code
    // We could add more validation here (e.g., check length, character set)
    return path;
}

} // namespace http::handler