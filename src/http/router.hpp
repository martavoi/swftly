#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>
#include <boost/container_hash/hash.hpp>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace http
{

namespace beast = boost::beast;
namespace http = beast::http;

using request_t = http::request<http::string_body>;
using response_t = http::response<http::string_body>;
using handler_t = std::function<boost::asio::awaitable<void>(const request_t *, response_t *)>;

/**
 * @brief A composite key used for routing lookups in the router's map.
 *
 * This struct owns the target path string to ensure its lifetime persists
 * as long as the route exists in the router. It is constructed from a
 * string_view to allow for flexibility at the call site.
 */
struct RouteKey
{
    http::verb method_;
    std::string target_; // The router must own the string.

    /**
     * @brief Constructs a RouteKey for storage in the router.
     *
     * This constructor is marked explicit to prevent accidental or inefficient conversions.
     * @param method The HTTP method (verb) for the route.
     * @param target The request path. The view's data will be copied into an owned string.
     */
    explicit RouteKey(http::verb method, std::string_view target) : method_(method), target_(target)
    {
    }
};

/**
 * @brief A high-performance HTTP request router.
 *
 * This class maps incoming requests based on their method and path to specific
 * handler functions. It uses C++20's transparent hashing and equality features
 * to perform lookups without allocating a new string for each request, which
 * significantly improves performance.
 */
class Router
{
  public:
    /**
     * @brief Constructs the router.
     * @param not_found_handler A handler to be invoked when no route matches a request.
     */
    explicit Router(handler_t not_found_handler);

    /**
     * @brief Adds a new route to the router's dispatch table.
     * @param key The RouteKey specifying the method and path.
     * @param handler The function to execute when the route is matched.
     */
    void add_route(RouteKey key, handler_t handler);

    /**
     * @brief Dispatches a request to the appropriate handler.
     *
     * The handler will directly modify the response object provided.
     * If no route is found, the not_found_handler will be invoked.
     * @param req The incoming HTTP request.
     * @param res The response object to be populated by the handler.
     */
    auto dispatch(const request_t *req, response_t *res) const -> boost::asio::awaitable<void>;

  private:
    // A transparent hasher that can hash both RouteKey and temporary lookup pairs
    // without allocating strings.
    struct RouteKeyHash
    {
        using is_transparent = void;

        // Hash for stored RouteKey
        auto operator()(const RouteKey &k) const noexcept -> std::size_t
        {
            return boost::hash_value(std::tie(k.method_, k.target_));
        }

        // Hash for temporary lookup pair (avoids string allocation)
        auto operator()(const std::pair<http::verb, std::string_view> &k) const noexcept -> std::size_t
        {
            return boost::hash_value(k);
        }
    };

    // A transparent equality comparator that can compare both RouteKey and lookup pairs
    struct RouteKeyEqual
    {
        using is_transparent = void;

        // Compare two RouteKeys
        auto operator()(const RouteKey &lhs, const RouteKey &rhs) const noexcept -> bool
        {
            return lhs.method_ == rhs.method_ && lhs.target_ == rhs.target_;
        }

        // Compare lookup pair with RouteKey
        auto operator()(const std::pair<http::verb, std::string_view> &lhs, const RouteKey &rhs) const noexcept -> bool
        {
            return lhs.first == rhs.method_ && lhs.second == rhs.target_;
        }

        // Compare RouteKey with lookup pair (for symmetry)
        auto operator()(const RouteKey &lhs, const std::pair<http::verb, std::string_view> &rhs) const noexcept -> bool
        {
            return lhs.method_ == rhs.first && lhs.target_ == rhs.second;
        }
    };

    std::unordered_map<RouteKey, handler_t, RouteKeyHash, RouteKeyEqual> routes_;
    handler_t not_found_handler_;
};

} // namespace http