#ifndef STRATEGY_H
#define STRATEGY_H

#include "world.h"
#include "healthpack.h"

// More information is inside the corresponding cpp file
// The strategy for the automatic game runner to determine
// different strategic goal locations which can be used
// by the model to find a path using the pathfinder and
// automatically play the game
class Strategy
{
public:
    Strategy(std::vector<std::shared_ptr<Tile>> &tiles,
             std::vector<std::shared_ptr<Enemy>> &enemies,
             std::vector<std::shared_ptr<HealthPack>> &healthPacks,
             std::shared_ptr<Protagonist> &protagonist,
             std::vector<std::shared_ptr<Tile>> &poisonedTiles,
             std::shared_ptr<int> enemiesLeft, std::shared_ptr<int> healthpacksLeft);

    // Get the next strategic move (a goal location)
    std::shared_ptr<Tile> getNextMove();
    // Get the closest living enemy
    // (keeps different enemy types and their characteristics in mind)
    std::shared_ptr<Enemy> getClosestEnemy();
    // Get the closest available healthpack
    std::shared_ptr<HealthPack> getClosestHealthpack();
    // Calculate the outcome of a fight between the protagonist and passed enemy
    // (keeps different enemy types in mind)
    float fightHealthOutcome(std::shared_ptr<Enemy> &enemy);

private:
    std::vector<std::shared_ptr<Tile>> tiles;
    std::vector<std::shared_ptr<Enemy>> enemies;
    std::vector<std::shared_ptr<HealthPack>> healthPacks;
    std::shared_ptr<Protagonist> protagonist;
    std::vector<std::shared_ptr<Tile>> poisonedTiles;
    std::shared_ptr<int> enemiesLeft;
    std::shared_ptr<int> healthpacksLeft;
};
#endif // STRATEGY_H
