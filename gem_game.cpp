#include "gem_game.h"
#include <cstdlib>
#include <map>
#include <set>
#include <ctime>
#include <iostream>
using namespace std;

Game::Game(bool treasure, int rowMax, int colMax, int seed)
    : row_max_(rowMax)
    , col_max_(colMax)
{
    if (treasure) {
        collections_ = {GT_COPPER, GT_SILVER, GT_GOLD, GT_PURSE};
    } else {
        collections_ = {GT_RED, GT_GREEN, GT_BROWN, GT_PURPLE, GT_BLUE, GT_YELLOW, GT_SKELETON};
    }
    srand(time(NULL) * seed);
    map_.resize(rowMax);
    for (size_t i = 0; i < map_.size(); ++i) {
        map_[i].resize(colMax);
        for (size_t j = 0; j < map_[i].size(); ++j) {
            map_[i][j] = NewGem();
        }
    }
    treasure_ = false;
    auto gems = GemCollection();
    while (true) {
        CreateNewGems(gems, {INVALID_POS, INVALID_POS});
        vector<Position> c;
        FallDown(false, c);
        gems = GemCollection();
        if (gems.empty()) {
            break;
        }
    }
    treasure_ = treasure;
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
    for (size_t i = 0; i < map_.size(); ++i) {
        map_[i].resize(col_max_);
        for (size_t j = 0; j < map_[i].size(); ++j) {
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

int find(int x, const vector<int> &uset) {
    while (x != uset[x]) {
        x = uset[x];
    }
    return x;
}

MoveResult Game::Move(const Movement &movement, bool show) {
    std::vector<std::pair<GameMap, std::vector<Position>>> snapshots;
    Position newPos = NewPos(movement.first, movement.second);
    if (!ValidPosition(newPos)) {
        return {MS_INVALID, snapshots};
    }
    GemType &first = GetGem(movement.first);
    GemType &second = GetGem(newPos);

    if (first == GT_DOOR || second == GT_DOOR) {
        return {MS_INVALID, snapshots};
    }
    
    vector<Position> movePos{movement.first, newPos};
    if (show) {
        snapshots.push_back({map_, movePos});
    }

    std::swap(first, second);

    auto gems = GemCollection();
    if (gems.empty()) {
        std::swap(first, second);
        return {MS_INVALID, snapshots};
    }

    MoveState state = MS_OK;
    while (true) {
        if (state != MS_EXTERTURN) {
            for (auto g : gems) {
                if (g.size() > 4) {
                    state = MS_EXTERTURN;
                } else if (g.size() == 4) {
                    state = MS_FOUR;
                }
            }
        }
        vector<Position> changedPos;
        if (show) {
            for (const auto &g: gems) {
                changedPos.insert(changedPos.end(), g.begin(), g.end());
            }
            snapshots.push_back({map_, changedPos});
        }
        CreateNewGems(gems, movePos);
        movePos.clear();
        if (show) {
            snapshots.push_back({map_, changedPos});
        }
        vector<Position> changed;
        FallDown(show, changed);
        if (show) {
            snapshots.push_back({map_, changed});
        }
        gems = GemCollection();
        if (gems.empty()) {
            break;
        }
    }
    return {state, snapshots};
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
    vector<int> uset;
    vector<vector<int>> collectionIds(row_max_, vector<int>(col_max_, -1));
    for (int i = 0; i < row_max_; ++i) {
        int len = 1;
        for (int j = 1; j <= col_max_; ++j) {
            if (j != col_max_ && map_[i][j] > 0 && map_[i][j] == map_[i][j-len]) {
                len++;
            } else {
                if (len < 3) {
                    len = 1;
                    continue;
                }
                int id = gems.size();
                uset.push_back(id);
                vector<Position> collections;
                for (int col = j-len; col < j; ++col) {
                    collectionIds[i][col] = id;
                    collections.push_back(make_pair(i, col));
                }
                gems.push_back(collections);
                len = 1;
            }
        }
    }
    for (int i = 0; i < col_max_; ++i) {
        int len = 1;
        for (int j = 1; j <= row_max_; ++j) {
            if (j != row_max_ && map_[j][i] > 0 && map_[j][i] == map_[j-len][i]) {
                len++;
            } else {
                if (len < 3) {
                    len = 1;
                    continue;
                }
                
                int id = gems.size();
                uset.push_back(id);
                vector<Position> collections;
                for (int row = j-len; row < j; ++row) {
                    if (collectionIds[row][i] != -1) {
                        uset[find(collectionIds[row][i], uset)] = id;
                    } else {
                        collectionIds[row][i] = id;
                    }
                    collections.push_back(make_pair(row, i));
                }
                gems.push_back(collections);
                len = 1;
            }
        }
    }

    map<int,set<Position>> result;
    for (size_t i = 0; i < uset.size(); ++i) {
        int id = find(i, uset);
        set<Position> &target = result[id];
        target.insert(gems[i].begin(), gems[i].end());
    }

    vector<vector<Position>> finalResult;
    for (auto iter : result) {
        finalResult.push_back(vector<Position>(iter.second.begin(), iter.second.end()));
    }

    return finalResult;
}
void Game::FallDown(bool show, vector<Position> &changed) {
    if (collections_.size() == 0) {
        //for predict mode
        return;
    }

    for (int col = 0; col < col_max_; ++col) {
        int cursor = row_max_ - 1;
        for (int row = row_max_ - 1; row >= 0; --row) {
            if (map_[row][col] != GT_EMPTY) {
                map_[cursor--][col] = map_[row][col];
            }
            if (show && (cursor+1) != row) {
                changed.push_back({row, col});
            }
        }
        for (int row = 0; row <= cursor; ++row) {
            map_[row][col] = NewGem();
            if (show) {
                changed.push_back({row, col});
            }
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
        cout << '{';
        for (int j = 0; j < col_max_; ++j) {
            cout << (int(map_[i][j]) > 0 ? int(map_[i][j]) : 0);
            if (j != col_max_ - 1) {
                cout << ',';
            }
        }
        cout << "}," << endl;
    }
    cout << endl;
}

void add(uint64_t &org, uint64_t append) {
    assert((append & 0x0f) == append);
    org = (org << 4) | append;
}

vector<PredictResult> Game::AllValidMove() {
    vector<PredictResult> ret;
    std::vector<GemType> collections;
    swap(collections_, collections);
    for (int i = 0; i < row_max_; ++i) {
        for (int j = 0; j < col_max_; ++j) {
            {
                vector<vector<GemType>> map = map_;
                Movement movement({i, j}, MT_DOWN);
                auto result = Move(movement, false);
                if (result.state != MS_INVALID) {
                    ret.push_back({movement, result, vector<vector<GemType>>()});
                    swap(ret.rbegin()->map, map_);
                }
                swap(map, map_);
            }
            {
                vector<vector<GemType>> map = map_;
                Movement movement({i, j}, MT_RIGHT);
                auto result = Move(movement, false);
                if (result.state != MS_INVALID) {
                    ret.push_back({movement, result, vector<vector<GemType>>()});
                    swap(ret.rbegin()->map, map_);
                }
                swap(map, map_);
            }
        }
    }

    for (auto &result : ret) {
        const auto &map = result.map;
        auto &c33 = result.c33;
        for (int i = 0; i < row_max_ - 3; ++i) {
            for (int j = 0; j < col_max_ - 3; ++j) {
                uint64_t x = 0;
                add(x, map[i][j]);
                add(x, map[i][j+1]);
                add(x, map[i][j+2]);
                
                add(x, map[i+1][j]);
                add(x, map[i+1][j+1]);
                add(x, map[i+1][j+2]);
                
                add(x, map[i+2][j]);
                add(x, map[i+2][j+1]);
                add(x, map[i+2][j+2]);
                c33.push_back(x);
            }
        }
        // auto &c44 = result.c44;
        // for (int i = 0; i < row_max_ - 4; ++i) {
        //     for (int j = 0; j < col_max_ - 4; ++j) {
        //         uint64_t x = 0;
        //         add(x, map[i][j]);
        //         add(x, map[i][j+1]);
        //         add(x, map[i][j+2]);
        //         add(x, map[i][j+3]);
                
        //         add(x, map[i+1][j]);
        //         add(x, map[i+1][j+1]);
        //         add(x, map[i+1][j+2]);
        //         add(x, map[i+1][j+3]);
                
        //         add(x, map[i+2][j]);
        //         add(x, map[i+2][j+1]);
        //         add(x, map[i+2][j+2]);
        //         add(x, map[i+2][j+3]);

        //         add(x, map[i+3][j]);
        //         add(x, map[i+3][j+1]);
        //         add(x, map[i+3][j+2]);
        //         add(x, map[i+3][j+3]);

        //         c44.push_back(x);
        //     }
        // }
        result.map.clear();
    }
    
    swap(collections_, collections);
    return ret;
}
