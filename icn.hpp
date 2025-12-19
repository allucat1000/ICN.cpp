#include <raylib.h>
#include <raymath.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <utility>
#include <variant>

using Arg = std::variant<float, std::string>;

enum class Op {
    Color, Width, Square, Rect, Line, Cont, Dot,
    Cutcircle, Ellipse, Move, Back, Tri, Curve, Unknown
};

struct cmd {
    Op op;
    std::vector<Arg> args;
};

enum class ArgType { Float, String };

using Signature = std::vector<ArgType>;

inline const std::unordered_map<Op, Signature> signatures = {
    { Op::Width, { ArgType::Float } },
    { Op::Color, { ArgType::String } },
    { Op::Square, { ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float }},
    { Op::Rect, { ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float }},
    { Op::Move, { ArgType::Float, ArgType::Float } },
    { Op::Line, { ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float } },
    { Op::Cont, { ArgType::Float, ArgType::Float } },
    { Op::Dot, { ArgType::Float, ArgType::Float } },
    { Op::Ellipse, { ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float } },
    { Op::Cutcircle, { ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float } },
    { Op::Tri, { ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float } },
    { Op::Curve, { ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float, ArgType::Float } },
    { Op::Back, {}},
};

using Cmd = cmd;
using CmdList = std::vector<Cmd>;


inline std::unordered_map<std::string, CmdList> cache{};

inline Op parseOp(std::string_view s) {
    if (s == "w") return Op::Width;
    if (s == "c") return Op::Color;
    if (s == "square") return Op::Square;
    if (s == "rect") return Op::Rect;
    if (s == "line") return Op::Line;
    if (s == "dot") return Op::Dot;
    if (s == "cont") return Op::Cont;
    if (s == "move") return Op::Move;
    if (s == "cutcircle") return Op::Cutcircle;
    if (s == "ellipse") return Op::Ellipse;
    if (s == "tri") return Op::Tri;
    if (s == "curve") return Op::Curve;

    return Op::Unknown;
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
        return BLACK;
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

inline CmdList parseCode(const std::string& code) {
    CmdList returned;

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
        std::string cmdStr = tokens[i];

        Op op = parseOp(cmdStr);

        auto it = signatures.find(op);
        if (it == signatures.end()) {
            std::cout << "Icn: Unknown command '" << cmdStr << "'\n";
            i += 1;
            continue;
        }
        const auto& sig = it->second;

        size_t expected = sig.size();

        if (i + expected >= tokens.size()) {
            std::cout << "Icn: Not enough arguments for '" << cmdStr << "'\n";
            i += 1;
            continue;
        }

        std::vector<Arg> args;
        args.reserve(sig.size());

        for (size_t a = 0; a < sig.size(); ++a) {
            const std::string& tok = tokens[i + 1 + a];

            switch (sig[a]) {
                case ArgType::Float:
                    args.emplace_back(safeStof(tok));
                    break;
                case ArgType::String:
                    args.emplace_back(tok);
                    break;
            }
        }

        returned.push_back(cmd{ op, std::move(args) });
        i += 1 + expected;
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

    for (int i = 0; i < segments; i++) {
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

inline void drawRoundedSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c, Vector2 p2, int thickness, Color color) {
    DrawSplineSegmentBezierQuadratic(p1, c, p2, thickness, color);
    DrawCircle(p1.x, p1.y, thickness / 2, color);
    DrawCircle(p2.x, p2.y, thickness / 2, color);
}

inline void renderIcn(const CmdList& data, int origX, int origY, float size) {
    int offX = origX;
    int offY = origY;
    Color curColor = WHITE;
    float width = 2.5 * size;
    float xSave = 0;
    float ySave = 0;
    for (const auto& line : data) {
        const cmd& c = line;
        const auto& args = line.args;

        switch (c.op) {
            case Op::Color: {
                curColor = hexToColor(std::get<std::string>(args[0]));
                break;
            }
            case Op::Width: {
                width = std::get<float>(args[0]) * size;
                break;
            }
            case Op::Square: {
                float x = std::get<float>(args[0]) * size;
                float y = 0 - std::get<float>(args[1]) * size;
                float w = std::get<float>(args[2]) * (size * 2);
                float h = std::get<float>(args[3]) * (size * 2);

                xSave = x + (w / 2);
                ySave = y + (h / 2);
                DrawRectangleOutline(x + offX, y + offY, w, h, width, curColor);
                break;
            }
            case Op::Tri: {
                Vector2 p1 = { std::get<float>(args[0]) * size + offX, 0 - std::get<float>(args[1]) * size + offY };
                Vector2 p2 = { std::get<float>(args[2]) * size + offX, 0 - std::get<float>(args[3]) * size + offY };
                Vector2 p3 = { std::get<float>(args[4]) * size + offX, 0 - std::get<float>(args[5]) * size + offY };

                xSave = p3.x;
                ySave = p3.y;
                bool flip = checkDirs(p1, p2, p3);
                DrawTriangle(p1, flip ? p2 : p3, flip ? p3 : p2, curColor);
                break;
            }
            case Op::Rect: {
                float x = std::get<float>(args[0]) * size;
                float y = 0 - std::get<float>(args[1]) * size;
                float w = std::get<float>(args[2]) * (size * 2);
                float h = std::get<float>(args[3]) * (size * 2);

                xSave = x + (w / 2);
                ySave = y + (h / 2);
                DrawRectangle(x + offX, y + offY, w, h, curColor);
                break;
            }
            case Op::Line: {
                float x = std::get<float>(args[0]) * (size);
                float y = 0 - std::get<float>(args[1]) * (size);
                float x2 = std::get<float>(args[2]) * (size);
                float y2 = 0 - std::get<float>(args[3]) * (size);
                xSave = x2;
                ySave = y2;
                DrawRoundedLine({static_cast<float>(x + offX), static_cast<float>(y + offY)}, {static_cast<float>(x2 + offX), static_cast<float>(y2 + offY)}, static_cast<float>(width), curColor);
                break;
            }
            case Op::Cont: {
                float x = std::get<float>(args[0]) * size;
                float y = 0 - std::get<float>(args[1]) * size;
                DrawRoundedLine({static_cast<float>(xSave + offX), static_cast<float>(ySave + offY)}, {static_cast<float>(x + offX), static_cast<float>(y + offY)}, static_cast<float>(width), curColor);
                xSave = x;
                ySave = y;
                break;
            }
            case Op::Dot: {
                float x = std::get<float>(args[0]) * (size);
                float y = 0 - std::get<float>(args[1]) * (size);
                DrawCircle(x + offX, y + offY, (width) / 2, curColor);
                xSave = x;
                ySave = y;
                break;
            }
            case Op::Cutcircle: {
                float x = std::get<float>(args[0]) * (size);
                float y = 0 - std::get<float>(args[1]) * (size);
                float radius = (std::get<float>(args[2]) * size);
                float angleICN = std::get<float>(args[3]);
                float filledICN = std::get<float>(args[4]);

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
                break;
            }
            case Op::Ellipse: {
                float x = std::get<float>(args[0]) * (size);
                float y = 0 - std::get<float>(args[1]) * (size);
                float w = (std::get<float>(args[2]) * size);
                float h = w * std::get<float>(args[3]);
                float d = std::get<float>(args[4]);
                DrawCustomEllipse(x + offX, y + offY, w, h, d, width, 256, curColor);
                break;
            }
            case Op::Move: {
                float x = std::get<float>(args[0]) * (size);
                float y = 0 - std::get<float>(args[1]) * (size);
                offX += x;
                offY += y;
                break;
            }
            case Op::Back: {
                offX = origX;
                offY = origY;
                break;
            }
            case Op::Curve: {
                Vector2 p1 = {
                    std::get<float>(args[0]) * size + offX,
                    -std::get<float>(args[1]) * size + offY
                };

                Vector2 control = {
                    std::get<float>(args[4]) * size + offX,
                    -std::get<float>(args[5]) * size + offY
                };

                Vector2 p2 = {
                    std::get<float>(args[2]) * size + offX,
                    -std::get<float>(args[3]) * size + offY
                };

                xSave = p2.x;
                ySave = p2.y;

                drawRoundedSplineSegmentBezierQuadratic(p1, control, p2, width, curColor);
                break;
            }
            case Op::Unknown: {
                std::cout << "Icn: Unknown command" << std::endl;
                break;
            }
        }
    }
}

class Icn {
public:
    void render(std::string code, Vector2 pos = { 0, 0 }, float size = 10) {
        renderIcn(parseC(code), pos.x, pos.y, size);
    }
    CmdList parse(std::string code) {
        return parseC(code);
    }
private:
    std::unordered_map<std::string, CmdList> cache;

    const CmdList& parseC(const std::string& code) {
        auto it = cache.find(code);
        if (it != cache.end())
            return it->second;

        CmdList parsed = parseCode(code);
        auto [inserted, _] = cache.emplace(code, std::move(parsed));
        return inserted->second;
    }
};
