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
    // A private hasher that knows how to hash both our real key and a temporary
    // key made of non-owning views.
    struct RouteKeyHash
    {
        // This tag enables C++20's high-performance lookup feature.
        using is_transparent = void;

        auto operator()(const RouteKey &k) const noexcept -> std::size_t
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, k.method_);
            boost::hash_combine(seed, k.target_);
            return seed;
        }

        // This overload allows hashing a key without creating a std::string.
        auto operator()(const std::pair<http::verb, std::string_view> &k) const noexcept -> std::size_t
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, k.first);
            boost::hash_combine(seed, k.second);
            return seed;
        }
    };

    // A private equality checker that can compare our real key against a temporary one.
    struct RouteKeyEqual
    {
        using is_transparent = void;

        // Compares a stored key with a temporary lookup key.
        auto operator()(const RouteKey &lhs, const std::pair<http::verb, std::string_view> &rhs) const noexcept -> bool
        {
            return lhs.method_ == rhs.first && lhs.target_ == rhs.second;
        }

        // Compares two stored keys with each other.
        auto operator()(const RouteKey &lhs, const RouteKey &rhs) const noexcept -> bool
        {
            return lhs.method_ == rhs.method_ && lhs.target_ == rhs.target_;
        }
    };

    std::unordered_map<RouteKey, handler_t, RouteKeyHash, RouteKeyEqual> routes_;
    handler_t not_found_handler_;
};

} // namespace http