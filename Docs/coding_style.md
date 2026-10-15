# Coding Style

## Naming

- Public functions: snake_case
- Private functions: camelCase
- Constants: kCamelCase
- Types: PascalCase
- Variables: camelCase

## File Organization

1. File Header
2. Includes
3. Private Constants
4. Private Types
5. Private Variables
6. Private Function Prototypes
7. Private Functions
8. Public Functions

## Formatting

- Opening braces stay on the same line.
- Four-space indentation.
- Early return preferred.

## Automated formatting

Firmware C++ files use the repository-root `.clang-format` configuration. Run `clang-format -i` on
`.cpp` and `.h` files. For the Arduino sketch, pass `--assume-filename=main.cpp` when formatting
`Firmware/main/main.ino`.