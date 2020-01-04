#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H


#include <QPlainTextEdit>
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>
#include <QStandardItem>
#include <memory>
#include <iostream>
#include <QTextStream>
#include <QTableWidget>
#include <QDebug>
#include <QHeaderView>
#include <QSizePolicy>
#include <QScrollBar>
#include <QCompleter>
#include <world.h>
#include <movedirection.h>
#include "healthpack.h"
#include "erenemy.h"

class Command : public QLineEdit
{
    Q_OBJECT
public:
    Command() = default;

    // getter and setter for boolean move: This variable is needed for the view to know if the command: "MOVE" is active
    bool getMove();
    void setMove(bool move);

    // getter and setter for boolean checkOut: This variable is needed for the view to know if the command: "CHECK OUT" is active
    bool getCheckOut() const;
    void setCheckOut(bool value);

    /* getter and setter for integer checkoutIndex: This variable is used to indicate in which stage the command "CHECK OUT" is.
     * STAGE 0: Check out is just activated -> insert X value
     * STAGE 1: Check out has X value -> insert Y value
     * STAGE 2: Check out is visible -> press RETURN to move back to protagonist view
     */
    int getCheckoutIndex() const;
    void setCheckoutIndex(int value);

    // Keypressevent on QLineEdit gets overwritten in case command: "MOVE" or "CHECK OUT" is active
    void keyPressEvent(QKeyEvent *) override;

signals:
    void protagChanged(MoveDirection direction);

private:
    bool move;
    bool checkOut;
    int checkoutIndex;
};
class TerminalView : public QVBoxLayout
{
    Q_OBJECT
public:
    TerminalView();

    // Getter to return the command object (QLineEdit)
    std::shared_ptr<Command> getCommand();

public slots:

    // Basic slots connected to signals from GameModel:
    void healthChanged(int newHealth);
    void energyChanged(int newEnergy);
    void enemyDead(int xEn, int yEn);
    void healthpackTaken(int xhealth, int yhealth);
    void poisonRegionChanged(std::vector<std::shared_ptr<Tile>> newPoisonTiles);
    void drawPath(std::vector<std::shared_ptr<Tile>> newPathTiles);
    void updateEnemyList(int x, int y, float strength, float energyVal);

    // Start Table is only called when the tab is switched to TERMINALVIEW
    void startTable(const std::vector<std::shared_ptr<Tile>> tiles, int rows, int cols, std::shared_ptr<const Protagonist> prot, const std::vector<std::shared_ptr<HealthPack>> healthPacks, const std::vector<std::shared_ptr<Enemy>> enemies, std::vector<std::shared_ptr<Tile>> poisonTiles);
    // UpdateTable is connected to protagonistMoved signal
    void updateTable(int x, int y);
    // UpdateLog is called when a command is entered
    void updateLog();

private:
    std::shared_ptr<QVBoxLayout> innerLayout;
    std::shared_ptr<QTableWidget> table;
    std::shared_ptr<QPlainTextEdit> log;
    std::shared_ptr<Command> command;

    QStringList commands = QStringList() <<
                                            "help"
                                         <<
                                            "move"
                                         <<
                                            "position"
                                         <<
                                            "health"
                                         <<
                                            "energy"
                                         <<
                                            "change table size"
                                         <<
                                            "get nearest enemy"
                                         <<
                                            "get nearest healthpack"
                                         <<
                                            "check out"
                                         <<
                                            "zoom max";

    // Variables Game:
    std::vector<std::shared_ptr<Tile>> tilesTerminal;
    int rowsTerminal;
    int colsTerminal;
    int TERMINAL_HEIGHT;
    int TERMINAL_WIDTH;

    // Variables healthpacks:
    std::vector<std::vector<int>> healthpacksLocations;
    int xClosesthealthpack;
    int yClosestHealthpack;

    // Variables enemies:
    std::vector<std::vector<int>> enemiesLocations;
    std::vector<bool> enemiesDead;
    std::vector<bool> enemiesPoisonous;
    std::vector<bool> enemiesERE;
    int xClosestEnemy;
    int yClosestEnemy;
    std::vector<std::vector<int>> poisonLocations;

    // Variables Protagonist:
    int x;
    int y;
    int health;
    int energy;

    // Variables table:
    int xStart;
    int xEnd;
    int yStart;
    int yEnd;

    // private Functions: used to update the table
    void changeProt(int xNew, int yNew);
    QTableWidgetItem * fillItem(QTableWidgetItem * oldTile, int xTile, int yTile);
    void fillTable(int xFill, int yFill);
    float calculateGrayValue(float grayValue);
    void checkPoison();
    void adjustTableSize();
    void autoFill(int x, int y, bool up, bool right, bool zoom);

    // private functions :calculate closest object using Manhattan distance
    void calculateClosestEnemy();
    void calculateClosestHealthpack();

    // Object initializers
    void setLogStandard();
    void setCommandStandard();
    void setTableStandard();

    // Command-Log variables;
    bool newTableSize;
    bool heightInserted;
    int xZoom;
    int yZoom;

    // Variable for pathfinder
    std::vector<std::vector<int>> pathLocations;
};

#endif // TERMINALVIEW_H
