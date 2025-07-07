#include "router.hpp"
#include <boost/asio/awaitable.hpp>
#include <utility>

namespace http
{

Router::Router(handler_t not_found_handler) : not_found_handler_(std::move(not_found_handler))
{
}

void Router::add_route(RouteKey key, handler_t handler)
{
    routes_.emplace(std::move(key), std::move(handler));
}

auto Router::dispatch(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>
{
    if (const auto it = routes_.find(std::pair{req->method(), req->target()}); it != routes_.end())
    {
        co_await it->second(req, res);
        co_return;
    }

    co_await not_found_handler_(req, res);
}

} // namespace http