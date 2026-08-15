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

### Automatic Integration via .ssh/config

You can integrate the binary directly into your `~/.ssh/config` so it runs
automatically before every connection to a specific host.

Add the following to your `~/.ssh/config`:

```ssh
Host target-host
    # Expects it to be locatable with PATH; in my case ~/.local/bin is in PATH
    ProxyCommand /bin/sh -c "ssh-route-fix --silent && exec nc %h %p"
```
