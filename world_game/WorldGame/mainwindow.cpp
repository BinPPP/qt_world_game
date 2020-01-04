#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameStarted = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startGame(){
    auto tiles = game->getTiles();
    auto protag = game->getProtagonist();
    auto healthPacks = game->getHealthPacks();
    auto enemies = game->getEnemies();
    auto poisonTiles = game->getPoisonTiles();
    int rows = game->getMapRows();
    int cols = game->getMapCols();
    qDebug() << "started game terminal";
    emit startTable(tiles, rows, cols, protag, healthPacks, enemies, poisonTiles);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    //qDebug() << "wheel event detected";
    const int degrees = event->delta() / 8;
    int steps = degrees / 15;
    emit zoomIn(steps);
}

// Do something when another tab of the tabwidget was selected
// You can connect/disconnect different signals & slots depending on the tab
// for example
void MainWindow::on_visualizationTabWidget_currentChanged(int index)
{
    qDebug() << "tab: " + QString::number(index);
    if(index == 0  && gameStarted){ //tab is at GraphicsView
        // disconnect terminalview
        disconnect(game->getProtagonist().get(), &Protagonist::posChanged, terminalView.get(), &TerminalView::updateTable);
        disconnect(game->getProtagonist().get(), &Protagonist::energyChanged, terminalView.get(), &TerminalView::energyChanged);
        disconnect(game->getProtagonist().get(), &Protagonist::healthChanged, terminalView.get(), &TerminalView::healthChanged);
        disconnect(game.get(), &GameModel::healthPackTaken, terminalView.get(), &TerminalView::healthpackTaken);
        disconnect(game.get(), &GameModel::enemyDefeated, terminalView.get(), &TerminalView::enemyDead);
        disconnect(game.get(), &GameModel::newTilesPoisoned, terminalView.get(), &TerminalView::poisonRegionChanged);
        disconnect(game.get(), &GameModel::EREnemyRespawned, terminalView.get(), &TerminalView::updateEnemyList);
        disconnect(this->terminalView->getCommand().get(), &Command::protagChanged, this->game.get(), &GameModel::moveProtagonist);

        // connect graphicsview
        connect(this, &MainWindow::zoomIn, graphicalView.get(), &GraphicsView::scaleUp);
        connect(game->getProtagonist().get(), &Protagonist::posChanged, graphicalView.get(), &GraphicsView::updateProt, Qt::QueuedConnection);
        connect(this, &MainWindow::moveInGraphicsView, this->game.get(), &GameModel::moveProtagonist);
        connect(this, &MainWindow::controlKeyPress, graphicalView.get(), &GraphicsView::setControlKeyStatus);
        connect(this, &MainWindow::controlKeyReleased, graphicalView.get(), &GraphicsView::setControlKeyStatus);
        connect(game.get(), &GameModel::healthPackTaken, graphicalView.get(), &GraphicsView::healthPackConsumed);
        connect(game.get(), &GameModel::enemyDefeated, graphicalView.get(), &GraphicsView::enemyDefeated);
        connect(game.get(), &GameModel::newPath, graphicalView.get(), &GraphicsView::drawPathTo);
        connect(ui->speedSlider, &QSlider::valueChanged, graphicalView.get(), &GraphicsView::setAnimationSpeed);
        connect(game.get(), &GameModel::EREnemyRespawned, graphicalView.get(), &GraphicsView::eREnemyRespawned, Qt::QueuedConnection);
        connect(game.get(), &GameModel::newTilesPoisoned, graphicalView.get(), &GraphicsView::drawFireTile);
        qDebug() << "connection made" ;
        ui->errorStatusbar->showMessage("Use CTRL+W/S to zoom in/out and WASD to move");
    }
    else if(index == 1 && gameStarted) //tab is at TerminalView
    {
        connect(this, &MainWindow::startTable, terminalView.get(), &TerminalView::startTable);
        startGame();
        // disconnect from graphicsview
        disconnect(this, &MainWindow::zoomIn, graphicalView.get(), &GraphicsView::scaleUp);
        disconnect(game->getProtagonist().get(), &Protagonist::posChanged, graphicalView.get(), &GraphicsView::updateProt);
        disconnect(this, &MainWindow::moveInGraphicsView, this->game.get(), &GameModel::moveProtagonist);
        disconnect(this, &MainWindow::controlKeyPress, graphicalView.get(), &GraphicsView::setControlKeyStatus);
        disconnect(this, &MainWindow::controlKeyReleased, graphicalView.get(), &GraphicsView::setControlKeyStatus);
        disconnect(game.get(), &GameModel::healthPackTaken, graphicalView.get(), &GraphicsView::healthPackConsumed);
        disconnect(game.get(), &GameModel::enemyDefeated, graphicalView.get(), &GraphicsView::enemyDefeated);
        disconnect(game.get(), &GameModel::newPath, graphicalView.get(), &GraphicsView::drawPathTo);
        disconnect(ui->speedSlider, &QSlider::valueChanged, graphicalView.get(), &GraphicsView::setAnimationSpeed);
        disconnect(game.get(), &GameModel::EREnemyRespawned, graphicalView.get(), &GraphicsView::eREnemyRespawned);
        disconnect(game.get(), &GameModel::newTilesPoisoned, graphicalView.get(), &GraphicsView::drawFireTile);

        // connect to terminalview
        connect(game->getProtagonist().get(), &Protagonist::posChanged, terminalView.get(), &TerminalView::updateTable, Qt::QueuedConnection);
        connect(game->getProtagonist().get(), &Protagonist::energyChanged, terminalView.get(), &TerminalView::energyChanged, Qt::QueuedConnection);
        connect(game->getProtagonist().get(), &Protagonist::healthChanged, terminalView.get(), &TerminalView::healthChanged, Qt::QueuedConnection);
        connect(game.get(), &GameModel::healthPackTaken, terminalView.get(), &TerminalView::healthpackTaken, Qt::QueuedConnection);
        connect(game.get(), &GameModel::enemyDefeated, terminalView.get(), &TerminalView::enemyDead, Qt::QueuedConnection);
        connect(game.get(), &GameModel::newTilesPoisoned, terminalView.get(), &TerminalView::poisonRegionChanged);
        connect(game.get(), &GameModel::EREnemyRespawned, terminalView.get(), &TerminalView::updateEnemyList, Qt::QueuedConnection);
        connect(this->terminalView->getCommand().get(), &Command::protagChanged, this->game.get(), &GameModel::moveProtagonist);
        connect(game.get(), &GameModel::newPath, terminalView.get(), &TerminalView::drawPath);
        qDebug() << "connection made" ;

        ui->errorStatusbar->showMessage("Type help and press enter to show the available commands");
    }
}

// Open a file dialog to select a map image to load
void MainWindow::on_mapFileDialogButton_clicked()
{
    ui->mapURILineEdit->setText(QFileDialog::getOpenFileName(this, tr("Open Image"),
                      QDir::currentPath(), tr("Image Files (*.png *.jpg *.bmp)")));

    // If an image was selected, enable the button to generate the world
    // If nothing was selected leave/set the button disabled
    if (!ui->mapURILineEdit->text().isEmpty())
    {
        ui->generateWorldButton->setEnabled(true);
    } else
    {
        ui->generateWorldButton->setEnabled(false);
    }
}

// Generate a world depending on the chosen map image and parameters
void MainWindow::on_generateWorldButton_clicked()
{
    // Start the game in the graphical view
    ui->visualizationTabWidget->setCurrentIndex(0);
    // Initialize graphical/terminalview
    graphicalView = std::make_shared<GraphicsView>();
    terminalView = std::make_unique<TerminalView>();
    // Load the visualizations into their corresponding widgets (right tab in the tabwidget)
    //ui->visualizationGraphicsView->setScene(graphicalView.get());
    graphicalView -> setGraphicsView(ui->visualizationGraphicsView);
    ui->terminalWidget->setLayout(terminalView.get());

    game = std::make_shared<GameModel>();

    // Generate the world based on the chosen map and parameters
    game->generateWorld(ui->mapURILineEdit->text(),
                        unsigned(ui->nrEnemiesSpinBox->value()),
                        unsigned(ui->nrHealthpacksSpinBox->value()));

    ui->energyBar->setValue(100);
    ui->healthBar->setValue(100);
    ui->testerDestXSpinBox->setRange(0, game->getMapCols()-1);
    ui->testerDestYSpinBox->setRange(0, game->getMapRows()-1);
    ui->testerStartXSpinBox->setRange(0, game->getMapCols()-1);
    ui->testerStartYSpinBox->setRange(0, game->getMapRows()-1);

    // Connect Main UI specific signals to slots
    connect(game->getProtagonist().get(), &Protagonist::energyChanged, ui->energyBar, &QProgressBar::setValue);
    connect(game->getProtagonist().get(), &Protagonist::healthChanged, ui->healthBar, &QProgressBar::setValue);
    connect(game.get(), &GameModel::gameOver, this, &MainWindow::gameOver);
    connect(game.get(), &GameModel::pathFinderMessage, this, &MainWindow::dispPathfinderTestResult);
    connect(ui->speedSlider, &QSlider::valueChanged, this, &MainWindow::setRunSpeed);
    connect(game.get(), &GameModel::deadEndMap, this, &MainWindow::deadEndMapError);

    graphicalView -> loadMap(ui->mapURILineEdit->text());
    graphicalView -> initGame(game.get());
    // Connect the graphical view signal & slots
    // Game is defaut to run on graphical view every time a new game starts
    connect(this, &MainWindow::zoomIn, graphicalView.get(), &GraphicsView::scaleUp);
    connect(game->getProtagonist().get(), &Protagonist::posChanged, graphicalView.get(), &GraphicsView::updateProt, Qt::QueuedConnection);
    connect(this, &MainWindow::moveInGraphicsView, this->game.get(), &GameModel::moveProtagonist);
    connect(this, &MainWindow::controlKeyPress, graphicalView.get(), &GraphicsView::setControlKeyStatus);
    connect(this, &MainWindow::controlKeyReleased, graphicalView.get(), &GraphicsView::setControlKeyStatus);
    connect(game.get(), &GameModel::healthPackTaken, graphicalView.get(), &GraphicsView::healthPackConsumed);
    connect(game.get(), &GameModel::enemyDefeated, graphicalView.get(), &GraphicsView::enemyDefeated);
    connect(game.get(), &GameModel::newPath, graphicalView.get(), &GraphicsView::drawPathTo);
    connect(ui->speedSlider, &QSlider::valueChanged, graphicalView.get(), &GraphicsView::setAnimationSpeed);
    connect(game.get(), &GameModel::EREnemyRespawned, graphicalView.get(), &GraphicsView::eREnemyRespawned, Qt::QueuedConnection);
    connect(game.get(), &GameModel::newTilesPoisoned, graphicalView.get(), &GraphicsView::drawFireTile);
    ui->errorStatusbar->showMessage("Use CTRL+W/S to zoom in/out and WASD to move");

    ui->testerFindButton->setEnabled(true);
    ui->autoRunnerButton->setEnabled(true);
    ui->autoRunPathfinderWeightSpinBox->setEnabled(true);
    gameStarted = true;
}

/*
 * controlling using WASD on keyboard
 * arrow keys reserved for view position control
 *
 */
void MainWindow::keyPressEvent(QKeyEvent * event)
{
        //qDebug() << "Key pressed: " << event->key();
        if(event->key() == Qt::Key_W){
            if(graphicalView -> getControlKeyStatus()){
                emit zoomIn(1);
            }
            else{
                 emit moveInGraphicsView(MoveDirection::UP);
            }


            //qDebug() << "Key pressed: " << event->key();
        }
        else if(event->key() == Qt::Key_S){
            if(graphicalView -> getControlKeyStatus()){
                emit zoomIn(-1);
            }
            else{
                 emit moveInGraphicsView(MoveDirection::DOWN);
            }
        }
        else if(event->key() == Qt::Key_A){
            emit moveInGraphicsView(MoveDirection::LEFT);
            //qDebug() << "Key pressed: " << event->key();
        }
        else if(event->key() == Qt::Key_D){
            emit moveInGraphicsView(MoveDirection::RIGHT);
            //qDebug() << "Key pressed: " << event->key();
        }
        else if(event->key() == Qt::Key_Control){
            emit controlKeyPress(true);
            //qDebug() << "Key pressed: " << event->key();
        }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control){
                emit controlKeyReleased(false);
                //qDebug() << "Key released: " << event->key();
            }
}

// If the game is over, popup a message displaying the outcome (together with trophy or tombstone depending on outcome)
// Give player the option to restart or just quit
void MainWindow::gameOver(bool result)
{
    QString message = "Game is over, you";
    QPixmap resultIcon;
    if(result)
    {
         message += " won!";
         resultIcon.load(":/icons/trophy.png");
    }
    else
    {
         message += " lost.";
         resultIcon.load(":/icons/loss.png");
    }

    QMessageBox gameOverBox;
    gameOverBox.setText(message);
    gameOverBox.setInformativeText("Do you want to restart the game?");
    gameOverBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    gameOverBox.setDefaultButton(QMessageBox::Yes);
    gameOverBox.setIconPixmap(resultIcon);
    int ret = gameOverBox.exec();

    switch(ret)
    {
        case QMessageBox::Yes: // Restart game
            gameStarted = false;
            ui->generateWorldButton->animateClick();
            break;
        case QMessageBox::No: // Quit game
            QApplication::quit();
            break;
        default: // Quit game
            QApplication::quit();
            break;
    }
}

// Automatically run the while game using a strategy and the A* pathfinder
// Runs with the speed set by the speedSlider and the A* weight set by the autoRunPathfinderWeightSpinBox
void MainWindow::on_autoRunnerButton_clicked()
{
    ui->autoRunnerButton->setEnabled(false);
        ui->testerFindButton->setEnabled(false);
        ui->autoRunPathfinderWeightSpinBox->setEnabled(false);
        game->setAutorunSpeed(1200-10*ui->speedSlider->value());
        game->setAutorunWeight(ui->autoRunPathfinderWeightSpinBox->value());
        graphicalView->setAnimationSpeed(ui->speedSlider->value());
        game->autorun();
}

// Starts the pathfinder tester (isolated from game logic, except tile values of course)
// Can be used to check how long it takes to calculate the path on the biggest maze (around 350 ms including freeing memory)
// findTimeIsolatedPath also emits signals to draw the path in the views and display time and cost information
void MainWindow::on_testerFindButton_clicked()
{
    graphicalView->setAnimationSpeed(199);
    ui->testerFindButton->setEnabled(false);
    game->findTimeIsolatedPath(ui->testerStartXSpinBox->value(),
                   ui->testerStartYSpinBox->value(),
                   ui->testerDestXSpinBox->value(),
                   ui->testerDestYSpinBox->value(),
                   float(ui->testerPathFinderWeightSpinBox->value()));
}

// Display the message containing how long it took to find the path and how much energy the path costs
void MainWindow::dispPathfinderTestResult(bool pathFound, QString costTimeMessage)
{
    ui->testerResultLineEdit->setText(costTimeMessage);
    ui->testerFindButton->setEnabled(true);
}

// Set the run speed of the path animation and auto running when the slider is moved
void MainWindow::setRunSpeed(int value)
{
    graphicalView->setAnimationSpeed(value);
    game->setAutorunSpeed(1400-10*value);
}

// Slot to run when signal is received that the provided map has an enemy completely surrounded by impassable tiles
// and hence, is unsolvable
void MainWindow::deadEndMapError()
{
    ui->errorStatusbar->showMessage(tr("Provided map has a dead end (enemy surrounded by impassable tiles), provide a different one!"));
}

