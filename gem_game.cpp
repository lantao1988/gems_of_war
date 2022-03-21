#include "gem_game.h"
#include <cstdlib>
#include <map>
#include <set>
#include <ctime>
#include <iostream>
using namespace std;

Game::Game(bool treasure, int rowMax, int colMax)
    : row_max_(rowMax)
    , col_max_(colMax)
{
    if (treasure) {
        collections_ = {GT_COPPER, GT_SILVER, GT_GOLD, GT_PURSE};
    } else {
        collections_ = {GT_RED, GT_GREEN, GT_BROWN, GT_PURPLE, GT_BLUE, GT_YELLOW, GT_SKELETON};
    }
    srand(time(NULL));
    map_.resize(rowMax);
    for (int i = 0; i < map_.size(); ++i) {
        map_[i].resize(colMax);
        for (int j = 0; j < map_[i].size(); ++j) {
            map_[i][j] = NewGem();
        }
    }
    /*
    bool useless;
    while (NextState(useless)) {}
    (void)useless;
    */
}

Game::Game(const vector<vector<int>> &map, const vector<int> &collections, bool treasure)
    : treasure_(treasure)
    , row_max_(map.size())
    , col_max_(map[0].size())
{
    map_.resize(row_max_);
    for (int i = 0; i < map_.size(); ++i) {
        map_[i].resize(col_max_);
        for (int j = 0; j < map_[i].size(); ++j) {
            map_[i][j] = (GemType)map[i][j];
        }
    }
    for (auto gem : collections) {
        collections_.push_back((GemType)gem);
    }
}


Game::~Game() {
}

void Game::CreateNewGems(const vector<vector<Position>> &gems, 
                         const vector<Position> &movePos)
{
    for (auto gem : gems) {
        Position targetPos = INVALID_POS;
        if (treasure_) {
            if (movePos.size() > 0) {
                assert(movePos.size() == 2);
                for (auto pos : gem) {
                    if (pos == movePos[0] || pos == movePos[1]) {
                        targetPos = pos;
                        break;
                    }
                }
            }
            if (targetPos == INVALID_POS) {
                Position midPos = make_pair(0, 0);
                for (auto pos : gem) {
                    midPos.first += pos.first;
                    midPos.second += pos.second;
                }
                midPos.first /= gem.size();
                midPos.second /= gem.size();
                for (auto pos : gem) {
                    if ((abs(pos.first-midPos.first) + abs(pos.second-midPos.second))
                        < (abs(targetPos.first - midPos.first) + abs(targetPos.second - midPos.second)))
                    {
                        targetPos = pos;
                    }
                }
            }
        }
        for (auto pos : gem) {
            if (pos == targetPos) {
                GetGem(pos) = GemType(GetGem(pos) + 1);
            } else {
                GetGem(pos) = GT_EMPTY;
            }
        }
    }
}

MoveState Game::Move(const Movement &movement) {
    Position newPos = NewPos(movement.first, movement.second);
    if (!ValidPosition(newPos)) {
        return MS_INVALID;
    }
    GemType &first = GetGem(movement.first);
    GemType &second = GetGem(newPos);
    std::swap(first, second);

    auto gems = GemCollection();
    if (gems.empty()) {
        return MS_INVALID;
    }

    vector<Position> movePos{movement.first, newPos};
    while (true) {
        CreateNewGems(gems, movePos);
        movePos.clear();
        FallDown();
        gems = GemCollection();
        if (gems.empty()) {
            break;
        }
    }
    return MS_OK;
}

Position Game::NewPos(const Position& pos, MoveType moveType) const {
    switch(moveType) {
        case MT_UP:
            return std::make_pair(pos.first-1, pos.second);
        case MT_DOWN:
            return std::make_pair(pos.first+1, pos.second);
        case MT_LEFT:
            return std::make_pair(pos.first, pos.second-1);
        case MT_RIGHT:
            return std::make_pair(pos.first, pos.second+1);
    }
    assert(false);
    return std::make_pair(0,0);
}

bool Game::ValidPosition(const Position &pos) const {
    return pos.first >=0 && pos.first < row_max_
    && pos.second >= 0 && pos.second < col_max_;
}

vector<Position> Game::AllSameCollection(const Position &pos, MoveType type) const {
    vector<Position> vec;
    Position current = pos;
    while(true) {
        Position newPos = NewPos(current, type);
        if (!ValidPosition(newPos)) {
            break;
        }
        if (GetGem(pos) != GetGem(newPos)) {
            break;
        }
        current = newPos;
        vec.push_back(newPos);
    }
    return vec;
}

vector<vector<Position>> Game::GemCollection() const {
    vector<vector<Position>> gems;
    vector<vector<pair<int, int>>> collectionIds(row_max_, vector<pair<int, int>>(col_max_, make_pair(-1, -1)));
    for (int i = 0; i < row_max_; ++i) {
        int len = 1;
        for (int j = 0; j < col_max_; ++j) {
            if (map_[i][j] != GT_EMPTY && j != col_max_ - 1 && map_[i][j+1] == map_[i][j+1-len]) {
                len++;
            } else {
                if (len < 3) {
                    continue;
                }
                int id = gems.size();
                vector<Position> collections;
                for (int col = j+1-len; col <= j; ++col) {
                    collectionIds[i][col].first = id;
                    collections.push_back(make_pair(i, col));
                }
                gems.push_back(collections);
                len = 1;
            }
        }
    }
    for (int i = 0; i < col_max_; ++i) {
        int len = 1;
        for (int j = 0; j < row_max_; ++j) {
            if (map_[j][i] != GT_EMPTY && j != row_max_ - 1 && map_[j+1][i] == map_[j+1-len][i]) {
                len++;
            } else {
                if (len < 3) {
                    continue;
                }
                
                int id = gems.size();
                vector<Position> collections;
                for (int row = j+1-len; row <= j; ++row) {
                    collectionIds[row][i].second = id;
                    collections.push_back(make_pair(row, i));
                }
                gems.push_back(collections);
                len = 1;
            }
        }
    }
    vector<int> uset(gems.size());
    for (int i = 0; i < uset.size(); ++i) {
        uset[i] = i;
    }
    for (int i = 0; i < row_max_; ++i) {
        for (int j = 0; j < col_max_; ++j) {
            const pair<int, int> &ids = collectionIds[i][j];
            if (ids.first != -1 && ids.second != -1) {
                uset[uset[ids.first]] = uset[ids.second];
            }
        }
    }

    map<int,set<Position>> result;
    for (int i = 0; i < uset.size(); ++i) {
        int id = i;
        while (true) {
            if (uset[id] == id) {
                break;
            }
            id = uset[id];
        }
        set<Position> &target = result[id];
        target.insert(gems[i].begin(), gems[i].end());
    }

    vector<vector<Position>> finalResult;
    for (auto iter : result) {
        finalResult.push_back(vector<Position>(iter.second.begin(), iter.second.end()));
    }

    return finalResult;
}

void Game::FallDown() {
    if (collections_.size() == 0) {
        //test mode
        return;
    }

    for (int col = 0; col < col_max_; ++col) {
        int cursor = row_max_ - 1;
        for (int row = row_max_ - 1; row > 0; --row) {
            if (map_[row][col] != GT_EMPTY) {
                map_[cursor--][col] = map_[row][col];
            }
        }
        for (int row = 0; row < cursor; ++row) {
            map_[row][col] = NewGem();
        }
    }
}

bool Game::ValidMove(const Movement &movement) {
    return true;
}

GemType Game::NewGem() const {
    return collections_[rand() % collections_.size()];
}

GemType &Game::GetGem(const Position &pos) {
    assert(ValidPosition(pos));
    return map_[pos.first][pos.second];
}

const GemType &Game::GetGem(const Position &pos) const {
    assert(ValidPosition(pos));
    return map_[pos.first][pos.second];
}

void Game::Show() const {
    for (int i = 0; i < row_max_; ++i) {
        for (int j = 0; j < col_max_; ++j) {
            cout << char('a' + map_[i][j]);
        }
        cout << endl;
    } 
}