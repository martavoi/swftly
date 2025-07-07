#include "new_short_code_handler.hpp"
#include <boost/asio/awaitable.hpp>
#include <boost/json.hpp>
#include <boost/system/system_error.hpp>
#include <string_view>

namespace http::handler
{

namespace json = boost::json;

NewShortCodeHandler::NewShortCodeHandler(boost::asio::any_io_executor executor, encode::Encoder encoder,
                                         storage::StorageService storage)
    : executor_{std::move(executor)}, encoder_{std::move(encoder)}, storage_{std::move(storage)}
{
}

auto NewShortCodeHandler::operator()(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>
{
    auto bad_req = [&](std::string_view error_message)
    {
        res->result(http::status::bad_request);
        res->set(http::field::content_type, "application/json");
        res->body() = json::serialize(json::object{{"error", error_message}});
    };

    // Validate JSON input first (synchronous validation)
    json::value jv;
    try
    {
        jv = json::parse(req->body());
    }
    catch (const boost::system::system_error &)
    {
        bad_req("Invalid JSON format in request body.");
        co_return;
    }

    if (!jv.is_object())
    {
        bad_req("Request body must be a JSON object.");
        co_return;
    }

    const json::value *url_val = jv.get_object().if_contains("url");
    if (!url_val)
    {
        bad_req("Missing 'url' field in request body.");
        co_return;
    }

    if (!url_val->is_string())
    {
        bad_req("'url' field must be a string.");
        co_return;
    }

    const std::string_view url = url_val->as_string();
    if (url.empty())
    {
        bad_req("'url' field cannot be empty.");
        co_return;
    }

    // Now do the async Redis operations
    try
    {
        auto index = co_await storage_.generate_next_id();
        auto short_code = encoder_.encode(index);

        co_await storage_.store_url(index, url);

        json::object success_body;
        success_body["short_code"] = short_code;
        success_body["url"] = url;

        res->result(http::status::created);
        res->set(http::field::content_type, "application/json");
        res->body() = json::serialize(success_body);
    }
    catch (const std::exception &e)
    {
        res->result(http::status::internal_server_error);
        res->set(http::field::content_type, "application/json");
        res->body() = json::serialize(json::object{{"error", "Internal server error"}});
    }
}

} // namespace http::handler