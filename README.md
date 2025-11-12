# ICN.cpp, a port of ICN to C++

## How to use

1. Import the icn.hpp file along with Raylib ```cpp
#include "icn.hpp"
#include <raylib.h>
```

2. Setup the window ```cpp
int main() {
    InitWindow(800, 800, "ICN in C++ Example");
    Icn icn;
}
```

3. Render the ICN of your preference (add script after window setup) ```cpp
while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    icn.render(R"()", { 400, 400 }, 10); // Code, Position, Size
    EndDrawing();
}
```