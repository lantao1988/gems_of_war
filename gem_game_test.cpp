#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gem_game.h"

using namespace std;
using namespace testing;

class GameTest : public ::testing::Test {
};

TEST_F(GameTest, testGemCollection) {
    {
        vector<vector<int>> map = {{2,2,2,2,2}};
        Game game(map);
        auto actual = game.GemCollection();
        ASSERT_EQ(actual.size(), 1);
        ASSERT_THAT(actual[0], UnorderedElementsAreArray(
            vector<Position>{{0,0}, {0,1}, {0,2}, {0,3}, {0,4}}));
    }
    {
        vector<vector<int>> map = {{2,2,2,2,2,2}};
        Game game(map);
        auto actual = game.GemCollection();
        ASSERT_EQ(actual.size(), 1);
        ASSERT_THAT(actual[0], UnorderedElementsAreArray(
            vector<Position>{{0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}}));
    }
    {
        vector<vector<int>> map = {
            {4,2,3},
            {2,4,3},
            {3,3,3}
            };
        Game game(map);
        auto actual = game.GemCollection();
        ASSERT_EQ(actual.size(), 1);
        ASSERT_THAT(actual[0], UnorderedElementsAreArray(
            vector<Position>{{0,2}, {1,2}, {2,1}, {2,2}, {2,0}}));
    }
    {
        vector<vector<int>> map = {
            {4,3,4},
            {3,3,3},
            {4,3,2}
            };
        Game game(map);
        auto actual = game.GemCollection();
        ASSERT_EQ(actual.size(), 1);
        ASSERT_THAT(actual[0], UnorderedElementsAreArray(
            vector<Position>{{0,1}, {1,0}, {1,1}, {1,2}, {2,1}}));
    }
}

TEST_F(GameTest, testMove) {
    vector<vector<int>> map = {
        {4,2,3},
        {4,2,3},
        {2,4,5}
    };
    Game game(map);
    auto state = game.Move(Movement(make_pair(2,0), MT_RIGHT));
    ASSERT_EQ(state, MS_OK);
}