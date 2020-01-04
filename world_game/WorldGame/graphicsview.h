#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <memory>
#include <QDebug>
#include <QWheelEvent>
#include <QTimeLine>
#include <QTimer>
#include "gamemodel.h"


class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView();
    GraphicsView(QGraphicsView * view);
    void loadMap(QString mapName);
    void setGraphicsView(QGraphicsView *graphicsView);
    //void keyPressEvent( QKeyEvent * event );
    QGraphicsView *graphicsView() const;
    void initGame(GameModel * mGameModel);
    void clearMap();

    void setScaleFactor(double value);

    int getAnimationSpeed() const;
    QGraphicsPixmapItem * setImage(QString path, double scale);



    bool getControlKeyStatus() const;

public slots:
    void scaleUp(int steps);
    void updateProt(int x, int y);
    void setControlKeyStatus(bool value);
    void healthPackConsumed(int x, int y);
    void enemyDefeated(int x, int y);
    void drawPathTo(std::vector<std::shared_ptr<Tile>> newPathTiles);
    void drawPath();
    void deletePath();
    void setAnimationSpeed(int value);
    void eREnemyRespawned(int x, int y);
    void drawFireTile(std::vector<std::shared_ptr<Tile>> newPoisonTiles);


private:
    std::shared_ptr<QGraphicsScene> mGraphicsScene;
    QGraphicsView * mGraphicsView;
    std::shared_ptr<QPixmap>  mPixMap;
    bool controlKeyStatus;
    std::vector<QGraphicsRectItem*> pathRect;
    std::vector<QGraphicsRectItem*> prevPathRect;

    std::shared_ptr<QTimer> timer;
    std::shared_ptr<QTimer> deletTimer;
    int animationSpeed = 200;
    QGraphicsPixmapItem* protImage;
    std::vector<QGraphicsPixmapItem*> hpPackImage;
    std::vector<QGraphicsPixmapItem*> enemyImage;
    std::vector<QGraphicsPixmapItem*> fireTile;
    void freeItemVector(std::vector<QGraphicsPixmapItem*> items);
    bool drawPathFlag = true;



    //zoom in&out settings
    qreal h11 = 1.0;
    qreal h12 = 0;
    qreal h21 = 0;
    qreal h22 = 1.0;
    double scaleFactor = 1.0; //How fast we zoom
    const qreal minFactor = 1.0;
    const qreal maxFactor = 100.0;
    int pathItr = 0;

};

#endif // GRAPHICSVIEW_H
