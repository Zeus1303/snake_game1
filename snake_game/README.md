# Snake Game 🐍

## Yêu cầu
- CMake >= 3.16
- Compiler hỗ trợ C++17 (GCC 10+, Clang 10+, MSVC 2019+)
- **SFML 3.x**
- File `arial.ttf` (xem hướng dẫn bên dưới)

---

## Cài đặt SFML 3

### Ubuntu / Debian
```bash
sudo apt install libsfml-dev
```
> Nếu repo chỉ có SFML 2, cài từ source: https://github.com/SFML/SFML

### macOS (Homebrew)
```bash
brew install sfml
```

### Windows (vcpkg)
```bash
vcpkg install sfml:x64-windows
```

---

## Font arial.ttf

**Linux:**
```bash
# Dùng font Liberation thay thế
cp /usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf arial.ttf
```

**macOS:**
```bash
cp /Library/Fonts/Arial.ttf .
```

**Windows:** Copy `C:\Windows\Fonts\arial.ttf` vào thư mục này.

---

## Build & Chạy

```bash
mkdir build && cd build
cmake ..
cmake --build .

# Linux/macOS
./SnakeGame

# Windows
.\SnakeGame.exe
```

### Windows + vcpkg
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

---

## Điều khiển
| Phím | Hành động |
|------|-----------|
| ↑ ↓ ← → | Di chuyển |
| Click chuột | Tương tác menu |
