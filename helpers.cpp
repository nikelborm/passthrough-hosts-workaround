#ifndef HELPERS_CPP
#define HELPERS_CPP

#include <array>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>

template <size_t N>
// Compile-Time String of fixed-capacity for constexpr concatenation
struct CTS {
    char buf[N] = {};

    constexpr CTS(const char (&s)[N]) {
        for (size_t i = 0; i < N; ++i) buf[i] = s[i];
    }

    template <size_t N1, size_t N2>
    constexpr CTS(const CTS<N1>& a, const CTS<N2>& b) {
        size_t idx = 0;
        for (size_t i = 0; i < N1 - 1; ++i) buf[idx++] = a.buf[i];
        for (size_t i = 0; i < N2; ++i) buf[idx++] = b.buf[i];
    }

    constexpr const char* data() const { return buf; }
    constexpr size_t size() const { return N - 1; }
};

template <size_t N1, size_t N2>
constexpr auto operator+(const CTS<N1>& a, const CTS<N2>& b) {
    return CTS<N1 + N2 - 1>(a, b);
}

// Flattened stream matcher
template <size_t N>
bool output_contains(const char* cmd, const std::array<char, N>& target_arr) {
    FILE* fp = popen(cmd, "r");
    if (!fp) return false;

    constexpr size_t target_len = N - 1;
    size_t match_count = 0;
    char ch;

    while ((ch = static_cast<char>(fgetc(fp))) != static_cast<char>(EOF)) {
        match_count = (ch == target_arr[match_count]) ? (match_count + 1) : (ch == target_arr[0]);
        if (match_count == target_len) {
            pclose(fp);
            return true;
        }
    }

    pclose(fp);
    return false;
}

// Helper to convert CTS to std::array for output_contains
template <size_t N>
constexpr auto to_array(const CTS<N>& cs) {
    std::array<char, N> arr{};
    for (size_t i = 0; i < N; ++i) arr[i] = cs.buf[i];
    return arr;
}

#endif // HELPERS_CPP
