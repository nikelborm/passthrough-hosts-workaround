#include <array>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>

template <size_t TSize>
// Compile-Time String of fixed-capacity for constexpr concatenation
struct CTS {
    char buf[TSize] = {};

    constexpr CTS(const char (&s)[TSize]) {
        for (size_t i = 0; i < TSize; ++i) buf[i] = s[i];
    }

    template <size_t ASize, size_t BSize>
    constexpr CTS(const CTS<ASize>& a, const CTS<BSize>& b) {
        size_t a_size_without_termination = ASize - 1;
        for (size_t i = 0; i < a_size_without_termination; ++i)
            buf[i] = a.buf[i];
        for (size_t i2 = 0; i2 < BSize; ++i2)
            buf[a_size_without_termination + i2] = b.buf[i2];
    }

    constexpr const char* data() const { return buf; }
    constexpr size_t size() const { return TSize - 1; }
};

template <size_t ASize, size_t BSize>
constexpr auto operator+(const CTS<ASize>& a, const CTS<BSize>& b) {
    return CTS<ASize + BSize - 1>(a, b);
}

// Flattened stream matcher
template <size_t TSize>
bool output_contains(const char* cmd, const std::array<char, TSize>& target_arr) {
    FILE* fp = popen(cmd, "r");
    if (!fp) return false;

    constexpr size_t target_len = TSize - 1;
    size_t match_count = 0;
    char ch;

    while ((ch = static_cast<char>(fgetc(fp))) != static_cast<char>(EOF)) {
        match_count = ch == target_arr[match_count]
            ? match_count + 1
            : ch == target_arr[0];
        if (match_count == target_len) {
            pclose(fp);
            return true;
        }
    }

    pclose(fp);
    return false;
}

template <size_t N_cmd, size_t N_target>
bool output_contains(const CTS<N_cmd>& cmd, const CTS<N_target>& target) {
    return output_contains(cmd.data(), to_array(target));
}

// Helper to convert CTS to std::array for output_contains
template <size_t N>
constexpr auto to_array(const CTS<N>& cs) {
    std::array<char, N> arr{};
    for (size_t i = 0; i < N; ++i) arr[i] = cs.buf[i];
    return arr;
}
