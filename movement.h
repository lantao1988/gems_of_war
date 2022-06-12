#pragma once
#include <utility>
#include <vector>
#include <cassert>
#include <cstdint>

enum GemType {
    GT_UNKNOWN = -1,
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
typedef std::vector<std::vector<GemType>> GameMap;

enum MoveState {
    MS_INVALID = 0,
    MS_OK,
    MS_FOUR,
    MS_EXTERTURN  
};

struct MoveResult {
    MoveState state;
    std::vector<std::pair<GameMap, std::vector<Position>>> snapshots;
};

struct PredictResult {
    Movement movement;
    MoveResult moveResult;
    GameMap map;
    std::vector<uint64_t> c33;
    std::vector<uint64_t> c44;
};

struct MoveOptions {
    bool showState = false;
};

struct PredictOptions {
    bool needC33 = true;
    bool needC44 = false;
};
