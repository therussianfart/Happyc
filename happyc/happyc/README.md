# HappyC

A compiled language with a simple syntax, written in C++.

## Requirements

- CMake 3.10+
- A C++17 compiler (MSVC, GCC, or Clang)
- g++ on PATH (for compiling .hc files)

## Compiling the compiler

```
cmake -S . -B build
cmake --build build
```

## Using the compiler

```
hc <file.hc> <output.exe>
```

Example:

```
hc example.hc example.exe
```

Then run the output:

```
example.exe
```

## Language Basics

| Feature | Syntax |
|---|---|
| Variable | `let x = 1;` |
| User input | `let x = User_input();` |
| Function | `func main() { }` |
| Function call | `greet();` |
| Return value | `return "hello";` |
| Condition | `if x = 1 { }` |
| Print (no newline) | `print("hello");` |
| Print (with newline) | `println("hello");` |
| Concatenation | `println("hi " + name);` |

Types are inferred automatically. Functions can be defined in any order.

## Example

```hc
func main() {
    greet();
}

func greet() {
    print("Enter your name: ");
    let name = User_input();
    println("Hello " + name);
}
```
