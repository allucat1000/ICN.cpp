#include "icn.hpp"
#include <raylib.h>

int main() {
    InitWindow(800, 800, "ICN in C++ Example");
    Icn icn;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        icn.render(R"(w 2 line -12.1 -5.4 2.2 -7.6 line -12.1 10.3 2.2 12.5 line -2.7 7.3 6.8 8.1 line -2.7 -2.4 6.8 -3.2 line -12.1 -5.4 -12.1 10.3 line 2.2 -7.6 2.2 12.5 line -2.7 -2.4 -2.7 7.3 line 6.8 -3.2 6.8 8.1 line -12.1 -5.4 -2.7 -2.4 line 2.2 -7.6 6.8 -3.2 line 2.2 12.5 6.8 8.1 line -12.1 10.3 -2.7 7.3)", { 400, 400 },10);
        EndDrawing();
    }
}