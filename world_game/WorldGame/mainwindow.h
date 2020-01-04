#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gamemodel.h"
#include "graphicsview.h"
#include "terminalview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void startGame();
    /*
     * wheel event for zooming in,
     * keypressevent for controlling prot in graphics view
     */
    void wheelEvent( QWheelEvent * event );
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
private slots:
    void on_visualizationTabWidget_currentChanged(int index);

    void on_mapFileDialogButton_clicked();

    void on_generateWorldButton_clicked();

    void gameOver(bool result);

    void on_autoRunnerButton_clicked();

    void on_testerFindButton_clicked();

    void setRunSpeed(int value);

    void deadEndMapError();

    void dispPathfinderTestResult(bool pathFound, QString costTimeMessage);

signals:
    void startTable(const std::vector<std::shared_ptr<Tile>> tiles,
                    int rows, int cols, std::shared_ptr<const Protagonist> prot,
                    const std::vector<std::shared_ptr<HealthPack>> healthpacks,
                    const std::vector<std::shared_ptr<Enemy>> enemies,
                    std::vector<std::shared_ptr<Tile>> poisonTiles);
    void updateTable(int x, int y);
    void zoomIn(int steps);
    void moveInGraphicsView(MoveDirection direction);
    void controlKeyPress(bool status);
    void controlKeyReleased(bool status);

private:
    Ui::MainWindow *ui;

    std::shared_ptr<GameModel> game;
    std::shared_ptr<GraphicsView> graphicalView;
    std::unique_ptr<TerminalView> terminalView;

    bool gameStarted;
};
#endif // MAINWINDOW_H
