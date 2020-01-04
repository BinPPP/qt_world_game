#include "pathfinder.h"
#include <limits>
#include <math.h>
#include <queue>

/*
 * Author: Floris Janssens
 * E-mail: florisjanssens@outlook.com
 */

class CompareFinalCost
{
public:
    bool operator()(FinderNode* left, FinderNode* right)
    {
        return (left->getFinalCost()) > (right->getFinalCost());
    }
};

Pathfinder::Pathfinder(std::vector<std::shared_ptr<Tile>> &tiles, int rows, int cols):
    tiles{tiles}, rows{rows}, cols{cols}
{
}

bool Pathfinder::findPath(int startX, int startY, int goalX, int goalY, float weight)
{
    solutionList.clear();

    bool pathFound = false;

    int successorCoord[4][2] = {	{0,  -1},	// N
                               // {1,  -1},	// NE
                                {1,   0},	// E
                                //{1,   1},	// SE
                                {0,   1},	// S
                                //{-1,  1},	// SW
                                {-1,  0},	// W
                                //{-1, -1}	// NW
    };

    std::vector<FinderNode*> vec(unsigned(rows*cols), nullptr);

    std::priority_queue<FinderNode*, std::vector<FinderNode*>, CompareFinalCost> openQueue;

    FinderNode *rootNode;
    FinderNode *currentNode;
    rootNode = new FinderNode(tiles[unsigned(startY * cols + startX)], nullptr);
    rootNode->setHeuristicCost(calcHeuristic(startX, goalX, startY, goalY));
    rootNode->setFinalCost(calcFinalCost(0, rootNode->getHeuristicCost(), weight));
    vec[unsigned(startY * cols + startX)] = rootNode;
    openQueue.push(rootNode);

    while(!openQueue.empty())
    {
        currentNode = openQueue.top();
        openQueue.pop();

        if (currentNode->getPosition()->getXPos() == goalX &&
                        currentNode->getPosition()->getYPos() == goalY)
        {
            while(currentNode != nullptr)
            {
                solutionList.push_back(currentNode->getPosition());
                currentNode = currentNode->getParentNode();
            }

            pathFound = true;
            break;
        }


        for (int i=0; i<4; i++)
        {
            int successorX = successorCoord[i][0] + currentNode->getPosition()->getXPos();
            int successorY = successorCoord[i][1] + currentNode->getPosition()->getYPos();

            if (successorX < 0 || successorY < 0) continue;
            if (successorX >= cols || successorY >= rows) continue;
            int map2Dto1D = successorY * cols + successorX;
            if (isinf(tiles[unsigned(map2Dto1D)]->getValue())) continue;

            float successorGivenCost = currentNode->getGivenCost() + (1-tiles[unsigned(map2Dto1D)]->getValue());
            FinderNode* oldNode = vec[unsigned(map2Dto1D)];
            if (oldNode != nullptr)
            {
                if (successorGivenCost >= oldNode->getGivenCost()) continue;
                oldNode->setParentNode(currentNode);
                oldNode->setGivenCost(successorGivenCost);
                oldNode->setFinalCost(calcFinalCost(oldNode->getGivenCost(), oldNode->getHeuristicCost(), weight));
                if (oldNode->getClosed())
                {
                    oldNode->setClosed(false);
                    openQueue.push(oldNode);
                } else
                {
                    // If in open and final cost changes, priority should be updated
                    // Just reinsert the node
                    // It has to be processed multiple times but is still considerably
                    // faster and easier than other methods
                    openQueue.push(oldNode);
                }
            } else
            {
                FinderNode* successor = new FinderNode(tiles[unsigned(map2Dto1D)], currentNode);

                successor->setHeuristicCost(calcHeuristic( successorX, goalX, successorY, goalY));
                successor->setGivenCost(successorGivenCost);
                successor->setFinalCost(calcFinalCost(successor->getGivenCost(), successor->getHeuristicCost(), weight));
                openQueue.push(successor);
                vec[unsigned(map2Dto1D)] = successor;
            }

         }

        currentNode->setClosed(true);
    }

    for (auto &g : vec)
    {
        delete g;
    }
    vec.clear();

    return pathFound;

}

const std::vector<std::shared_ptr<Tile> > &Pathfinder::getSolutionList() const
{
    return solutionList;
}

inline float Pathfinder::calcHeuristic(int x1, int x2, int y1, int y2)
{
    return sqrt(float(((x2-x1))*((x2-x1)))+(((y2-y1))*((y2-y1))));
}

inline float Pathfinder::calcFinalCost(float givenCost, float heuristicCost, float weight)
{
    return givenCost + weight * heuristicCost;
}
















