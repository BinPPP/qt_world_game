#include "healthpack.h"

HealthPack::HealthPack(int xPosition, int yPosition, float heal):
    Tile(xPosition, yPosition, heal), taken{false}
{
}

bool HealthPack::getTaken() const
{
    return taken;
}

void HealthPack::setTaken(bool state)
{
    taken = state;
    if (taken) emit consumed();
}
