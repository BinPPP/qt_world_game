#include "strategy.h"
#include "math.h"
#include "gamemodel.h"
#include <QDebug>
#include <QObject>

Strategy::Strategy(std::vector<std::shared_ptr<Tile>> &tiles,
                   std::vector<std::shared_ptr<Enemy>> &enemies,
                   std::vector<std::shared_ptr<HealthPack>> &healthPacks,
                   std::shared_ptr<Protagonist> &protagonist,
                   std::vector<std::shared_ptr<Tile>> &poisonedTiles,
                   std::shared_ptr<int> enemiesLeft, std::shared_ptr<int> healthpacksLeft):
                   tiles{tiles}, enemies{enemies}, healthPacks{healthPacks},
                   protagonist{protagonist}, poisonedTiles{poisonedTiles},
                   enemiesLeft{enemiesLeft}, healthpacksLeft{healthpacksLeft}
{
}

// Get the next strategic move (just a goal location, in the model, the pathfinder is used to find the path)
std::shared_ptr<Tile> Strategy::getNextMove()
{
    // Get the closest enemy (keeps defeated enemies and PEnemies in mind, etc.)
    // It is assumed that there are still enemies left here (otherwise the game would have ended)
    std::shared_ptr<Enemy> enemy = getClosestEnemy();

    // Calculate resulting fight health (keeps type of enemy in mind)
    float newHealth = fightHealthOutcome(enemy);

    // If the fight would result in a dead for the prot, go for a healthpack
    if (newHealth <= 0 && *healthpacksLeft > 0) // go for health
    {
        std::shared_ptr<HealthPack> healthpack = getClosestHealthpack();
        return std::make_shared<Tile>(healthpack->getXPos(), healthpack->getYPos(), healthpack->getValue());
    } else // go for enemy
    {
        return std::make_shared<Tile>(enemy->getXPos(), enemy->getYPos(), enemy->getValue());
    }
}

// Get the closest living enemy (keeps defeated enemies and PEnemies in mind, etc.)
// It is assumed that there are still enemies left here (otherwise the game would have ended)
// Also waits on a PEnemy to let it lose its poison when it's the last one alive (avoids a lot of damage)
// If a new EREnemy spawns, it also stops waiting and moves to this EREnemy, etc.
std::shared_ptr<Enemy> Strategy::getClosestEnemy()
{
    std::shared_ptr<Enemy> enemy = enemies.at(0);
    int min_distance = std::numeric_limits<float>::infinity();

    for (auto &e : enemies)
    {
        // If the enemy is already defeated, go to the next one (PEnemies are also set to defeated without emitting dead() twice in the model)
        if (e->getDefeated()) continue;

        if (std::shared_ptr<PEnemy> pe = std::dynamic_pointer_cast<PEnemy>(e)) // if PEnemy
        {
            // If the PEnemy was touched already, keep floating if he is the last enemy left (wait until he has lost all his poison and dies, less damage) or move to the next enemy
            // If the EREnemy respawns in the mean time, the strategy moves to the respawned enemy
            if (!(fabs(pe->getPoisonLevel() - pe->getValue()) <= std::numeric_limits<float>::epsilon())) // if touched
            {
                // if it's the last enemy
                if (*enemiesLeft == 1)
                {
                    // Keep standing on the current position
                    return std::make_shared<Enemy>(protagonist->getXPos(), protagonist->getYPos(), protagonist->getValue());
                } else { // Otherwise move to some other enemy
                    continue;
                }
            }
        }

        // Calculate the manhattan distance to the enemy, if the enemy is closer than the previous one, keep him
        int manhattanDistance = std::abs(protagonist->getXPos() - e->getXPos()) + std::abs(protagonist->getYPos() - e->getYPos());
        if (manhattanDistance < min_distance)
        {
            min_distance = manhattanDistance;
            enemy = e;
        }

    }
    return enemy;
}

// Calculate the outcome of a fight between the protagonist and an enemy
// Also keeps the different damage model of the PEnemies in mind
// (only the poison does random damage for these, and in smaller amounts to
// make the game more balanced)
float Strategy::fightHealthOutcome(std::shared_ptr<Enemy> &enemy)
{
    float newHealth = 0;
    if (std::shared_ptr<PEnemy> pe = std::dynamic_pointer_cast<PEnemy>(enemy)) // If PEnemy
    {
        // Find the poisoned tile underneath the enemy and calculate the outcome of moving to the tile on the health
        for (auto &ptile : poisonedTiles)
        {
            if (pe->getXPos() == ptile->getXPos() && pe->getYPos() == ptile->getYPos())
            {
                newHealth = static_cast<int>(protagonist->getHealth() - ptile->getValue());
            }
        }
    } else // Any other type of enemy
    {
        // For all other enemies the damage is just the strength
        newHealth = static_cast<int>(protagonist->getHealth() - enemy->getValue());
    }
    return newHealth;
}

// Find the closest (available) healthpack using the Manhattan distance
std::shared_ptr<HealthPack> Strategy::getClosestHealthpack()
{
    std::shared_ptr<HealthPack> healthpack = healthPacks.at(0);
    int min_distance = std::numeric_limits<float>::infinity();
    for (auto &h : healthPacks)
    {
        // If the healthpack is already taken, go to the next one
        if (h->getTaken()) continue;
        int manhattanDistance = std::abs(protagonist->getXPos() - h->getXPos()) + std::abs(protagonist->getYPos() - h->getYPos());
        if (manhattanDistance < min_distance)
        {
            min_distance = manhattanDistance;
            healthpack = h;
        }
    }
    return healthpack;
}
