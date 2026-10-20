# ft_shield


## Architecture

```
ft_shield/
├── Makefile
├── README.md
├── en.subject.pdf
├── include/
│   ├── ft_shield.h
│   ├── daemon.h
│   ├── network.h
│   ├── auth.h
│   ├── shell.h
│   ├── persistence.h
│   └── stealth.h
└── src/
    ├── main.c
    ├── daemon.c
    ├── network.c
    ├── auth.c
    ├── shell.c
    ├── persistence.c
    └── stealth.c
```

### Modules

| File | Responsibility |
|---|---|
| `main.c` | Entry point. Checks root rights, prints login, spawns daemon. |
| `daemon.c` | Forks into background, ensures single instance via PID file or mutex. |
| `network.c` | Creates TCP socket on port 4242, accepts up to 3 simultaneous clients (select/poll/epoll). |
| `auth.c` | Handles password authentication. Password stored as a SHA-256 hash, never in clear. |
| `shell.c` | Spawns an interactive root shell (execve /bin/sh, not `system()`). |
| `persistence.c` | Copies binary to `/bin/ft_shield`, registers startup service (init.d script or crontab). |
| `stealth.c` | Suppresses error output, hides process from logs, sets process name. |

## Build

```bash
make            # Build ft_shield
make clean      # Remove object files
make fclean     # Remove binary and object files
make re         # Rebuild from scratch
```

## Usage

```bash
sudo ./ft_shield   # Launch as root
```

After first run, the daemon persists in `/bin/ft_shield` and starts on boot.

```bash
nc localhost 4242   # Connect to the daemon
```

## Requirements

- Root privileges required
- Port 4242 must be available
- Linux (Debian-based recommended for stability)
- C compiler (gcc/clang)

## Mandatory Features

- [x] Shows user login on stdout when run
- [x] Creates `/bin/ft_shield` copy
- [x] Daemonizes as background process
- [x] Starts on machine boot
- [x] Single instance enforcement
- [x] Listens on port 4242
- [x] Accepts 3 simultaneous clients
- [x] Password authentication (hashed, not in clear)
- [x] Root shell spawning on authenticated connection
- [x] No error messages during install or use

## Bonus

- [ ] Quantify I/O data
- [ ] Log user actions (keylogger)
- [ ] Disguise trojan binary
- [ ] Packing/optimization for detectability
