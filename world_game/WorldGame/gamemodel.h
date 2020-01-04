#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <memory>
#include <QDebug>
#include "world.h"
#include "healthpack.h"
#include "movedirection.h"
#include "math.h"
#include "erenemy.h"
#include "pathfinder.h"
#include "strategy.h"

class GameModel : public QObject
{
    Q_OBJECT
public:
    GameModel();

    // Generate the world and store generated information (tiles, enemies, protagonist, healthpacks)
    // into the appropriate variables (transfer ownership of unique pointers, etc.)
    void generateWorld(QString filename, unsigned int nrOfEnemies, unsigned int nrOfHealthpacks);
    void clearTiles();

    // Getters to make signal-slots connections and get position information
    // Best to not use these to change something, add more methods like moveProtagonist if necessary
    const std::vector<std::shared_ptr<Tile>> &getTiles() const;
    const std::vector<std::shared_ptr<Enemy>> &getEnemies() const;
    const std::vector<std::shared_ptr<HealthPack>> &getHealthPacks() const;
    std::shared_ptr<const Protagonist> getProtagonist() const;
    const std::vector<std::shared_ptr<Tile>> &getPoisonTiles() const;
    // Getters to get map dimensions
    int getMapRows() const;
    int getMapCols() const;

    void findTimeIsolatedPath(int startX, int startY, int goalX, int goalY, float weight);

signals:
    void gameOver(bool result);
    void healthPackTaken(int x, int y);
    void enemyDefeated(int x, int y);
    void newTilesPoisoned(std::vector<std::shared_ptr<Tile>> newPoisonTiles);
    void EREnemyRespawned(int x, int y, float strength, float energyVal);

    void newPath(std::vector<std::shared_ptr<Tile>> newPathTiles);
    void pathFinderMessage(bool pathFound, QString costTimeMessage);
    void deadEndMap();

public slots:
    // Move the protagonist, based on this, the game logic should be run
    // This should probably be the only game logic related method you call from outside this class
    // Moving the protagonist updates the protagonist, enemies, healthpacks, etc. and afterwards the
    // generated signals by the World library can be used
    void moveProtagonist(MoveDirection direction);
    void updatePoison(float poisonLevel);
    void enemyDead();
    void enemyRespawned();
    void autorun();
    void setAutorunSpeed(int value);
    void setAutorunWeight(float value);

private:
    std::vector<std::shared_ptr<Tile>> tiles;
    std::vector<std::shared_ptr<Enemy>> enemies;
    std::vector<std::shared_ptr<HealthPack>> healthPacks;
    std::shared_ptr<Protagonist> protagonist;
    std::vector<std::shared_ptr<Tile>> poisonedTiles;
    std::shared_ptr<Pathfinder> pathfinder;
    std::shared_ptr<Strategy> strategy;
    std::vector<std::shared_ptr<Tile>> autorunTiles;
    int mapRows;
    int mapCols;
    std::shared_ptr<int> enemiesLeft;
    std::shared_ptr<int> healthpacksLeft;
    int currentPathPos; // Keeps the curent position in the autorunTiles vector while autorunning
    int autorunSpeed;
    float autorunWeight;
    float maxHealth;
    float maxEnergy;
    bool autorunBusy; // To save if the autorunner is currently executing the logic on a found path

    // Run the game logic
    // Updates protagonist, enemies, healthpacks, etc. based on this game logic
    void runGameLogic();

    void updateProtagonistEnergy();
    void checkProtOnLivingEnemy();
    void checkProtOnHealthpack();
    void checkProtOnPoison();
};

#endif // GAMEMODEL_H
