#include "ping_handler.hpp"
#include <boost/json.hpp>

namespace http::handler
{

namespace json = boost::json;

auto PingHandler::operator()([[maybe_unused]] const request_t *req, response_t *res) const
    -> boost::asio::awaitable<void>
{
    json::object body;
    body["status"] = "ok";
    body["message"] = "pong";

    res->result(http::status::ok);
    res->set(http::field::content_type, "application/json");
    res->body() = json::serialize(body);

    co_return;
}

} // namespace http::handler