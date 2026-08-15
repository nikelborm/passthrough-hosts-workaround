# Passthrough traffic of ip

## Build

```sh
make
```

## Install

Puts it into `~/.local/bin` and gives it proper suid flags and priveleges.

```sh
make install
```

## Usage

The binary is designed to be set up with the SUID bit to allow execution without `sudo`.

`TARGET_IP`, `GATEWAY_IP`, `INTERFACE` are hardcoded values into the
`ssh-route-fix.cpp`, because I have only one host that needs this and run this
only on my laptop with the specific network interface name.

### Automatic Integration via .ssh/config

You can integrate the binary directly into your `~/.ssh/config` so it runs
automatically before any connection to a specific host.

Add the following to your `~/.ssh/config`:

```ssh
Host target-host
    # Expects it to be locatable with PATH; in my case ~/.local/bin is in PATH
    ProxyCommand /bin/sh -c "ssh-route-fix --silent && exec nc %h %p"
```

## Lisence

MIT-0
