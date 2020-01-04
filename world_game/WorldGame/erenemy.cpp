#include "erenemy.h"
#include <iostream>
#include <QTimer>

#define ENERGY_FACTOR 1.5f

EREnemy::EREnemy(int xPosition, int yPosition, float strength):
    Enemy(xPosition, yPosition, strength), energyVal{ENERGY_FACTOR*strength}
{
    qsrand(time(nullptr));
}

float EREnemy::getEnergyVal() const
{
    return energyVal;
}

void EREnemy::setEnergyVal(float eVal)
{
    energyVal = eVal;
}

void EREnemy::initRespawn(int xPosition, int yPosition, float strength)
{
    this->setXPos(xPosition);
    this->setYPos(yPosition);
    this->setValue(strength);
    this->setEnergyVal(ENERGY_FACTOR * strength);

    int t = qrand()%15;
    std::cout << "starting respawn timer for " << t << " seconds" << std::endl;
    QTimer::singleShot(t*1000, this, SLOT(respawn()));
}

void EREnemy::respawn()
{
    std::cout << "enemy respawned at x:" << this->getXPos() << " y:" << this->getYPos() << " with strength: "
              << this->getValue() << " energyValue: " << this->getEnergyVal() << std::endl;
    this->setDefeated(false);
    emit respawned();
}
