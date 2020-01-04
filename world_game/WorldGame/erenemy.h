#ifndef ERENEMY_H
#define ERENEMY_H

#include "world.h"

// Takes health away when touched but gives back energy
// Respawns in a random location and should be dead at the end of the game
// The goal is to only have one of these in each game
// to serve as some way to sacrifice health for more energy
// (as you would need to kill this enemy multiple times and lose
// more health in total than just killing it once in the end)
// The health it takes and energy it gives is also random so it's
// risky to go for the EREnemy while it can give great rewards at the same time
class EREnemy : public Enemy
{
    Q_OBJECT
public:
    EREnemy(int xPosition, int yPosition, float strength);
    ~EREnemy() override = default;

    float getEnergyVal() const;
    void setEnergyVal(float Eval);

public slots:
    void initRespawn(int xPosition, int yPosition, float strength);
    void respawn();

signals:
    void respawned();

private:
    float energyVal;
};

#endif // ERENEMY_H
