# Contributing to MYUNIXLIKEOS

Thank you for your interest in contributing! This project is a 32-bit Unix-like OS built with a focus on clean attribution and open-source growth under the GPLv3.

## Legal & Attribution (Required)
To maintain the integrity of the project and comply with our licensing, we require strict attribution:
- **Full Legal Name:** Every commit that changes code or translations **must** include the author's full legal name (in Latin letters; diacritics are allowed).
- **GitHub Profile:** It is highly recommended that your "Name" field in your GitHub profile matches the legal name used in your commits.
- **Licensing:** By contributing, you agree that your code will be licensed under the **GNU General Public License v3**.

## Technical Standards
- **Target Architecture:** 32-bit (i686-elf).
- **Toolchain:** All code must be compatible with `i686-elf-gcc`.
- **Data Types:** We recommend (but do not strictly require) using fixed-width types like `uint64_t`, `uint32_t`, and `uint8_t` from `<stdint.h>` rather than `unsigned long` or `int` for hardware-sensitive logic.
- **Indentation:** You may use either **Tabs** or **4 Spaces** for indentation. We do not enforce a strict rule, provided the code remains readable.

## 🚀 How to Contribute

### 1. Development Process
1. Fork the repository.
2. Ensure your build environment uses the `i686-elf` cross-compiler.
3. Verify your changes using the local `build_boot.sh` script and QEMU.

### 2. Pull Requests
- Ensure your commit history is clean.
- Double-check that your commit metadata contains your **Full Legal Name**.
- If adding a new feature (like a new Unix command), please update the `help` command logic to include it.

### 3. Areas Needing Help
- **Keyboard Driver:** Implementing the state-machine for Shift, Control, and all symbol keys.
- **Unix Commands:** Expanding the CLI (e.g., `rm`, `mv`, `alias`).
- **Networking:** Implementing 32-bit drivers for common network cards.

## 🏗️ Commit Message Example
`feat: add uppercase support to keyboard driver - John Doe`

---
*Building a legacy, one system call at a time.*
