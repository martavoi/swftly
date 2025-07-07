#include "root_handler.hpp"
#include <boost/json.hpp>

namespace http::handler
{

namespace json = boost::json;

auto RootHandler::operator()([[maybe_unused]] const request_t *req, response_t *res) const
    -> boost::asio::awaitable<void>
{
    json::object body;
    body["server"] = "Swftly";
    body["version"] = "1.0.0";

    res->result(http::status::ok);
    res->set(http::field::content_type, "application/json");
    res->body() = json::serialize(body);

    co_return;
}

} // namespace http::handler