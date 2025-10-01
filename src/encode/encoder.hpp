#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace encode
{

/**
 * @brief Error codes for Base62 encoding/decoding operations.
 */
enum class EncoderError
{
    InvalidCharacter, ///< Input contains non-Base62 characters
    EmptyInput,       ///< Input string is empty
    Overflow          ///< Decoded value would exceed uint64_t range
};

/**
 * @brief Base62 encoder/decoder for URL shortening.
 *
 * Uses characters [a-zA-Z0-9] which are URL-safe and case-sensitive.
 * This gives us 62^n possible combinations for n-character strings.
 *
 * Design considerations:
 * - Counter-based encoding ensures no collisions
 * - Optimized for performance with lookup tables
 * - Maximum 5 characters for 255M URLs (100k/day for 7 years)
 * - Thread-safe (stateless operations)
 */
class Encoder
{
  public:
    /**
     * @brief Constructs a Base62 encoder with default character set.
     */
    Encoder() = default;

    /**
     * @brief Encodes a number to Base62 string.
     *
     * @param n The number to encode
     * @return Base62 encoded string (1-5 characters)
     */
    [[nodiscard]] auto encode(std::uint64_t n) const -> std::string;

    /**
     * @brief Decodes a Base62 string back to a number.
     *
     * @param short_code The Base62 encoded string
     * @return The decoded number or an error code
     */
    [[nodiscard]] auto decode(std::string_view short_code) const -> std::expected<std::uint64_t, EncoderError>;

    /**
     * @brief Get the character set used for encoding.
     *
     * @return The Base62 character set
     */
    [[nodiscard]] auto get_charset() const
    {
        return kCharset;
    }

    /**
     * @brief Get the base value (62).
     *
     * @return The base value
     */
    [[nodiscard]] auto get_base() const
    {
        return kBase;
    }

    /**
     * @brief Calculate maximum capacity for given string length.
     *
     * @param length The string length
     * @return Maximum number of unique IDs for that length
     */
    [[nodiscard]] auto calculate_capacity(std::size_t length) const -> std::uint64_t;

  private:
    // The character set for Base62 encoding
    static constexpr std::string_view kCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static constexpr std::size_t kBase = kCharset.size();

    // Compile-time lookup table for O(1) character decoding
    static constexpr std::uint16_t kAsciiTableSize = 256;
    static constexpr std::uint8_t kMaxAsciiValue = 127;
    static constexpr std::uint8_t kInvalidValue = 255;

    // Buffer size for our use case: max 5 chars for 255M URLs
    static constexpr std::size_t kMaxDigits = 5;

    /**
     * @brief Creates the decode lookup table at compile time.
     *
     * @return Lookup table for character-to-value mapping
     */
    static constexpr auto make_decode_table() -> std::array<std::uint8_t, kAsciiTableSize>;

    // Static lookup table for decoding
    static const std::array<std::uint8_t, kAsciiTableSize> kDecodeTable;
};

} // namespace encode
