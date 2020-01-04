#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "findernode.h"
#include <vector>

/*
 * Author: Floris Janssens
 * E-mail: florisjanssens@outlook.com
 */

class Pathfinder
{
public:
    Pathfinder(std::vector<std::shared_ptr<Tile>> &tiles, int rows, int cols);

    bool findPath(int startX, int startY, int goalX, int goalY, float weight);

    const std::vector<std::shared_ptr<Tile>> &getSolutionList() const;

private:
    std::vector<std::shared_ptr<Tile>> tiles;
    int rows;
    int cols;

    std::vector<std::shared_ptr<Tile>> solutionList;

    float calcHeuristic(int x1, int x2, int y1, int y2);
    float calcFinalCost(float givenCost, float heuristicCost, float weight);

};

#endif // PATHFINDER_H
