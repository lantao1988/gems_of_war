#pragma once
#include <vector>
#include "movement.h"

class Game {
public:
    Game(bool treasure = false, int rowMax = 9, int colMax = 8, int seed = 1);
    Game(const std::vector<std::vector<int>> &map, 
         const std::vector<int> &c = std::vector<int>(), 
         bool treasure = false);
    ~Game();
public:
    MoveResult Move(const Movement &movement, bool show);
    void Show() const;
    std::vector<PredictResult> AllValidMove();

    const std::vector<std::vector<GemType>> &CurrentState() const {
        return map_;
    }
    Position NewPos(const Position &pos, MoveType moveType) const;
public:
    std::vector<std::vector<Position>> GemCollection() const;
    void FallDown(bool show, std::vector<Position> &changed);
    void CreateNewGems(const std::vector<std::vector<Position>> &gems, 
                        const std::vector<Position> &movePos);
private:
    bool ValidMove(const Movement &movement, bool doMove, std::vector<std::vector<Position>> &gems);

    GemType NewGem() const;

    GemType &GetGem(const Position &pos);
    const GemType &GetGem(const Position &pos) const;

    std::vector<Position> AllSameCollection(const Position &pos, MoveType type) const;
    bool ValidPosition(const Position &pos) const;

private:
    friend class GameTest_testGemCollection_Test;
    friend class GameTest_testMove_Test;
private:
    bool treasure_;
    const int row_max_;
    const int col_max_;

    std::vector<std::vector<GemType>> map_;
    std::vector<GemType> collections_;
};
