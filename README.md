# LittleShell

**LittleShell is a lightweight POSIX command execution engine written in C.**
It implements a minimal Unix-style runtime for parsing commands, dispatching built-ins, and launching Linux processes via `fork()` / `exec()` with deterministic, low-overhead behavior.

It is not a toy shell.
It is a small, readable process orchestration core.

---

## Why LittleShell Exists

Modern shells are dense, sprawling codebases.
LittleShell exists to expose the *actual machinery* behind Unix command execution — stripped of frameworks, scripting layers, and historical clutter.

It models the core Unix execution loop:

* Read → Parse → Dispatch → Execute → Synchronize
* Parent–child process coordination
* Deterministic termination handling
* Direct kernel interaction

Everything is explicit. Nothing is hidden.

---

## Architecture

LittleShell is built as a single execution loop with three subsystems:

• **Input Frontend** – Reads and tokenizes command streams
• **Dispatch Core** – Routes built-ins vs external executables
• **Execution Runtime** – Spawns, synchronizes, and monitors Unix processes

Process control is handled through `fork()`, `execvp()`, `waitpid()`, and Unix signal semantics — the same primitives used by real shells and runtimes.

---

## Features

* POSIX-style command execution
* Built-in commands (`cd`, `exit`, `help`)
* `fork()` / `exec()` process spawning
* `waitpid()` parent–child synchronization
* Unix signal-based termination handling
* Deterministic, low-overhead interactive loop

---

## Build & Run

```bash
gcc -O2 -Wall main.c -o littleshell
./littleshell
```

---

## Design Philosophy

LittleShell favors:

• Explicit control over abstraction
• Deterministic behavior over magic
• Readability over cleverness
• Kernel-level primitives over frameworks

It is intentionally small so that the execution model can be understood completely — not partially.

---

## Intended Use

LittleShell is ideal for:

• Learning Unix process internals
• Studying fork/exec semantics
• Building custom embedded runtimes
• Prototyping execution loops
• OS and systems experimentation
