#include <iostream>
#include "helpers.cpp"

constexpr CTS TARGET_IP  = "80.237.111.146";
constexpr CTS GATEWAY_IP = "192.168.0.1";
constexpr CTS INTERFACE  = "wlp1s0";

constexpr auto CHECK_RULE  = CTS("ip rule show to ") + TARGET_IP;
constexpr auto CHECK_ROUTE = CTS("ip route show table 100 to ") + TARGET_IP;
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

constexpr auto SEARCH_RULE_TARGET  = CTS("lookup 100");
constexpr auto SEARCH_ROUTE_TARGET = GATEWAY_IP;

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
