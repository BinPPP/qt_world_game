#include "graphicsview.h"

GraphicsView::GraphicsView()
{
    mGraphicsScene = std::make_shared<QGraphicsScene>();
    mPixMap = std::make_shared<QPixmap>();
    controlKeyStatus = false;
    timer = std::make_shared<QTimer>();
    deletTimer = std::make_shared<QTimer>();

    connect(timer.get(), SIGNAL(timeout()), this, SLOT(drawPath()));
    connect(deletTimer.get(), SIGNAL(timeout()), this, SLOT(deletePath()));
    //mGraphicsView->s
}

GraphicsView::GraphicsView(QGraphicsView * view) : mGraphicsView(view)
{
    GraphicsView();
}

void GraphicsView::loadMap(QString mapName)
{
    clearMap();
    if(mPixMap -> load(mapName))
    {
        mGraphicsScene -> addPixmap( * mPixMap.get());
    }

    else
    {
        qDebug() << "the map is not loaded";
    }
    if(!mGraphicsView)
    {
        qDebug() << "the graphics view is not loaded";
    }
    else{
        mGraphicsView -> setScene(mGraphicsScene.get());
        mGraphicsView -> show();
    }

}

void GraphicsView::setGraphicsView(QGraphicsView *graphicsView)
{
    mGraphicsView = graphicsView;
    mGraphicsView -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mGraphicsView -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mGraphicsView -> setDragMode(QGraphicsView::NoDrag);

}

QGraphicsView *GraphicsView::graphicsView() const
{
    return mGraphicsView;
}

void GraphicsView::initGame(GameModel * mGameModel)
{
    protImage = setImage(":/icons/player.png", 0.002);
    protImage -> setPos(mGameModel -> getProtagonist() -> getXPos(), mGameModel -> getProtagonist()->getYPos());
    mGraphicsScene -> addItem(protImage);
    protImage -> setZValue(1);
    //ui->graphicsView->centerOn(protRect);
    int i = 0;
    for (auto &x : mGameModel -> getEnemies())
    {
        if(std::dynamic_pointer_cast<PEnemy>(x))
        {
            qDebug() <<"displaying poison at position " << x -> getXPos() << x -> getYPos();
            enemyImage.push_back(setImage(":/icons/pEnemy.png", 0.00045));
            mGraphicsScene -> addItem(enemyImage[i]);
            enemyImage[i] -> setPos(x -> getXPos(), x -> getYPos());
            enemyImage[i] -> setZValue(1);
            fireTile.push_back(setImage(":/icons/fire.png", 0.00161));
            mGraphicsScene -> addItem(fireTile.back());
            fireTile.back() -> setPos(x -> getXPos(), x -> getYPos());
            //fireTile.back() -> setZValue(1);
            fireTile.back() ->setOpacity(0.5);
        }
        else if(std::dynamic_pointer_cast<EREnemy>(x))
        {
            qDebug() <<"displaying EREnemy at position " << x -> getXPos() << x -> getYPos();
            enemyImage.push_back(setImage(":/icons/erEnemy.png", 0.00128));
            mGraphicsScene -> addItem(enemyImage[i]);
            enemyImage[i] -> setPos(x -> getXPos(), x -> getYPos());
            enemyImage[i] -> setZValue(1);
        }
        else{
            qDebug() <<"displaying normal enemy at position " << x -> getXPos() << x -> getYPos();
            enemyImage.push_back(setImage(":/icons/normalEnemy.png", 0.0017));
            mGraphicsScene -> addItem(enemyImage[i]);
            enemyImage[i] -> setPos(x -> getXPos(), x -> getYPos());
            enemyImage[i] -> setZValue(1);
        }

        i++;

    }
    i = 0;
    for (auto &x : mGameModel -> getHealthPacks())
    {
        qDebug() <<"displaying health pack at position " << x -> getXPos() << x -> getYPos();
        hpPackImage.push_back(setImage(":/icons/healthPack.png", 0.0019));
        mGraphicsScene -> addItem(hpPackImage[i]);
        hpPackImage[i] -> setPos(x -> getXPos(), x -> getYPos());
        hpPackImage[i] -> setZValue(1);
        i++;

    }
}

void GraphicsView::clearMap()
{
    mGraphicsScene->clear();
    if(enemyImage.size() != 0) freeItemVector(enemyImage);
    if(hpPackImage.size() != 0) freeItemVector(hpPackImage);
    if(fireTile.size() != 0) freeItemVector(fireTile);
    qDebug() << "image vectors freed/deleted";
}

void GraphicsView::updateProt(int x, int y)
{
    protImage -> setPos(x, y);
    mGraphicsView -> centerOn(protImage);
    qDebug() << "player is moved to positioin" << protImage -> scenePos();
}

void GraphicsView::setControlKeyStatus(bool value)
{
    controlKeyStatus = value;
    qDebug() << "control key is set to" << value;
}

void GraphicsView::healthPackConsumed(int x, int y)
{
    for(auto &k : hpPackImage)
    {
        qDebug() << "this signal is at" << x << y;
        qDebug() << "the healthPackRect is at" << k -> scenePos();
        if(QPointF(x, y) == k -> scenePos())
        {
            k -> setVisible(false);
            qDebug() << "the health pack is set to invisiable";
            break;
        }
        qDebug() << "the health pack is not found";
    }
}

void GraphicsView::enemyDefeated(int x, int y)
{
    for(auto &k : enemyImage)
    {
        if(k -> scenePos() == QPointF(x, y))
        {
            k -> setVisible(false);
            qDebug() << "the ememy is set to invisiable";
            break;
        }
        qDebug() << "the ememy is not found";
    }
}

void GraphicsView::drawPathTo(std::vector<std::shared_ptr<Tile>> newPathTiles)
{
    qDebug() << "draw path to signal caught";
    if(drawPathFlag){
        for(unsigned int i = 0 ; i< newPathTiles.size(); i++){
            pathRect.push_back(mGraphicsScene->addRect(newPathTiles[i] -> getXPos(), newPathTiles[i] -> getYPos(), 1, 1));
            pathRect[i] -> setVisible(false);
            mGraphicsView->centerOn(protImage);
        }
        drawPathFlag = false;
        timer -> start(animationSpeed);

    }
    else{
        for(unsigned int i = 0 ; i< newPathTiles.size(); i++){
            prevPathRect.push_back(mGraphicsScene->addRect(newPathTiles[i] -> getXPos(), newPathTiles[i] -> getYPos(), 1, 1));
            prevPathRect[i] -> setVisible(false);
            mGraphicsView->centerOn(protImage);
        }
        drawPathFlag = true;
        timer -> start(animationSpeed);
    }


}

void GraphicsView::drawPath()
{
    if(!drawPathFlag){
        if(!prevPathRect.empty())
        {
            deletTimer -> start();
        }
        if(pathItr < pathRect.size()){
            //qDebug() << "draw a single tile signal caught";
            pathRect[pathItr] -> setBrush(Qt::blue);
            pathRect[pathItr]->setPen(QPen(Qt::blue));
            pathRect[pathItr] -> setVisible(true);
            pathRect[pathItr] -> setOpacity(0.5);
            pathItr++;
        }
        else{
            qDebug() << "draw path finished";
            pathItr = 0;
            timer -> stop();
        }
    }
    else{
        if(!pathRect.empty())
        {
            deletTimer -> start();
        }
        if(pathItr < prevPathRect.size()){
            //qDebug() << "draw a single tile signal caught";
            prevPathRect[pathItr] -> setBrush(Qt::blue);
            prevPathRect[pathItr]->setPen(QPen(Qt::blue));
            prevPathRect[pathItr] -> setVisible(true);
            prevPathRect[pathItr] -> setOpacity(0.5);
            pathItr++;
        }
        else{
            qDebug() << "draw path finished";
            pathItr = 0;
            timer -> stop();
        }

    }

}

void GraphicsView::deletePath()
{

    if(drawPathFlag){
        if(!pathRect.empty())
        {
            //qDebug() << "deleting a path tile";
            mGraphicsScene -> removeItem(pathRect[0]);
            delete pathRect.front();
            pathRect.erase(pathRect.begin());
        }
        else{
            qDebug() << "path deleted";
            deletTimer -> stop();
        }
    }
    else{
        if(!prevPathRect.empty())
        {
            //qDebug() << "deleting a path tile";
            mGraphicsScene -> removeItem(prevPathRect[0]);
            delete prevPathRect.front();
            prevPathRect.erase(prevPathRect.begin());
        }
        else{
            qDebug() << "path deleted";
            deletTimer -> stop();
        }
    }

}

void GraphicsView::setAnimationSpeed(int value)
{
    //qDebug() << "animation speed is set to " << 200 - value;
    animationSpeed = 200 - value;
}

void GraphicsView::eREnemyRespawned(int x, int y)
{
    qDebug() <<"displaying respawned EREnemy at position " << x  << y;
    enemyImage.push_back(setImage(":/icons/erEnemy.png", 0.00128));
    mGraphicsScene -> addItem(enemyImage.back());
    enemyImage.back() -> setPos(x, y);
    enemyImage.back() -> setZValue(1);
}

void GraphicsView::drawFireTile(std::vector<std::shared_ptr<Tile> > newPoisonTiles)
{
    for (auto &x : newPoisonTiles)
    {
        qDebug() <<"displaying fire tile at position " << x -> getXPos() << x -> getYPos();
        fireTile.push_back(setImage(":/icons/fire.png", 0.00161));
        mGraphicsScene -> addItem(fireTile.back());
        fireTile.back() -> setPos(x -> getXPos(), x -> getYPos());
        //fireTile.back() -> setZValue(1);
        fireTile.back() ->setOpacity(0.5);
    }
}

void GraphicsView::freeItemVector(std::vector<QGraphicsPixmapItem *> items)
{
    for (auto &x : items)
    {
        delete x;
    }
    items.clear();
}

bool GraphicsView::getControlKeyStatus() const
{
    return controlKeyStatus;
}

int GraphicsView::getAnimationSpeed() const
{
    return animationSpeed;
}


void GraphicsView::setScaleFactor(double value)
{
    scaleFactor = value;
}


void GraphicsView::scaleUp(int steps)
{
    if(controlKeyStatus){
        //qDebug() << "signal catched";

    if(steps > 0)
    {
        h11 = (h11 >= maxFactor) ? h11 : (h11 + scaleFactor);
        h22 = (h22 >= maxFactor) ? h22 : (h22 + scaleFactor);
    }
    else
    {
        h11 = (h11 <= minFactor) ? minFactor : (h11 - scaleFactor);
        h22 = (h22 <= minFactor) ? minFactor : (h22 - scaleFactor);
    }

    mGraphicsView -> setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    mGraphicsView -> setTransform(QTransform(h11, h12, h21, h22, 0, 0));
    mGraphicsView -> centerOn(protImage);
    }
}

QGraphicsPixmapItem * GraphicsView::setImage(QString path, double scale)
{
    QImage qImage;
    qImage.load(path);
    QGraphicsPixmapItem * image = new QGraphicsPixmapItem(QPixmap::fromImage(qImage));
    image -> setScale(scale);
    return image;
}










