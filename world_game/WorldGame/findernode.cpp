#include "findernode.h"

/*
 * Author: Floris Janssens
 * E-mail: florisjanssens@outlook.com
 */

FinderNode::FinderNode(std::shared_ptr<Tile> position, FinderNode* parentNode):
    position{position}, parentNode{parentNode}, finalCost{0.0f}, givenCost{0.0f}, heuristicCost{0.0f}, closed{false}
{}

std::shared_ptr<Tile> FinderNode::getPosition() const
{
    return position;
}

void FinderNode::setPosition(const std::shared_ptr<Tile> &value)
{
    position = value;
}

FinderNode* FinderNode::getParentNode() const
{
    return parentNode;
}

void FinderNode::setParentNode(FinderNode* &value)
{
    parentNode = value;
}

float FinderNode::getFinalCost() const
{
    return finalCost;
}

float FinderNode::getGivenCost() const
{
    return givenCost;
}

void FinderNode::setGivenCost(float value)
{
    givenCost = value;
}

float FinderNode::getHeuristicCost() const
{
    return heuristicCost;
}

void FinderNode::setHeuristicCost(float value)
{
    heuristicCost = value;
}

bool FinderNode::getClosed() const
{
    return closed;
}

void FinderNode::setClosed(bool value)
{
    closed = value;
}

void FinderNode::setFinalCost(float value)
{
    finalCost = value;
}
