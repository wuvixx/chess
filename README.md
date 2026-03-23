# ♟️ Cross-Platform C++ Chess (PvP)

A robust, cross-platform 2D Chess engine built with **C++**. This project features a complete implementation of standard Chess rules for local multiplayer (Player vs. Player) gameplay, featuring a dynamic main menu and a unified CMake-based build system.

<p align="center">
  <img width="600" height="628" alt="Screenshot_20260323_155204" src="https://github.com/user-attachments/assets/c418bd67-98a1-411b-9bb2-0ffec85e05e4" /><br />
  *A look at the board during an active session.*
</p>

## 🚀 Key Features
* **Full Chess Logic:** Complete implementation of piece movement, turn-based mechanics, and board state validation.
* **Dynamic Main Menu:** Features an automated background "attract mode" with randomized piece movement.
* **Local Multiplayer:** Optimized for 1v1 play on a single keyboard/mouse setup.
* **Cross-Platform:** Fully compatible with both **Windows** (MSVC/Clang-cl) and **Linux** (GCC/Clang).

## 🛠️ Tech Stack
* **Language:** C++
* **Build System:** CMake (Version 3.10+)
* **Graphics:** [Insert your library here, e.g., SFML, SDL2, or Raylib]
* **Compiler Support:** * **Windows:** MSVC / Clang-cl
    * **Linux:** GCC / Clang

## 📂 Project Structure
```text
├── src/                # Source code (.cpp files)
├── include/            # Source code (.h files)
├── assets/             # Textures, fonts, and icons
├── CMakeLists.txt      # Cross-platform build configuration
└── README.md           # Documentation
```
## 🔨 How to Build
This project uses **CMake** to ensure a consistent build process across different operating systems.

### 1. Prerequisites
Ensure you have a C++ compiler (MSVC, Clang, or GCC) and CMake installed on your system.

### 2. Build Instructions
Open your terminal or command prompt in the project root and run:

```bash
# Create a build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build the executable
cmake --build .
```
The executable will be generated in the `build/` directory under the name `Game`.

## 🧠 Development Philosophy
I developed this project to master state machine logic and cross-platform build systems. By moving from platform-specific scripts to a unified CMake configuration, I ensured the codebase remains portable and maintainable. The UI is intentionally kept minimal to focus on core engine stability and reliable move-validation logic.
