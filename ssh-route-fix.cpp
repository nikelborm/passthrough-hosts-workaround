#include <iostream>
#include <string>
#include <climits>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include "helpers.cpp"

constexpr CTS TARGET_IP_0 = "80.237.111.146";
// github
constexpr CTS TARGET_IP_1 = "140.82.121.3";
constexpr CTS TARGET_IP_2 = "140.82.121.6";
constexpr CTS TARGET_IP_3 = "140.82.121.4";
constexpr CTS TARGET_IP_4 = "140.82.114.22";
constexpr CTS TARGET_IP_5 = "185.199.110.215";

constexpr CTS GATEWAY_IP = "192.168.0.1";
constexpr CTS INTERFACE  = "wlp1s0";

constexpr auto SEARCH_RULE_TARGET  = CTS("lookup 100");
constexpr auto SEARCH_ROUTE_TARGET = GATEWAY_IP;

// Installs (or with `restore`, removes) the route isolation for one target IP.
// All commands are built at compile time, so the target is a template parameter.
template <auto TARGET_IP>
bool process_target(bool restore, const auto& log) {
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

    bool rule_exists  = output_contains(CHECK_RULE, SEARCH_RULE_TARGET);
    bool route_exists = output_contains(CHECK_ROUTE, SEARCH_ROUTE_TARGET);

    if (restore) {
        if (!rule_exists && !route_exists) {
            log(CTS("[=] Nothing to restore for ") + TARGET_IP + CTS(".\n"));
            return true;
        }

        if (rule_exists) {
            log(CTS("[-] Removing ip rule (priority 10) for ") + TARGET_IP + CTS("...\n"));
            if (system(RESTORE_RULE_CMD.data()) != 0) return false;
        }

        if (route_exists) {
            log(CTS("[-] Removing route in table 100 for ") + TARGET_IP + CTS("...\n"));
            if (system(RESTORE_ROUTE_CMD.data()) != 0) return false;
        }

        log(CTS("[✓] Route isolation removed for ") + TARGET_IP + CTS(".\n"));
        return true;
    }

    if (rule_exists && route_exists) {
        log(CTS("[=] Route and ip rule for ") + TARGET_IP + CTS(" already exist. Doing nothing.\n"));
        return true;
    }

    if (!route_exists) {
        log(CTS("[+] Adding route in table 100 for ") + TARGET_IP + CTS("...\n"));
        if (system(INSTALL_ROUTE_CMD.data()) != 0) return false;
    }

    if (!rule_exists) {
        log(CTS("[+] Adding ip rule (priority 10) for ") + TARGET_IP + CTS("...\n"));
        if (system(INSTALL_RULE_CMD.data()) != 0) return false;
    }

    log(CTS("[✓] Route isolation verified and active for ") + TARGET_IP + CTS(".\n"));
    return true;
}

constexpr auto USAGE =
    CTS("Usage: ssh-route-fix [OPTIONS]\n\n")
    + CTS("Installs route isolation for ")
    + TARGET_IP_0
    + CTS(" and ")
    + TARGET_IP_1
    + CTS(" via gateway ")
    + GATEWAY_IP
    + CTS(" on interface ")
    + INTERFACE
    + CTS(".\n\n")
    + CTS("Options:\n")
    + CTS("  --silent             Suppress output messages\n")
    + CTS("  --restore            Remove the route and ip rule instead of installing them\n")
    + CTS("  --print-completion   Print the bash completion script and exit\n")
    + CTS("  --help               Show this help message and exit\n");

constexpr auto COMPLETION_SCRIPT =
    CTS("# bash completions for ssh-route-fix\n")
    + CTS("_ssh_route_fix() {\n")
    + CTS("    local cur opts\n")
    + CTS("    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n")
    + CTS("    opts='--silent --restore --print-completion --help -h'\n")
    + CTS("    if [[ \"$cur\" == -* ]]; then\n")
    + CTS("        COMPREPLY=( $(compgen -W \"$opts\" -- \"$cur\") )\n")
    + CTS("    fi\n")
    + CTS("    return 0\n")
    + CTS("}\n")
    + CTS("complete -F _ssh_route_fix ssh-route-fix\n");

void print_usage() {
    std::cout << USAGE.data();
}

void print_completion() {
    std::cout << COMPLETION_SCRIPT.data();
}

bool validate_installation() {
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len <= 0) {
        std::cerr << "Error: cannot resolve own binary path via /proc/self/exe: "
                  << std::strerror(errno) << "\n";
        return false;
    }
    exe_path[len] = '\0';

    struct stat st{};
    if (stat(exe_path, &st) != 0) {
        std::cerr << "Error: cannot stat own binary '" << exe_path << "': "
                  << std::strerror(errno) << "\n";
        return false;
    }

    constexpr mode_t EXPECTED_MODE = S_ISUID | 0755;
    const mode_t actual_mode = st.st_mode & 07777;

    std::string problems;

    if (st.st_uid != 0) {
        struct passwd* pw = getpwuid(st.st_uid);
        problems += "  - owner is uid " + std::to_string(st.st_uid);
        if (pw) problems += std::string(" (") + pw->pw_name + ")";
        problems += ", expected root (uid 0)\n";
    }

    if (st.st_gid != 0) {
        struct group* gr = getgrgid(st.st_gid);
        problems += "  - group is gid " + std::to_string(st.st_gid);
        if (gr) problems += std::string(" (") + gr->gr_name + ")";
        problems += ", expected root (gid 0)\n";
    }

    if (actual_mode != EXPECTED_MODE) {
        char octal[8];
        std::snprintf(octal, sizeof(octal), "%04o", actual_mode);
        problems += std::string("  - permissions are 0") + octal
                  + ", expected 04755";
        if (!(actual_mode & S_ISUID)) problems += " (the setuid flag is missing)";
        problems += "\n";
    }

    if (!problems.empty()) {
        std::cerr << "Error: " << exe_path << " is not installed correctly:\n"
                  << problems
                  << "Reinstall it properly with 'make install'.\n";
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--print-completion") {
            print_completion();
            return 0;
        }
    }

    if (!validate_installation()) {
        return 1;
    }

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

    bool ok = process_target<TARGET_IP_0>(restore, log);
    ok = process_target<TARGET_IP_1>(restore, log) && ok;
    ok = process_target<TARGET_IP_2>(restore, log) && ok;
    ok = process_target<TARGET_IP_3>(restore, log) && ok;
    ok = process_target<TARGET_IP_4>(restore, log) && ok;
    ok = process_target<TARGET_IP_5>(restore, log) && ok;

    return ok ? 0 : 1;
}
