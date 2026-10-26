<div align="center">

# ft_shield

### A Win32 remote-administration study built from first principles

_Native Windows APIs · Persistent daemon · Authenticated multi-client shell_

[![Build](https://img.shields.io/badge/build-passing-2ea44f?style=for-the-badge)](#build)
[![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078d4?style=for-the-badge&logo=windows11&logoColor=white)](#requirements)
[![Language](https://img.shields.io/badge/language-C%20%7C%20Win32-555555?style=for-the-badge&logo=c&logoColor=white)](#technical-highlights)
[![VirusTotal](https://img.shields.io/badge/VirusTotal-9%2F69%20ML--only-d73a49?style=for-the-badge)](#stealth-layer)

</div>

> [!CAUTION]
> This project demonstrates trojan techniques for the **42 `ft_shield` curriculum**. Run it only inside an isolated lab VM that you own and control. Never deploy it on third-party systems or networks.

---

## Contents

- [Overview](#overview)
- [How it works](#how-it-works)
- [Capabilities](#capabilities)
- [Stealth layer](#stealth-layer)
- [Technical highlights](#technical-highlights)
- [Project structure](#project-structure)
- [Build](#build)
- [Usage](#usage)
- [Bonus coverage](#bonus-coverage)
- [Verified behaviour](#verified-behaviour)

## Overview

**ft_shield** is a lightweight remote-administration tool that explores classic trojan techniques against a Windows x64 target. It is implemented entirely with raw Win32 APIs—without external libraries or frameworks.

The binary presents itself as a Microsoft Edge Update component through its icon, version metadata, process name, and scheduled-task name. Underneath that identity, it maintains an authenticated, multi-client command channel.

| Target | Transport | Concurrency | Authentication | Runtime |
|:--:|:--:|:--:|:--:|:--:|
| Windows x64 | TCP · port `4242` | 3 clients | SHA-256 · CNG | Native Win32 |

## How it works

[![Architecture diagram of zibgame/ft_shield](https://gitdiagram.com/zibgame/ft_shield/diagram.png)](https://gitdiagram.com/zibgame/ft_shield?utm_source=readme&utm_medium=picture)

Each accepted client passes through authentication before reaching the command interface. Shell sessions use a persistent `cmd.exe` child connected through anonymous pipes and marker-based output framing.

## Capabilities

| Module | Implementation |
|:--|:--|
| **Elevation** | Checks `TokenElevation`, prints the login, then relaunches through `ShellExecuteEx` with `runas`; exits silently if UAC is refused |
| **Installation** | Copies itself to `System32` and registers an `ONSTART` scheduled task running as `SYSTEM` with the `HIGHEST` run level |
| **Daemon** | Uses `CREATE_NO_WINDOW`, a named mutex, and a `select()`-based server loop |
| **Authentication** | Computes SHA-256 through Windows CNG (`bcrypt.dll`); no plaintext secret is stored in the binary |
| **Shell** | Maintains a `cmd.exe` child with anonymous pipes and framed output; a dedicated thread streams each command's output so the network loop never blocks |
| **Multi-client** | Provides 3 isolated authenticated sessions; a fourth connection receives `Server full` |
| **Protocol UI** | Emits ANSI colors for status, errors, prompts, and shell output |
| **Telemetry** | Records timestamped connections, authentication attempts, commands, disconnects, and I/O totals |
| **I/O accounting** | Tracks bytes sent and received per session and exposes them through `stats` |
| **Anti-analysis** | Combines `IsDebuggerPresent`, raw PEB flags, and `CheckRemoteDebuggerPresent` |
| **Cleanup** | `--kill` enables `SeDebugPrivilege`, stops running instances, removes persistence, and releases the port |

## Stealth layer

| Surface | Strategy |
|:--|:--|
| **Identity** | Edge icon, `Microsoft Corporation` version information, and `MicrosoftEdgeUpdate*` artifact names |
| **Visibility** | GUI subsystem and `CREATE_NO_WINDOW`; attaches to a parent console only when launched from a terminal |
| **Strings** | Removes port hints, authentication hints, debug text, and descriptive internal symbol names |
| **Anti-analysis** | Runs three debugger checks during startup and exits silently when instrumentation is detected |
| **Packing** | Uses Obsidian-Packer with LZMA compression, ChaCha20 encryption, and in-memory PE restoration |
| **Detection profile** | Reports no signature detections; observed hits are machine-learning heuristics such as `Wacatac.B!ml` |

## Technical highlights

- **Static memory model** — no dynamic allocation; state lives on the stack or in static storage.
- **Handle hygiene** — Windows handles are closed along every error and shutdown path.
- **Pure Win32 implementation** — Winsock, BCrypt, Toolhelp, ShellExecute, and `schtasks` do the platform work.
- **Framed streaming** — `__FT_READY__` and `__FT_END__` delimit shell output without relying on `system()`.
- **Disconnect resilience** — interrupted clients do not terminate the daemon or leak child processes.
- **Clean shell lifecycle** — `cmd_stop` gracefully closes the child; repeated `shell` commands do not leak `cmd.exe` instances.

## Project structure

<details>
<summary><strong>Expand the source tree</strong></summary>

```text
ft_shield/
├── GNUmakefile              # Wine/Windows compatibility wrapper
├── Makefile                 # mingw-w64 cross-compilation rules
├── include/
│   ├── ft_shield.h          # Global interface and password hash
│   ├── daemon.h             # Client state and daemon interface
│   └── cmd.h                # Shell-process interface
└── src/
    ├── main.c               # Install, daemon, and cleanup dispatch
    ├── version.rc           # Windows version resources
    ├── edge.ico             # Executable icon
    ├── Daemon/
    │   ├── daemon.c         # Mutex, persistence check, and select loop
    │   ├── network.c        # Winsock setup, bind, and listen
    │   └── client.c         # Authentication, protocol, and I/O counters
    ├── Cmd/
    │   ├── cmd_init.c       # Shell-state initialization
    │   ├── cmd_start.c      # Process creation and pipe wiring
    │   └── cmd_io.c         # Command dispatch and output framing
    ├── Install/
    │   └── install.c        # UAC, System32 copy, and daemon spawn
    ├── Kill/
    │   └── kill.c           # Privilege enablement and cleanup
    ├── Log/
    │   └── log.c            # Hidden append-only activity log
    ├── Other/
    │   ├── context.c        # Debugger checks
    │   ├── directory.c      # Executable-path resolution
    │   └── is_elevated.c    # Privilege-state check
    ├── Persistence/task/
    │   └── task.c           # Scheduled-task lifecycle
    └── encrypt/
        └── encryption.c     # SHA-256 verification through BCrypt
```

</details>

## Build

### Requirements

- `make`
- `x86_64-w64-mingw32-gcc` or a compatible LLVM-MinGW toolchain
- `git` and `objcopy` to install [Obsidian-Packer](https://github.com/Zibgame/Obsidian-Packer) for packed targets

```bash
make
```

| Command | Result |
|:--|:--|
| `make` | Builds `ft_shield.exe` |
| `make clean` | Removes intermediate objects |
| `make fclean` | Removes objects and the executable |
| `make re` | Performs a clean rebuild |
| `make install-packer` | Clones and builds Obsidian-Packer in `tools/` |
| `make pack` | Builds and packs with Obsidian-Packer |

## Usage

> [!IMPORTANT]
> Use the following workflow only on the isolated Windows VM dedicated to the project.

```bash
./ft_shield.exe             # Install; requests UAC elevation if needed
./ft_shield.exe --daemon    # Start daemon mode
./ft_shield.exe --kill      # Stop instances and remove persistence
```

After installation, the lab client connects to the VM on port `4242`:

```console
$ nc <vm-ip> 4242
Keycode: ********
Authentication success
$> ?
+--------------- COMMANDS ---------------+
| ?      Show available commands         |
| stats  Show session I/O statistics     |
| shell  Enter shell mode                |
+----------------------------------------+
$> shell
Shell started
shell> whoami
nt authority\system
shell> exit
Leaving shell
$>
```

## Bonus coverage

- [x] Per-session I/O quantification through `stats`
- [x] Timestamped action logging
- [x] Full executable identity and metadata customization
- [x] Obsidian-Packer pipeline with encrypted in-memory restoration
- [x] Multi-layer debugger detection

## Verified behaviour

All of the following was validated end-to-end on the target Windows VM:

- Daemon auto-start after a full reboot, running as `nt authority\system`
- Shell output streamed by worker threads: concurrent clients answered in under 10 ms during long commands (`ping -n 5`)
- Brutal client disconnects mid-command: daemon survives and accepts new sessions
- Three simultaneous authenticated clients; the fourth receives `Server full`
- Self-refresh on relaunch: the `System32` copy is updated after killing the previous daemon
- No visible error under any usage path, including missing-runtime DLL scenarios

---

<div align="center">

Built for the **42 `ft_shield` project** and intended exclusively for controlled, educational lab use.

</div>
