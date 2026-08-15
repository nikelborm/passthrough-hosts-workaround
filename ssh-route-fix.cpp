#include <iostream>
#include <string>
#include "helpers.cpp"

constexpr CTS TARGET_IP  = "80.237.111.146";
constexpr CTS GATEWAY_IP = "192.168.0.1";
constexpr CTS INTERFACE  = "wlp1s0";

constexpr auto CHECK_RULE  =
    CTS("ip rule show to ")
    + TARGET_IP;
constexpr auto CHECK_ROUTE =
    CTS("ip route show table 100 to ")
    + TARGET_IP;
constexpr auto INSTALL_ROUTE_CMD =
    CTS("ip route add ")
    + TARGET_IP
    + CTS(" via ")
    + GATEWAY_IP
    + CTS(" dev ")
    + INTERFACE
    + CTS(" table 100");
constexpr auto INSTALL_RULE_CMD =
    CTS("ip rule add to ")
    + TARGET_IP
    + CTS(" table 100 priority 10");
constexpr auto RESTORE_ROUTE_CMD =
    CTS("ip route del table 100 to ")
    + TARGET_IP;
constexpr auto RESTORE_RULE_CMD =
    CTS("ip rule del to ")
    + TARGET_IP
    + CTS(" table 100 priority 10");

constexpr auto SEARCH_RULE_TARGET  = CTS("lookup 100");
constexpr auto SEARCH_ROUTE_TARGET = GATEWAY_IP;

int main(int argc, char* argv[]) {
    if (setuid(0) != 0) {
        perror("setuid failed");
        return 1;
    }

    bool silent = false;
    bool restore = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--silent") silent = true;
        else if (arg == "--restore") restore = true;
        else {
            std::cerr << "Unknown flag: " << arg << "\n";
            return 1;
        }
    }

    auto log = [silent](auto const& msg) {
        if (!silent) std::cout << msg;
    };

    bool rule_exists  = output_contains(CHECK_RULE, SEARCH_RULE_TARGET);
    bool route_exists = output_contains(CHECK_ROUTE, SEARCH_ROUTE_TARGET);

    if (restore) {
        if (!rule_exists && !route_exists) {
            log("[=] Nothing to restore.\n");
            return 0;
        }

        if (rule_exists) {
            log("[-] Removing ip rule (priority 10)...\n");
            if (system(RESTORE_RULE_CMD.data()) != 0) return 1;
        }

        if (route_exists) {
            log("[-] Removing route in table 100...\n");
            if (system(RESTORE_ROUTE_CMD.data()) != 0) return 1;
        }

        log("[✓] Route isolation removed.\n");
        return 0;
    }

    if (rule_exists && route_exists) {
        log("[=] Route and ip rule already exist. Doing nothing.\n");
        return 0;
    }

    if (!route_exists) {
        log("[+] Adding route in table 100...\n");
        if (system(INSTALL_ROUTE_CMD.data()) != 0) return 1;
    }

    if (!rule_exists) {
        log("[+] Adding ip rule (priority 10)...\n");
        if (system(INSTALL_RULE_CMD.data()) != 0) return 1;
    }

    log("[✓] Route isolation verified and active.\n");
    return 0;
}
