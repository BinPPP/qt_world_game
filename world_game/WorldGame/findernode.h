#ifndef FINDERNODE_H
#define FINDERNODE_H

#include "world.h"
#include <memory>

/*
 * Author: Floris Janssens
 * E-mail: florisjanssens@outlook.com
 */

/*
 * Has position and parent pointer.
 * By chaining nodes together using the parent (or previous) pointer,
 * a path is represented (or a candidate solution).
 */

class FinderNode
{
public:
    FinderNode(std::shared_ptr<Tile> position, FinderNode* parentNode);

    std::shared_ptr<Tile> getPosition() const;
    void setPosition(const std::shared_ptr<Tile> &value);

    FinderNode* getParentNode() const;
    void setParentNode(FinderNode* &value);

    void setFinalCost(float value);
    float getFinalCost() const;

    float getGivenCost() const;
    void setGivenCost(float value);

    float getHeuristicCost() const;
    void setHeuristicCost(float value);


    bool getClosed() const;
    void setClosed(bool value);

private:
    std::shared_ptr<Tile> position;
    FinderNode* parentNode;

    float finalCost;
    float givenCost;
    float heuristicCost;
    bool closed;

};

#endif // FINDERNODE_H
