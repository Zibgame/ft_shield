# ft_shield

> A lightweight Windows trojan dressed as a Microsoft Edge updater.
> Signature-clean. Memory-resident persistence. Zero visibility.

[![Build](https://img.shields.io/badge/build-passing-brightgreen)](#)
[![VirusTotal](https://img.shields.io/badge/VirusTotal-9%2F69%20ML--only-red)](#)
[![Signatures](https://img.shields.io/badge/signature%20detections-0-success)](#)
[![Language](https://img.shields.io/badge/language-C%20(Win32)-blue)](#)

---

## Overview

**ft_shield** is a remote administration tool implementing classic trojan
techniques against a Windows x64 target. Everything is built on raw Win32 —
no external libraries, no frameworks, no shortcuts.

The binary presents itself as a legitimate Microsoft Edge Update component:
authentic icon, version metadata, process naming, scheduled task naming.
Under the hood it establishes a persistent, silent C2 channel on port 4242.

```
 ┌─────────────┐      ┌──────────────────────────────────────────┐
 │  ft_shield  │─────▶│  C:\Windows\System32\ft_shield.exe       │
 │  (victim)   │      │  Scheduled task @ boot (HIGHEST rights)  │
 └─────────────┘      │  Daemon: mutex-locked, port 4242         │
                      └──────────────────┬───────────────────────┘
                                         │ select() loop
                              ┌──────────┼──────────┐
                              ▼          ▼          ▼
                          client 1   client 2   client 3
                          (SHA-256 auth → interactive shell)
```

## Capabilities

| Module | Detail |
|---|---|
| **Elevation** | Detects non-elevated context via `TokenElevation`, re-launches itself through `ShellExecuteEx` (`runas`) — native UAC prompt, refuses silently if denied |
| **Installation** | Self-copies to `System32`, registers `ONSTART` scheduled task with `HIGHEST` run level |
| **Daemon** | `CREATE_NO_WINDOW`, single-instance via named mutex, `select()`-based multi-client server |
| **Authentication** | SHA-256 through the Windows CNG API (`bcrypt.dll`). No plaintext secrets anywhere in the binary, wrong keycodes retried infinitely |
| **Shell** | Persistent `cmd.exe` child with anonymous pipes and marker-based output framing — no `system()`, survives client disconnects, clean re-entry after `exit` |
| **Multi-client** | 3 simultaneous authenticated sessions, 4th rejected with `Server full`; per-client isolated shell state |
| **Colors** | ANSI-colored client protocol (green success, red errors, yellow prompts, cyan shell) — works over plain `nc` |
| **Telemetry** | Hidden timestamped activity log: connections, auth attempts, shell commands, disconnects with I/O totals |
| **I/O accounting** | Per-session byte counters (in/out), queryable with `stats` |
| **Anti-analysis** | Triple debugger detection (`IsDebuggerPresent`, raw PEB flags, `CheckRemoteDebuggerPresent`) — silent exit on instrumentation |
| **Privilege kill** | `--kill` enables `SeDebugPrivilege` to terminate even SYSTEM-owned daemon instances |
| **Cleanup** | `--kill`: terminates all instances, removes persistence, releases the port |

## Stealth Layer

- **Identity**: authentic Edge icon, `Microsoft Corporation` version info, `MicrosoftEdgeUpdate*` naming across task/mutex/log artifacts
- **Visibility**: no console ever (GUI subsystem + `CREATE_NO_WINDOW`); attaches to the parent console only when launched from a terminal
- **Strings**: zero debug/introspection traces in the binary — no port reference, no auth hint, neutral symbol names
- **Anti-analysis**: silent debugger triage at startup (API + PEB + remote checks)
- **Packing**: UPX (LZMA, 58% ratio) or MPRESS (50.7%) via a single make target, including a custom PE32+ TLS-callback patcher (`pack/strip_tls.py`)
- **Detection profile**: 0 signature detections on VirusTotal — every hit is a machine-learning heuristic (`Wacatac.B!ml`), which flags compression entropy, not behavior

## Technical Highlights

- **No dynamic allocation anywhere** — everything lives on the stack or in static storage; zero leak surface, every Windows handle closed on every path
- **Zero information leakage** — `strings` on the binary reveals no port, no auth hint, no debug message; symbol names are neutral (`verify_access`, `check_context`)
- **Win32 only** — Winsock, BCrypt, Toolhelp, ShellExecute, schtasks: pure system API, nothing that does the work for you
- **Resilient protocol** — marker-framed shell output (`__FT_READY__`/`__FT_END__`) with overflow-safe streaming buffer; brutal client disconnects never crash the daemon
- **Clean process hygiene** — `cmd_stop` gracefully exits the shell child before terminating; double `shell` command never leaks a `cmd.exe`

## Architecture

```
ft_shield/
├── GNUmakefile              # Wine/Windows compatibility wrapper
├── Makefile                 # mingw-w64 cross-compilation rules
├── pack/
│   └── strip_tls.py         # PE32+ TLS callbacks nullifier (MPRESS prep)
├── include/
│   ├── ft_shield.h          # Global interface, password hash
│   ├── daemon.h             # Client state, colors, daemon interface
│   └── cmd.h                # Shell process interface
└── src/
    ├── main.c               # Entry: dispatch daemon / kill / install
    ├── version.rc           # Icon + version resources
    ├── edge.ico
    ├── Daemon/
    │   ├── daemon.c         # Daemon mode: mutex, persistence check, select loop
    │   ├── network.c        # Socket setup (WSA, bind, listen)
    │   └── client.c         # Accept / auth / shell / commands, I/O counters
    ├── Cmd/
    │   ├── cmd_init.c       # Shell state initialization
    │   ├── cmd_start.c      # Process creation, pipe wiring
    │   └── cmd_io.c         # Command dispatch, framed output streaming
    ├── Install/
    │   └── install.c        # UAC elevation, System32 copy, daemon spawn, login
    ├── Kill/
    │   └── kill.c           # SeDebugPrivilege, process sweep, task removal
    ├── Log/
    │   └── log.c            # Hidden append-only activity log
    ├── Other/
    │   ├── context.c        # Anti-debug triage (API / PEB / remote)
    │   ├── directory.c      # Self-path resolution
    │   └── is_elevated.c    # Privilege check
    ├── Persistence/
    │   └── task/
    │       └── task.c       # schtasks install / detect / uninstall
    └── encrypt/
        └── encryption.c     # SHA-256 (BCrypt) password verification
```

## Build

Requires `x86_64-w64-mingw32-gcc` (llvm-mingw supported):

```bash
make            # build ft_shield.exe
make clean      # remove objects
make fclean     # full clean
make re         # rebuild
make pack       # build + UPX (LZMA)
make mpack      # build + TLS strip + MPRESS
```

## Usage

```bash
./ft_shield.exe             # install (UAC elevation if required)
./ft_shield.exe --daemon    # daemon mode (spawned by the scheduled task)
./ft_shield.exe --kill      # remove persistence and kill every instance
```

First run as admin prints the user login, installs itself, and the daemon
takes over:

```
$ nc <vm-ip> 4242
Keycode: ********
Authentication success
$> ?
+--------------- COMMANDS ---------------+
| ?      Show available commands        |
| stats  Show session I/O statistics    |
| shell  Enter safe shell mode           |
+----------------------------------------+
$> shell
Shell started
shell> whoami
desktop-xxxxx\user
shell> exit
Leaving shell
$>
```

## Bonus Coverage

- [x] I/O quantification — per-session `stats`
- [x] Action logging — hidden timestamped audit trail
- [x] Trojan disguise — full Edge identity spoofing
- [x] Packing — UPX / MPRESS pipelines
- [x] Detection hardening — signature-clean, ML-only flags, anti-debug triage

## Notes

Built for the ft_shield 42 school project, on an isolated lab VM.
Educational purposes — understand the techniques to defend against them.
