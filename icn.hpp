#include <raylib.h>
#include <raymath.h>
#include <string>
#include <iostream>
#include <sstream>

inline int getCmdLength(std::string cmd) {
    if (cmd == "w" || cmd == "c") return 1;
    if (cmd == "square" || cmd == "rect" || cmd == "line") return 4;
    if (cmd == "dot" || cmd == "cont" || cmd == "move") return 2;
    if (cmd == "cutcircle" || cmd == "ellipse") return 5;
    if (cmd == "tri") return 6;
    return 0;
}

inline Color hexToColor(const std::string& hex, unsigned char alpha = 255) {
    if (hex.empty()) return WHITE;

    std::string cleanHex = (hex[0] == '#') ? hex.substr(1) : hex;

    if (cleanHex.size() == 3) {
        std::string expanded;
        expanded.reserve(6);
        for (char c : cleanHex) {
            expanded.push_back(c);
            expanded.push_back(c);
        }
        cleanHex = expanded;
    }

    if (cleanHex.size() != 6) {
        std::cout << "Icn: Invalid color '" << hex << "'\n";
        return WHITE;
    }

    unsigned int hexValue;
    std::stringstream ss;
    ss << std::hex << cleanHex;
    ss >> hexValue;

    unsigned char r = (hexValue >> 16) & 0xFF;
    unsigned char g = (hexValue >> 8) & 0xFF;
    unsigned char b = hexValue & 0xFF;

    return Color{ r, g, b, alpha };
}

inline float safeStof(const std::string& in) {
    try { return std::stof(in); }
    catch (...) {
        std::cout << "Icn: Invalid number '" << in << "'\n";
        return 0.0f;
    }
}

inline std::vector<std::pair<std::string, std::vector<std::string>>> parseCode(const std::string& code) {
    std::vector<std::pair<std::string, std::vector<std::string>>> returned;

    std::vector<std::string> tokens;
    std::string current;

    for (char c : code) {
        if (c == ' ' || c == '\n' || c == '\t') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);

    for (size_t i = 0; i < tokens.size();) {
        std::string cmd = tokens[i];
        int argCount = getCmdLength(cmd);

        std::vector<std::string> args;
        for (int j = 1; j <= argCount && (i + j) < tokens.size(); j++) {
            args.push_back(tokens[i + j]);
        }

        returned.push_back({cmd, args});
        i += 1 + argCount;
    }

    return returned;
}

inline void DrawRoundedLine(Vector2 pos1, Vector2 pos2, float width, Color color) {
    DrawLineEx(pos1, pos2, width, color);
    DrawCircle(pos1.x, pos1.y, width / 2, color);
    DrawCircle(pos2.x, pos2.y, width / 2, color);
}

inline void DrawRectangleOutline(float x, float y, float w, float h, int thickness, Color color) {
    DrawRoundedLine({ x - (w / 2), y - (h / 2) }, { x + (w / 2), y - (h / 2) }, thickness, color);
    DrawRoundedLine({ x + (w / 2), y - (h / 2) }, { x + (w / 2), y + (h / 2) }, thickness, color);
    DrawRoundedLine({ x + (w / 2), y + (h / 2) }, { x - (w / 2), y + (h / 2) }, thickness, color);
    DrawRoundedLine({ x - (w / 2), y + (h / 2) }, { x - (w / 2), y - (h / 2) }, thickness, color);
}

inline void DrawCustomEllipse(int cx, int cy, float rx, float ry, float rotDeg, float thickness, int segments, Color color) {
    float rotation = rotDeg * DEG2RAD;
    float step = 2 * PI / segments;

    float cr = cosf(rotation);
    float sr = sinf(rotation);

    Vector2 first = {0};
    Vector2 prev  = {0};

    for (int i = 0; i <= segments; i++) {
        float a = step * i;

        float x = cosf(a) * rx;
        float y = sinf(a) * ry;

        Vector2 p = {
            cx + x * cr - y * sr,
            cy + x * sr + y * cr
        };

        if (i == 0)
            first = p;
        else
            DrawLineEx(prev, p, thickness, color);

        prev = p;
    }

    DrawLineEx(prev, first, thickness, color);
}

inline bool checkDirs(Vector2 p1, Vector2 p2, Vector2 p3) {
    return ((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x)) < 0;
}

inline void renderIcn(const std::vector<std::pair<std::string, std::vector<std::string>>>& data, int origX, int origY, float size) {
    int offX = origX;
    int offY = origY;
    Color curColor = WHITE;
    float width = 2.5 * size;
    float xSave = 0;
    float ySave = 0;
    for (const auto& line : data) {
        if (line.second.empty()) { 
            std::cout << "Icn: Empty command\n";
            continue;
        }
        std::string cmd = line.first;
        std::vector<std::string> args = line.second;

        if (cmd == "c") {
            curColor = hexToColor(args[0]);
        } else if (cmd == "w") {
            width = safeStof(args[0]) * size;
        } else if (cmd == "square") {
            float x = safeStof(args[0]) * size;
            float y = 0 - safeStof(args[1]) * size;
            float w = safeStof(args[2]) * (size * 2);
            float h = safeStof(args[3]) * (size * 2);

            xSave = x + (w / 2);
            ySave = y + (h / 2);
            DrawRectangleOutline(x + offX, y + offY, w, h, width, curColor);
        } else if (cmd == "tri") {
            Vector2 p1 = { safeStof(args[0]) * size + offX, 0 - safeStof(args[1]) * size + offY };
            Vector2 p2 = { safeStof(args[2]) * size + offX, 0 - safeStof(args[3]) * size + offY };
            Vector2 p3 = { safeStof(args[4]) * size + offX, 0 - safeStof(args[5]) * size + offY };

            xSave = p3.x;
            ySave = p3.y;
            bool flip = checkDirs(p1, p2, p3);
            DrawTriangle(p1, flip ? p2 : p3, flip ? p3 : p2, curColor);
        } else if (cmd == "rect") {
            float x = safeStof(args[0]) * size;
            float y = 0 - safeStof(args[1]) * size;
            float w = safeStof(args[2]) * (size * 2);
            float h = safeStof(args[3]) * (size * 2);

            xSave = x + (w / 2);
            ySave = y + (h / 2);
            DrawRectangle(x + offX, y + offY, w, h, curColor);
        } else if (cmd == "line") {
            float x = safeStof(args[0]) * (size);
            float y = 0 - safeStof(args[1]) * (size);
            float x2 = safeStof(args[2]) * (size);
            float y2 = 0 - safeStof(args[3]) * (size);
            xSave = x2;
            ySave = y2;
            DrawRoundedLine({static_cast<float>(x + offX), static_cast<float>(y + offY)}, {static_cast<float>(x2 + offX), static_cast<float>(y2 + offY)}, static_cast<float>(width), curColor);
        } else if (cmd == "cont") {
            float x = safeStof(args[0]) * size;
            float y = 0 - safeStof(args[1]) * size;
            DrawRoundedLine({static_cast<float>(xSave + offX), static_cast<float>(ySave + offY)}, {static_cast<float>(x + offX), static_cast<float>(y + offY)}, static_cast<float>(width), curColor);
            xSave = x;
            ySave = y;
        } else if (cmd == "dot") {
            float x = safeStof(args[0]) * (size);
            float y = 0 - safeStof(args[1]) * (size);
            DrawCircle(x + offX, y + offY, (width) / 2, curColor);
            xSave = x;
            ySave = y;
        } else if (cmd == "cutcircle") {
            float x = safeStof(args[0]) * (size * 1);
            float y = 0 - safeStof(args[1]) * (size * 1);
            float radius = (safeStof(args[2]) * size) / 1;
            float angleICN = safeStof(args[3]);
            float filledICN = safeStof(args[4]);

            float circleAngle = ((angleICN * 10) - filledICN);
            float oldX = x + sinf(circleAngle * DEG2RAD) * radius;
            float oldY = y - cosf(circleAngle * DEG2RAD) * radius;

            int steps = static_cast<int>((filledICN / 3) + 1);
            for (int i = 0; i < steps - 1; i++) {
                circleAngle += 6.0f;
                float newX = x + sinf(circleAngle * DEG2RAD) * radius;
                float newY = y - cosf(circleAngle * DEG2RAD) * radius;
                DrawRoundedLine({ oldX + offX, oldY + offY }, { newX + offX, newY + offY }, width * 1, curColor);
                oldX = newX;
                oldY = newY;
            }
        } else if (cmd == "ellipse") {
            float x = safeStof(args[0]) * (size * 1);
            float y = 0 - safeStof(args[1]) * (size * 1);
            float w = (safeStof(args[2]) * size) / 1;
            float h = w * safeStof(args[3]);
            float d = safeStof(args[4]);
            DrawCustomEllipse(x + offX, y + offY, w, h, d, width, 256, curColor);
        } else if (cmd == "move") {
            float x = safeStof(args[0]) * (size * 1);
            float y = 0 - safeStof(args[1]) * (size * 1);
            offX += x;
            offY += y;
        } else if (cmd == "back") {
            offX = origX;
            offY = origY;
        } else {
            std::cout << "Icn: Unknown command: '" << cmd << "'" << std::endl;
        }
    }
}

class Icn {
public:
    void render(std::string code, Vector2 pos = { 0, 0 }, float size = 10) {
        renderIcn(parseCode(code), pos.x, pos.y, size);
    }
    std::vector<std::pair<std::string, std::vector<std::string>>> parse(std::string code) {
        return parseCode(code);
    }
private:
};
