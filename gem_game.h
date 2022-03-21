#pragma once
#include <vector>
#include "movement.h"

enum MoveState {
    MS_INVALID = 0,
    MS_OK,
    MS_EXTERTURN  
};


class Game {
public:
    Game(bool treasure = false, int rowMax = 9, int colMax = 8);
    Game(const std::vector<std::vector<int>> &map, 
         const std::vector<int> &c = std::vector<int>(), 
         bool treasure = false);
    ~Game();
public:
    MoveState Move(const Movement &movement);
    void Show() const;
private:
    bool ValidMove(const Movement &movement);

    std::vector<std::vector<Position>> GemCollection() const;
    void FallDown();

    void CreateNewGems(const std::vector<std::vector<Position>> &gems, 
                        const std::vector<Position> &movePos);
    GemType NewGem() const;

    GemType &GetGem(const Position &pos);
    const GemType &GetGem(const Position &pos) const;

    Position NewPos(const Position &pos, MoveType moveType) const;

    std::vector<Position> AllSameCollection(const Position &pos, MoveType type) const;
    bool ValidPosition(const Position &pos) const;

private:
    friend class GameTest_testGemCollection_Test;
private:
    bool treasure_;
    const int row_max_;
    const int col_max_;

    std::vector<std::vector<GemType>> map_;
    std::vector<GemType> collections_;
};

