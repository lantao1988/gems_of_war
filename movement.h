#pragma once
#include <utility>
#include <cassert>

enum GemType {
    GT_UNKNOWN = 0,
    GT_EMPTY,
    // Treasure map
    GT_COPPER,
    GT_SILVER,
    GT_GOLD,
    GT_PURSE,
    GT_BOX,
    GT_GREENBOX,
    GT_REDBOX,
    GT_DOOR,

    // Normal map
    GT_RED,
    GT_GREEN,
    GT_BROWN,
    GT_PURPLE,
    GT_BLUE,
    GT_YELLOW,
    GT_SKELETON
};

enum MoveType {
    MT_UP = 0,
    MT_DOWN,
    MT_LEFT,
    MT_RIGHT
};

typedef std::pair<int, int> Position;
static const Position INVALID_POS = std::make_pair(-1, -1);

typedef std::pair<Position, MoveType> Movement;