#ifndef HEALTHPACK_H
#define HEALTHPACK_H

#include <QObject>
#include "world.h"

class HealthPack : public QObject, public Tile
{
    Q_OBJECT
public:
    HealthPack(int xPosition, int yPosition, float heal);
    ~HealthPack() override = default;
    bool getTaken() const;
    void setTaken(bool state);

signals:
    void consumed();

private:
    bool taken;
};

#endif // HEALTHPACK_H
