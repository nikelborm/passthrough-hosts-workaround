#include <iostream>
#include <string_view>
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

constexpr CTS TARGET_IP  = "80.237.111.146";
constexpr CTS GATEWAY_IP = "192.168.0.1";
constexpr CTS INTERFACE  = "wlp1s0";

constexpr auto CHECK_RULE  = CTS("ip rule show to ") + TARGET_IP;
constexpr auto CHECK_ROUTE = CTS("ip route show table 100 host ") + TARGET_IP;
constexpr auto CMD_ROUTE   = CTS("ip route add ")
    + TARGET_IP
    + CTS(" via ")
    + GATEWAY_IP
    + CTS(" dev ")
    + INTERFACE
    + CTS(" table 100");
constexpr auto CMD_RULE =
    CTS("ip rule add to ")
    + TARGET_IP
    + CTS(" table 100 priority 10");

constexpr auto SEARCH_RULE_TARGET  = CTS("table 100");
constexpr auto SEARCH_ROUTE_TARGET = GATEWAY_IP;

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

int main() {
    if (setuid(0) != 0) {
        perror("setuid failed");
        return 1;
    }

    bool rule_exists  = output_contains(CHECK_RULE.data(), to_array(SEARCH_RULE_TARGET));
    bool route_exists = output_contains(CHECK_ROUTE.data(), to_array(SEARCH_ROUTE_TARGET));

    if (rule_exists && route_exists) {
        std::cout << "[=] Route and ip rule for " << TARGET_IP.data() << " already exist. Doing nothing.\n";
        return 0;
    }

    if (!route_exists) {
        std::cout << "[+] Adding route in table 100...\n";
        if (system(CMD_ROUTE.data()) != 0) return 1;
    }

    if (!rule_exists) {
        std::cout << "[+] Adding ip rule (priority 10)...\n";
        if (system(CMD_RULE.data()) != 0) return 1;
    }

    std::cout << "[✓] Route isolation verified and active.\n";
    return 0;
}
