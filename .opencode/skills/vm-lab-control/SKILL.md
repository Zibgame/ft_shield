---
name: vm-lab-control
description: Inspect and operate the authorized local or remote lab VM from OpenCode, including process management, service control, command execution, and verification. Use when the user asks to act inside their VM; do not use it to bypass authentication or operate an unconfirmed target.
compatibility: opencode
metadata:
  opencode/slash: "true"
---

# VM lab control

Operate the user's authorized VM through the shell and management interfaces already available to OpenCode. A skill supplies instructions; it does not grant extra permissions or credentials.

## Determine where commands run

Before changing anything, establish the execution context with minimal read-only checks such as `pwd`, `id`, `hostname`, `uname -a`, and `ps`.

- If OpenCode is running inside the requested VM, use the local shell directly.
- If the target is another VM, use an existing authorized channel such as SSH, WinRM, a configured guest agent, or the hypervisor CLI.
- Do not scan networks, guess credentials, bypass authentication, or use a backdoor, reverse shell, malware C2, or the `ft_shield` listener as an administration channel.
- If several machines could match, ask the user to identify the target before making changes.

## Execute deliberately

Use the smallest command that completes the requested action. Prefer non-interactive commands whose exit status and output can be checked. Preserve the current working directory when it matters, and quote paths and arguments safely.

For a long-running or interactive command, keep the returned terminal session and send input to that same session. Do not start duplicate sessions merely because a command is still running.

Never expose passwords, tokens, private keys, environment secrets, or full credential-bearing command lines. Reuse already configured SSH agents, SSH config, or platform credential stores.

## Manage a process

For requests such as “kill that process”:

1. Identify the exact process with read-only evidence, for example `pgrep -af -- <name>` or `ps -o pid,ppid,user,stat,etime,cmd -p <pid>`.
2. Reject ambiguous name matches. Ask for the intended PID or disambiguating detail when choosing incorrectly could stop unrelated work.
3. Check ownership and whether the process is a service, container, or supervised child. Prefer its native stop mechanism (`systemctl stop`, container stop, or supervisor command) when applicable.
4. Send `SIGTERM` to the exact PID and wait briefly while polling that PID.
5. Use `SIGKILL` only if the same process is still alive, graceful shutdown failed, and force is within the user's request. Re-check the PID identity immediately before escalation to avoid PID-reuse mistakes.
6. Verify the final state and report what stopped. Do not use broad patterns such as `pkill -f` or `killall` unless the user explicitly wants every verified match stopped.

Do not terminate critical host, hypervisor, SSH, OpenCode, or session-parent processes unless the user explicitly names the consequence and confirms the exact target.

## Manage the VM lifecycle

Inspect state before lifecycle operations. Resolve the VM by an exact name or UUID. Prefer graceful guest shutdown or reboot; use forced power-off/reset only when explicitly requested or when a failed graceful attempt has been reported and the user wants escalation.

Do not delete a VM, disk, snapshot, network, or configuration as an implied part of “stop”, “restart”, or “clean”.

## Verify and report

After every mutation, run a proportionate check: process absence, service state, command exit status, file checksum, port ownership, or VM state. Report the target, action, result, and any remaining uncertainty. Never claim success from command submission alone.
