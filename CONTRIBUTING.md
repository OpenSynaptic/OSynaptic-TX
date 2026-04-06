# Contributing to OSynaptic-TX

Thank you for your interest in contributing to OSynaptic-TX! This document provides guidelines and instructions for contributing to the project.

## Getting Started

### Prerequisites

- Knowledge of C89 programming language
- Arduino IDE 2.x or Arduino CLI (for library integration validation)
- Familiarity with embedded systems development
- avr-gcc or similar C89-capable cross-compiler for testing

### Development Environment Setup

1. **Clone the repository:**
   ```powershell
   git clone https://github.com/OpenSynaptic/OSynaptic-TX.git
   cd OSynaptic-TX
   ```

2. **Native CMake build (Windows/Linux/macOS):**
   ```powershell
   cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
   cmake --build build
   ```

3. **Validate Arduino example build:**
   ```bash
   arduino-cli compile --fqbn arduino:avr:uno examples/BasicTX
   ```

## Contribution Workflow

1. **Create a feature branch:**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/issue-number
   ```

2. **Implement changes:**
   - All source files live in `src/` (headers + `.c` files side-by-side)
   - Keep the C89 standard — no `//` comments, no `<stdint.h>`, no VLAs
   - No `malloc`/`free` — static or stack allocation only
   - Match existing code style (4-space indent, `ostx_` prefix for all symbols)

3. **Test locally:**
   - Build with `cmake` (all 8 targets must pass)
   - Check that `include/` headers still match `src/` copies
   - Verify Arduino example still compiles

4. **Open a pull request:**
   - Reference the related issue number if applicable
   - Describe what changed and why
   - Include before/after Flash/RAM measurements if the change affects size

## Code Style

- **C standard**: C89 (ANSI C) only — must compile with `-std=c89 -pedantic`
- **Types**: use `ostx_u8`, `ostx_u16`, `ostx_u32`, `ostx_i32` from `ostx_types.h`; never `int` for wire data
- **No global mutable state** (except optional static scratch in API A/B)
- **Header guards**: `#ifndef OSTX_FOO_H` pattern, no `#pragma once`
- **No C++ features** — headers must be includable from `.cpp` via `extern "C"` wrappers

## Reporting Issues

- Search existing issues before opening a new one
- Include board/toolchain version, MCU model, and minimal reproduction sketch
- For security vulnerabilities, see [SECURITY.md](SECURITY.md) — do **not** open a public issue

## License

By contributing, you agree that your contributions will be licensed under the Apache License 2.0.
