#include "encoder.hpp"
#include <array>
#include <string_view>

namespace encode
{

// Static member initialization
const std::array<std::uint8_t, Encoder::kAsciiTableSize> Encoder::kDecodeTable = Encoder::make_decode_table();

constexpr auto Encoder::make_decode_table() -> std::array<std::uint8_t, kAsciiTableSize>
{
    std::array<std::uint8_t, kAsciiTableSize> table{};
    table.fill(kInvalidValue);

    // Set valid Base62 characters to their numeric values
    for (std::uint8_t i = 0; i < kBase; ++i)
    {
        const unsigned char ch = kCharset[i];
        table.at(ch) = i;
    }

    return table;
}

auto Encoder::encode(std::uint64_t n) const -> std::string
{
    if (n == 0) [[unlikely]]
    {
        return {1, kCharset[0]};
    }

    // Buffer size for our use case: max 5 chars for 255M URLs
    constexpr std::size_t kMaxDigits = 5;
    std::array<char, kMaxDigits> buffer{};

    // Fill buffer from right to left (avoids reverse)
    auto pos = buffer.end();
    while (n > 0)
    {
        *--pos = kCharset[n % kBase];
        n /= kBase;
    }

    // Return string from first written position to end
    return {pos, buffer.end()};
}

auto Encoder::decode(std::string_view short_code) const -> std::expected<std::uint64_t, EncoderError>
{
    if (short_code.empty()) [[unlikely]]
    {
        return std::unexpected(EncoderError::EmptyInput);
    }

    std::uint64_t result = 0;
    for (char c : short_code)
    {
        // Check for non-ASCII characters (Unicode chars > 127)
        if (static_cast<unsigned char>(c) > kMaxAsciiValue) [[unlikely]]
        {
            return std::unexpected(EncoderError::InvalidCharacter);
        }

        const auto value = kDecodeTable.at(static_cast<std::uint8_t>(c));
        if (value == kInvalidValue) [[unlikely]]
        {
            return std::unexpected(EncoderError::InvalidCharacter);
        }

        // Check for overflow before multiplication
        if (result > (UINT64_MAX - value) / kBase) [[unlikely]]
        {
            return std::unexpected(EncoderError::Overflow);
        }

        result = result * kBase + value;
    }

    return result;
}

auto Encoder::calculate_capacity(std::size_t length) const -> std::uint64_t
{
    if (length == 0)
    {
        return 0;
    }

    // Calculate base^length, but check for overflow
    std::uint64_t result = 1;
    for (std::size_t i = 0; i < length; ++i)
    {
        if (result > UINT64_MAX / kBase)
        {
            return UINT64_MAX; // Overflow, return max value
        }
        result *= kBase;
    }

    return result;
}

auto to_string(EncoderError error) -> std::string_view
{
    switch (error)
    {
    case EncoderError::InvalidCharacter:
        return "Invalid character in Base62 string";
    case EncoderError::EmptyInput:
        return "Empty input string";
    case EncoderError::Overflow:
        return "Decoded value exceeds maximum range";
    }
    return "Unknown error";
}

} // namespace encode