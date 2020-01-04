#include "gamemodel.h"
#include <QTimer>
#include <chrono>
#include <algorithm>

GameModel::GameModel() : autorunSpeed{1000}, autorunWeight{0.0f}{

}

void GameModel::generateWorld(QString filename, unsigned int nrOfEnemies, unsigned int nrOfHealthpacks)
{
    clearTiles();
    autorunBusy = false;
    currentPathPos = 0;

    std::unique_ptr<World> world = std::make_unique<World>();
    world->createWorld(filename, nrOfEnemies, nrOfHealthpacks);

    std::vector<std::unique_ptr<Tile>> tiles_unique = world->getTiles();
    std::move(tiles_unique.begin(), tiles_unique.end(),
               std::back_inserter(tiles));

    mapCols = world->getCols();
    mapRows = world->getRows();

    pathfinder = std::make_shared<Pathfinder>(tiles, mapRows, mapCols);

    std::vector<std::unique_ptr<Enemy>> enemies_unique = world->getEnemies();
    std::move(enemies_unique.begin(), enemies_unique.end(),
               std::back_inserter(enemies));

    // Spawn one EREnemy at a location where there isn't a PEnemy
    // (or none if there's only a PEnemy)
    bool EREnemyInserted = false;
    for (auto &e : enemies)
    {
        if(std::dynamic_pointer_cast<PEnemy>(e)) // If PEnemy, put poison underneath
        {
            poisonedTiles.push_back(std::make_shared<Tile>(e->getXPos(), e->getYPos(), qrand()%15));
        } else if (!EREnemyInserted) // If any other type and EREnemy not inserted, make the enemy an EREnemy
        {
            std::shared_ptr<EREnemy> pe = std::make_shared<EREnemy>(e->getXPos(), e->getYPos(), e->getValue());
            e = pe;
            connect(pe.get(), &EREnemy::respawned, this, &GameModel::enemyRespawned);
            EREnemyInserted = true;
        }

        connect(e.get(), &Enemy::dead, this, &GameModel::enemyDead);
    }

    for (auto &healthPack : world->getHealthPacks())
        healthPacks.push_back(std::make_shared<HealthPack>(healthPack->getXPos(), healthPack->getYPos(), healthPack->getValue()));

    protagonist = world->getProtagonist();
    maxHealth = protagonist->getHealth();
    maxEnergy = protagonist->getEnergy();

    enemiesLeft = std::make_shared<int>(int(enemies.size()));
    healthpacksLeft = std::make_shared<int>(int(healthPacks.size()));

    strategy = std::make_shared<Strategy>(tiles, enemies, healthPacks, protagonist, poisonedTiles, enemiesLeft, healthpacksLeft);
}

void GameModel::clearTiles()
{
    if(tiles.size()!=0) tiles.clear();
    if(enemies.size()!=0) enemies.clear();
    if(healthPacks.size()!=0) healthPacks.clear();
    if(poisonedTiles.size()!=0) poisonedTiles.clear();
}

std::shared_ptr<const Protagonist> GameModel::getProtagonist() const
{
    return protagonist;
}

const std::vector<std::shared_ptr<HealthPack>> &GameModel::getHealthPacks() const
{
    return healthPacks;
}

const std::vector<std::shared_ptr<Enemy>> &GameModel::getEnemies() const
{
    return enemies;
}

const std::vector<std::shared_ptr<Tile> > &GameModel::getPoisonTiles() const
{
    return poisonedTiles;
}

const std::vector<std::shared_ptr<Tile>> &GameModel::getTiles() const
{
    return tiles;
}

int GameModel::getMapRows() const
{
    return mapRows;
}

int GameModel::getMapCols() const
{
    return mapCols;
}

// Finds an isolated path from start to goal and time the pathfinding duration in ms
// Can be used to test on the biggest maze for example (takes around 350 ms including freeing memory
// with weight = 1 and set on release mode)
// Doesn't keep the game logic in mind as it serves as a way to show the workings of the pathfinder
// Of course the tile values are considered
// Also emits signals to visualize the path in the views and display timing and cost info
void GameModel::findTimeIsolatedPath(int startX, int startY, int goalX, int goalY, float weight)
{
    if (isinf(tiles.at(unsigned(startY * mapCols + startX))->getValue()) ||
            isinf(tiles.at(unsigned(goalY * mapCols + goalX))->getValue()))
    {
        emit pathFinderMessage(false, "Start or end position is an impassable tile");
    } else
    {
        auto start = std::chrono::high_resolution_clock::now();
        bool pathFound = pathfinder->findPath(startX, startY, goalX, goalY, weight);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        if (pathFound)
        {
            std::vector<std::shared_ptr<Tile>> solutionList = pathfinder->getSolutionList();
            std::reverse(std::begin(solutionList), std::end(solutionList));
            float sum = std::accumulate(std::begin(solutionList), std::end(solutionList), 0.0f,
                                      [&](float a, std::shared_ptr<Tile> &p){ return a + (1-p->getValue()); });
            emit newPath(solutionList);
            emit pathFinderMessage(true, "Path found in " + QString::number(duration.count()) + " ms (" + QString::number(sum) + " energy)");
        } else {
            emit pathFinderMessage(false, "Unable to find a path");
        }
    }
}

// Automatically run the game using a strategy (look in strategy.h/cpp)
// Gets the next coordinate to go to from the strategy
// uses the pathfinder to find the best path
// then moves the protagonist tile by tile and runs the game logic at every move
// Speed controlled by the slider
// Note: the strategy doesn't do anything with the energy but just finds the best next move
//  The actual gameover in this case still happens while running the game logic
//  as the purpose is to visualize the protagonist running out of energy/health too
//  so all gameover/win cases are still being handled by running the game logic
void GameModel::autorun()
{
    if (*enemiesLeft == 0) return;

    if (!autorunBusy)
    {
        autorunBusy = true;
        std::shared_ptr<Tile> destTile = strategy->getNextMove();
        qDebug() << QString::number(destTile->getXPos()) + " " + QString::number((destTile->getYPos()));

        bool pathFound = pathfinder->findPath(protagonist->getXPos(), protagonist->getYPos(), destTile->getXPos(), destTile->getYPos(), autorunWeight);

        if (!pathFound)
        {
            emit deadEndMap();
            return;
        }

        autorunTiles = pathfinder->getSolutionList();

        // Emit the path for visualization (has to be reversed)
        std::vector<std::shared_ptr<Tile>> reversed;
        reversed.insert(std::end(reversed), std::rbegin(autorunTiles), std::rend(autorunTiles));
        emit newPath(reversed);

        if (autorunTiles.size() > 1)
        {
            currentPathPos = autorunTiles.size() - 2;
        } else {
            currentPathPos = 0;
        }
    }

    auto tile = autorunTiles.at(currentPathPos);

    // If the next auto move actually moves to a new tile, update prot position and run logic
    // It may stay on the same tile in case there's only one enemy left and it's a PEnemy
    // In this case the protagonist will take the least damage just standing on the PEnemy until he dies
    // The standing around could get interrupted of course if the EREnemy respawns in the mean time
    if (!(tile->getXPos() == protagonist->getXPos() && tile->getYPos() == protagonist->getYPos()) &&
            static_cast<int>(protagonist->getHealth()) > 0)
    {
        protagonist->setPos(tile->getXPos(), tile->getYPos());
        runGameLogic();
    }

    if (--currentPathPos < 0) autorunBusy = false;

    QTimer::singleShot(autorunSpeed, this, SLOT(autorun()));

}

// Set the weight used by the A* pathfinder while autorunning
void GameModel::setAutorunWeight(float value)
{
    autorunWeight = value;
}

// Set the speed at which to run the game logic while autorunning
void GameModel::setAutorunSpeed(int value)
{
    autorunSpeed = value;
}

// Move the protagonist, based on this, the game logic should be run
// Also checks if the move would be out of bounds or not
void GameModel::moveProtagonist(MoveDirection direction)
{
    switch(direction)
    {
        case MoveDirection::UP:
            if (protagonist->getYPos() > 0 && !isinf(tiles[unsigned((protagonist->getYPos()-1)*mapCols + protagonist->getXPos())]->getValue())) // movement is not out of bounds
            {
                protagonist->setPos(protagonist->getXPos(), protagonist->getYPos()-1);
                runGameLogic();
            }
            break;
        case MoveDirection::DOWN:
            if (protagonist->getYPos() < mapRows-1 && !isinf(tiles[unsigned((protagonist->getYPos()+1)*mapCols + protagonist->getXPos())]->getValue())) // movement is not out of bounds
            {
                protagonist->setPos(protagonist->getXPos(), protagonist->getYPos()+1);
                runGameLogic();
            }
            break;
        case MoveDirection::LEFT:
            if (protagonist->getXPos() > 0 && !isinf(tiles[unsigned(protagonist->getYPos()*mapCols + protagonist->getXPos()-1)]->getValue())) // movement is not out of bounds
            {
                protagonist->setPos(protagonist->getXPos()-1, protagonist->getYPos());
                runGameLogic();
            }
            break;
        case MoveDirection::RIGHT:
            if (protagonist->getXPos() < mapCols-1 && !isinf(tiles[unsigned(protagonist->getYPos()*mapCols + protagonist->getXPos()+1)]->getValue())) // movement is not out of bounds
            {
                protagonist->setPos(protagonist->getXPos()+1, protagonist->getYPos());
                runGameLogic();
            }
            break;
    }
}

// If the poison level of a PEnemy is updated, find out which tiles to poison
// Starts with the neighbors, and expands after every update
void GameModel::updatePoison(float poisonLevel)
{
    Enemy* enemy = qobject_cast<Enemy*>(sender());
    int surroundingLevel = static_cast<int>((enemy->getValue()-poisonLevel)/10);
    std::vector<std::shared_ptr<Tile>> newTiles;

    // Calculates the next surrounding level of tiles in rows and columns
    // 2*surroundingLevel+1 is the length of such row or column (in the beginning this is 3 for example)
    // Basically just calculates the top/bottom row of the update (depending on the level)
    // and then the left/right column without the tiles which are already in the top/bottom row
    // Also checks if the tile would be within the boundaries
    bool protInNewPoison = false;
    for (int i=0; i<2*surroundingLevel+1; i++)
    {
        if (enemy->getXPos()-surroundingLevel+i >= 0 && enemy->getXPos()-surroundingLevel+i < mapCols)
        {
            if (enemy->getYPos()-surroundingLevel >= 0 && enemy->getYPos()-surroundingLevel < mapRows)
            {
                newTiles.push_back(std::make_shared<Tile>(enemy->getXPos()-surroundingLevel+i, enemy->getYPos()-surroundingLevel, qrand()%5));
                if (protagonist->getXPos() == enemy->getXPos()-surroundingLevel+i && protagonist->getYPos() == enemy->getYPos()-surroundingLevel) protInNewPoison = true;
            }
            if (enemy->getYPos()+surroundingLevel >= 0 && enemy->getYPos()+surroundingLevel < mapRows)
            {
                newTiles.push_back(std::make_shared<Tile>(enemy->getXPos()-surroundingLevel+i, enemy->getYPos()+surroundingLevel, qrand()%5));
                if (protagonist->getXPos() == enemy->getXPos()-surroundingLevel+i && protagonist->getYPos() ==  enemy->getYPos()+surroundingLevel) protInNewPoison = true;
            }
        }
        if (i!=0 && i!=2*surroundingLevel && enemy->getYPos()-surroundingLevel+i >= 0 && enemy->getYPos()-surroundingLevel+i < mapRows)
        {
            if(enemy->getXPos()-surroundingLevel >= 0 && enemy->getXPos()-surroundingLevel < mapCols)
            {
                newTiles.push_back(std::make_shared<Tile>(enemy->getXPos()-surroundingLevel, enemy->getYPos()-surroundingLevel+i, qrand()%5));
                if (protagonist->getXPos() == enemy->getXPos()-surroundingLevel && protagonist->getYPos() == enemy->getYPos()-surroundingLevel+i) protInNewPoison = true;
            }
            if(enemy->getXPos()+surroundingLevel >= 0 && enemy->getXPos()+surroundingLevel < mapCols)
            {
                newTiles.push_back(std::make_shared<Tile>(enemy->getXPos()+surroundingLevel, enemy->getYPos()-surroundingLevel+i, qrand()%5));
                if (protagonist->getXPos() == enemy->getXPos()+surroundingLevel && protagonist->getYPos() == enemy->getYPos()-surroundingLevel+i) protInNewPoison = true;
            }
        }
    }

    poisonedTiles.insert(std::end(poisonedTiles), std::begin(newTiles), std::end(newTiles));
    if (protInNewPoison) checkProtOnPoison();
    emit newTilesPoisoned(newTiles);

}

// If an enemy is dead, update amount of enemies, check gameOver or not, etc.
void GameModel::enemyDead()
{
    (*enemiesLeft)--;
    if (*enemiesLeft <= 0) emit gameOver(true); // Game is won if no enemies left

    Enemy* enemy = qobject_cast<Enemy*>(sender());

    // If th enemy is a PEnemy, disconnect the dead signal and set him to defeated
    // The disconnect is done because the library definition for PEnemy emits dead
    // when the poison runs out but setting the enemy to defeated would emit the dead signal again
    if (PEnemy* pe = dynamic_cast<PEnemy*>(enemy))
    {
        disconnect(pe, &Enemy::dead, this, &GameModel::enemyDead);
        pe->setDefeated(true);
    }

    emit enemyDefeated(enemy->getXPos(), enemy->getYPos());

    // If EREnemy, respawn on a location
    // Only respawns on tiles without enemies (except if they are dead) and
    // without healthpacks (except if they are taken).
    // Also doesn't spawn on infinity tiles (impassable)
    // and doesn't spawn on the location where the EREnemy previously was
    if (EREnemy* ere = dynamic_cast<EREnemy*>(enemy))
    {
        qsrand(time(nullptr));
        bool foundLocation = false;
        int xPos=0, yPos=0;
        float strength=0;
        while (!foundLocation)
        {
            xPos = qrand() % mapCols;
            yPos = qrand() % mapRows;
            strength = qrand()%100;

            if (std::find_if(enemies.begin(), enemies.end(),
                [&](std::shared_ptr<Enemy>& p)-> bool
                {return  p->getXPos() == xPos && p->getYPos() == yPos && !p->getDefeated();})
                == enemies.end() &&
                std::find_if(healthPacks.begin(), healthPacks.end(),
                [&](std::shared_ptr<HealthPack>& p)-> bool
                {return  p->getXPos() == xPos && p->getYPos() == yPos && !p->getTaken();})
                == healthPacks.end() &&
                ere->getXPos() != xPos && ere->getYPos() != yPos &&
                !isinf(tiles.at(unsigned(yPos*mapCols+xPos))->getValue()))
                {
                    foundLocation = true;
                }
        }
        ere->initRespawn(xPos,yPos,strength); // initialize the respawn (which happens after a random time interval)
    }

}

// If the EREnemy respawns, add him back to the game and emit signal for easy usage in view
void GameModel::enemyRespawned()
{
    if (enemiesLeft > 0 && protagonist->getHealth() > 0) // If game isn't over, respawn the enemy
    {
        EREnemy* enemy = qobject_cast<EREnemy*>(sender());
        (*enemiesLeft)++;
        emit EREnemyRespawned(enemy->getXPos(), enemy->getYPos(), enemy->getValue(), enemy->getEnergyVal());
    }
}

// Run the logic of the game, updates energy, health, enemies, healthpacks, etc.
void GameModel::runGameLogic()
{
    updateProtagonistEnergy();
    checkProtOnLivingEnemy();
    checkProtOnHealthpack();
    checkProtOnPoison();
}

// Update the energy of the protagonist, emit game over if energy is used up
void GameModel::updateProtagonistEnergy()
{
    int position1D = protagonist->getYPos()*mapCols + protagonist->getXPos();

    float newEnergy = protagonist->getEnergy() - (1 - tiles[unsigned(position1D)]->getValue());
    if (static_cast<int>(newEnergy) <= 0)
    {
        protagonist->setEnergy(0);
        emit gameOver(false);
    }

    protagonist->setEnergy(newEnergy);
}

// Check if protagonist moved to a tile with an enemy that's still alive and update the game accordingly
void GameModel::checkProtOnLivingEnemy()
{
    for (auto &enemy : enemies)
    {
        if(enemy->getXPos() == protagonist->getXPos() &&
           enemy->getYPos() == protagonist->getYPos() &&
           enemy->getDefeated() == false)
        {
            if (std::shared_ptr<PEnemy> pe = std::dynamic_pointer_cast<PEnemy>(enemy)) // PEnemy
            {
                // If the current poison level is the same as the strength, the PEnemy wasn't touched before
                // Then setup the connects and start the poison
                if (fabs(pe->getPoisonLevel() - pe->getValue()) <= std::numeric_limits<float>::epsilon()) // Compare floats
                {
                    connect(pe.get(), &PEnemy::poisonLevelUpdated, this, &GameModel::updatePoison);
                    pe->poison();
                }
            } else // Normal enemy or EREnemy (update health, determine outcome of fight, etc.)
            {
                float newHealth = protagonist->getHealth() - enemy->getValue();
                if (static_cast<int>(newHealth) > 0) // If prot has health left after the fight
                {
                    // if EREnemy, update energy too
                    if (std::shared_ptr<EREnemy> ere = std::dynamic_pointer_cast<EREnemy>(enemy))
                    {
                        float newEnergy = protagonist->getEnergy() + ere->getEnergyVal();
                        if (static_cast<int>(newEnergy) > maxEnergy) newEnergy = maxEnergy;
                        protagonist->setEnergy(newEnergy);
                    }

                    protagonist->setHealth(newHealth);
                    enemy->setDefeated(true); // dead signal executes enemyDead slot
                } else // If prot is dead after the fight
                {
                    protagonist->setHealth(0);
                    emit gameOver(false);
                }
            }
            break;
        }
    }
}

// Check if protagonist moved to a healthpack and update health
void GameModel::checkProtOnHealthpack()
{
    for (auto &healthpack : healthPacks)
    {
        if(healthpack->getXPos() == protagonist->getXPos() &&
           healthpack->getYPos() == protagonist->getYPos() &&
           healthpack->getTaken() == false)
        {
            healthpack->setTaken(true);
            (*healthpacksLeft)--;
            emit healthPackTaken(healthpack->getXPos(), healthpack->getYPos());
            float newHealth = protagonist->getHealth() + healthpack->getValue();
            if (newHealth >= maxHealth) newHealth = maxHealth;
            protagonist->setHealth(newHealth);
            break;
        }
    }
}

// Check if protagonist is on a poisoned tile, update health, check if game is over
// (A tile with a healthpack can also be poisoned)
void GameModel::checkProtOnPoison()
{
    for (auto &poisonTile : poisonedTiles)
    {
        if(poisonTile->getXPos() == protagonist->getXPos() &&
           poisonTile->getYPos() == protagonist->getYPos())
        {
            float newHealth = protagonist->getHealth() - poisonTile->getValue();
            if (static_cast<int>(newHealth) > 0) // If prot has health left after walking the poison
            {
                protagonist->setHealth(newHealth);
            } else // If prot is dead after walking on poison
            {
                protagonist->setHealth(0);
                emit gameOver(false);
            }
            break;
        }
    }
}
