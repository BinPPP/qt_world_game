#include "terminalview.h"

TerminalView::TerminalView()
{
    innerLayout = std::make_shared<QVBoxLayout>();
    table = std::make_shared<QTableWidget>();
    log = std::make_shared<QPlainTextEdit>();
    command = std::make_shared<Command>();
    this->addLayout(innerLayout.get());
    innerLayout->addWidget(table.get());
    innerLayout->setAlignment(table.get(), Qt::AlignCenter);
    innerLayout->addWidget(log.get());
    innerLayout->addWidget(command.get());
    setLogStandard();
    setCommandStandard();
    setTableStandard();
    innerLayout->setAlignment(Qt::AlignCenter);
    QObject::connect(command.get(), SIGNAL(returnPressed()), this, SLOT(updateLog()));

}

/* TERMINALVIEW:    GETTERS AND SETTERS
 * - getCommand(): get Command object
 */

std::shared_ptr<Command> TerminalView::getCommand()
{
    return command;
}

/* TERMINALVIEW:    SLOTS
 *      1-  healthChanged
 *      2-  energyChanged
 *      3-  enemyDead
 *      4-  healthpackTaken
 *      5-  poisonRegionChanged
 *      6-  drawPath
 *      7-  UpdateEnemyList
 *      8-  startTable
 *      9-  updateTable
 *      10- updateLog
 */

/* 1:   If health changed update newHealth */
void TerminalView::healthChanged(int newHealth)
{
    health = newHealth;
}

/* 2:   If energy changed update newEnergy */
void TerminalView::energyChanged(int newEnergy)
{
    energy = newEnergy;
}

/* 3:   If Enemy is dead update boolean list enemiesDead and visualize in table */
void TerminalView::enemyDead(int xEn, int yEn){
    int index = 0;
    for(auto location: enemiesLocations){
        if(location[0] == xEn && location[1] == yEn){
            enemiesDead[index] = true;
        }
        index++;
    }
    // Check boundary
    if(xEn >= xStart && xEn <= xEnd && yEn >= yStart && yEn <= yEnd){
        QTableWidgetItem * newTile = fillItem(table->takeItem(yEn-yStart,xEn-xStart), xEn,yEn);
        table->setItem(yEn-yStart,xEn-xStart,newTile);
    }
}

/* 4:   If healthpack is taken remove from list */
void TerminalView::healthpackTaken(int xhealth, int yhealth){
    int i = 0;
    int index = 0;
    for(auto location : healthpacksLocations){
        if(location[0] == xhealth && location[1] == yhealth)
            index = i;
        i++;
    }
    healthpacksLocations.erase(healthpacksLocations.begin() + index);
    if(xhealth >= xStart && xhealth <= xEnd && yhealth >= yStart && yhealth <= yEnd){
        QTableWidgetItem * newTile = fillItem(table->takeItem(yhealth-yStart,xhealth-xStart), xhealth,yhealth);
        table->setItem(yhealth-yStart,xhealth-xStart,newTile);
    }
}
/* 5:   Adapt poison region to new poisonous tiles */
void TerminalView::poisonRegionChanged(std::vector<std::shared_ptr<Tile>> newPoisonTiles){
    int poisonTilesSize = newPoisonTiles.size();
    int oldPoisonLocationSize = poisonLocations.size();
    poisonLocations.resize(poisonTilesSize+oldPoisonLocationSize, std::vector<int>(2,0));
    int index = oldPoisonLocationSize;
    for( auto i : newPoisonTiles){
        int column = i->getXPos();
        int row = i->getYPos();
        poisonLocations[index][0] = column;
        poisonLocations[index][1] = row;
        if(column>= xStart && column <= xEnd && row >= yStart && row <= yEnd){
            QTableWidgetItem * newTile = fillItem(table->takeItem(row-yStart, column-xStart), column,row); //new protagonist TableWidgetItem
            table->setItem(row-yStart,column-xStart,newTile);
        }
        index++;
    }
}

/* 6:   Add the new path tiles to the vector with the old path locations */
void TerminalView::drawPath(std::vector<std::shared_ptr<Tile> > newPathTiles)
{
    //Reset items from previous path
    auto oldPath = pathLocations;
    pathLocations.clear();
    for(int i = 0; i <oldPath.size(); i++){
        int column = oldPath[i][0];
        int row = oldPath[i][1];
        if(column>= xStart && column <= xEnd && row >= yStart && row <= yEnd){
            QTableWidgetItem * newTile = fillItem(table->takeItem(row-yStart, column-xStart), column,row);
            table->setItem(row-yStart,column-xStart,newTile);
        }
    }
    // Fill items from new path
    int newSize = newPathTiles.size();
    pathLocations.resize(newSize, std::vector<int>(2,0));
    int index = 0;
    for(auto i: newPathTiles){
        int column = i->getXPos();
        int row = i->getYPos();
        pathLocations[index][0] = column;
        pathLocations[index][1] = row;
        if(column>= xStart && column <= xEnd && row >= yStart && row <= yEnd){
            QTableWidgetItem * newTile = fillItem(table->takeItem(row-yStart, column-xStart), column,row); //new protagonist TableWidgetItem
            table->setItem(row-yStart,column-xStart,newTile);
        }
        index++;
    }
}

/*7:    Update enemy list if EREnemy respawns */
void TerminalView::updateEnemyList(int xE, int yE, float strength, float energyVal){
    bool exists = false;
    int index = 0,loc = 0, ere = 0;

    for(int i = 0; i < enemiesLocations.size(); i++){
        if(enemiesLocations[i][0] == x && enemiesLocations[i][1] == y){
            exists = true;
            loc = index;
        }
        if(enemiesERE[i] && enemiesDead[i]){
            ere = index;
        }
        index++;
    }
    int ereX = enemiesLocations[ere][0];
    int ereY = enemiesLocations[ere][1];
    if(exists){ // replace dead enemy with ere
        enemiesLocations[loc][0] = xE;
        enemiesLocations[loc][1] = yE;
        enemiesDead[loc] = false;
        enemiesPoisonous[loc] = false;
        enemiesERE[loc] = true;
        enemiesERE[ere] = false;
    }
    else{ // replace dead ere with new ere
        enemiesLocations[ere][0] = xE;
        enemiesLocations[ere][1] = yE;
        enemiesDead[ere] = false;
        enemiesPoisonous[ere] = false;
        enemiesERE[ere] = true;
    }
    if(xE >= xStart && xE <= xEnd && yE >= yStart && yE <= yEnd){
        QTableWidgetItem * newTile = fillItem(table->takeItem(yE-yStart,xE-xStart), xE,yE); //new protagonist TableWidgetItem
        table->setItem(yE-yStart,xE-xStart,newTile);
    }
    // clear old ere location
    if(ereX >= xStart && ereX <= xEnd && ereY >= yStart && ereY <= yEnd){
        QTableWidgetItem * newTile = fillItem(table->takeItem(ereY-yStart,ereX-xStart), ereX,ereY); //new protagonist TableWidgetItem
        table->setItem(ereY-yStart,ereX-xStart,newTile);
    }
}

/* 8:   startTable is a slot that is called when the tab is switched to TERMINALVIEW
 *      - Initializes necessary variables for table
 *      - Calls updateTable(x,y) to fill table
 */
void TerminalView::startTable(const std::vector<std::shared_ptr<Tile>> tiles, int rows, int cols, std::shared_ptr<const Protagonist> prot, const std::vector<std::shared_ptr<HealthPack>> healthPacks, const std::vector<std::shared_ptr<Enemy>> enemies, std::vector<std::shared_ptr<Tile>> poisonTiles)
{
    xStart = 0;
    xEnd = 0;
    yStart = 0;
    yEnd = 0;

    // Save world
    tilesTerminal = tiles;
    rowsTerminal = rows;
    colsTerminal = cols;

    // clear enemies and healthpacks locations
    healthpacksLocations.clear();
    enemiesLocations.clear();
    enemiesDead.clear();
    enemiesPoisonous.clear();
    poisonLocations.clear();
    xClosestEnemy = 0;
    yClosestEnemy = 0;
    xClosesthealthpack = 0;
    yClosestEnemy = 0;

    // Save healthpacks locations
    int healthPacksLength = std::count_if(std::begin(healthPacks), std::end(healthPacks),
                                         [&](const std::shared_ptr<HealthPack> &h) { return !h->getTaken() ;});
    healthpacksLocations.resize(healthPacksLength,std::vector<int>(2, 0));
    int index = 0;
    for(auto i : healthPacks){
        if (!i->getTaken())
        {
            healthpacksLocations[index][0] = i->getXPos();
            healthpacksLocations[index][1] = i->getYPos();
            index++;
        }
    }
    // Save enemies locations
    int enemiesSize = enemies.size();
    enemiesLocations.resize(enemiesSize, std::vector<int>(2,0));
    enemiesDead.resize(enemiesSize, false);
    enemiesPoisonous.resize(enemiesSize, false);
    enemiesERE.resize(enemiesSize, false);
    index = 0;
    for(auto i : enemies){
        enemiesLocations[index][0] = i->getXPos();
        enemiesLocations[index][1] = i->getYPos();
        enemiesDead[index] = i->getDefeated();
        if(std::shared_ptr<PEnemy> pe = std::dynamic_pointer_cast<PEnemy>(i)){
            enemiesPoisonous[index] = true;
        }
        if(std::shared_ptr<EREnemy> ere = std::dynamic_pointer_cast<EREnemy>(i)){
            enemiesERE[index] = true;
        }
        index++;
    }
    // Save poisoned tiles locations
    int poisonTilesSize = poisonTiles.size();
    poisonLocations.resize(poisonTilesSize, std::vector<int>(2,0));
    index = 0;
    for( auto i : poisonTiles){
        poisonLocations[index][0] = i->getXPos();
        poisonLocations[index][1] = i->getYPos();
        index++;
    }

    // Save protagonist location
    x = prot->getXPos();
    y = prot->getYPos();
    health = static_cast<int>(prot->getHealth());
    energy = static_cast<int>(prot->getEnergy());
    newTableSize = true;
    autoFill(x,y,false,false, false);
    fillTable(x,y);
}

/* 9:   Slot connected to movement to update table, also called multiple times in terminalview to show change */
void TerminalView::updateTable(int xNew, int yNew)
{
    // Update region boundaries
    if(TERMINAL_HEIGHT <= 10 && TERMINAL_WIDTH <= 10){
        //Update region boundaries
        autoFill(xNew,yNew, false, false, false);
        this->x = xNew;
        this->y = yNew;
        fillTable(xNew,yNew);
    }
    else // Map is to big to rewrite table entirely: Use sections of map and rewrite protagonist only
    {
        if(xNew < xStart || xNew > xEnd || yNew < yStart || yNew > yEnd){
            bool up = false;
            bool right = false;
            if(xNew < x)
                right = true;
            if(yNew < y)
                up = true;
            autoFill(xNew,yNew, up, right, false);
            this->x = xNew;
            this->y = yNew;
            fillTable(xNew,yNew);
        }
        else{
            int xOld = x;
            int yOld = y;
            this->x = xNew;
            this->y = yNew;
            changeProt(xOld,yOld);
        }
    }
}

/* 10:  UpdateLog is a slot that is connected to the returnPressed signal from the QLineEdit::command.
 *      The command input will be checked with a commandlist to return the right signal
 */
void TerminalView::updateLog()
{
    if(command->getMove()){
        log->insertPlainText("move command ended\n");
    }

    if(command->getCheckoutIndex() == 2){
        command->setInputMask("");
        command->setValidator(nullptr);
        autoFill(x,y,false,false, false);
        fillTable(x,y);

    }
    if(command->text() != nullptr)
    {
        QString commandstring = command->text();
        if(!newTableSize && !command->getCheckOut()){
            log->insertPlainText(commandstring + ": \n");
            switch(commands.indexOf(commandstring)){
            case -1: // Not a valid command
                log->insertPlainText(" -> Not a valid command. \n Try using the following commands: \n");
                for(auto i: commands){
                    log->insertPlainText("  -" + i + "\n");
                }
                break;
            case 0: // help
                log->insertPlainText(" -> You can use the following commands: \n");
                for(auto i: commands){
                    log->insertPlainText("  -" + i + "\n");
                }
                break;

            case 1: // move
                command->setMove(true);
                log->insertPlainText(" -> use keys to move or press return to stop \n");
                break;

            case 2: // position
                log->insertPlainText(" ->  X: " + QString::number(x) + " Y: " + QString::number(y) + "\n");
                break;

            case 3: // health
                log->insertPlainText(" -> " + QString::number(health) + "\n");
                break;


            case 4: // energy
                log->insertPlainText(" -> " + QString::number(energy) + "\n");
                break;

            case 5: // change table size
                log->insertPlainText(" -> Insert a new table heigth: \n");
                newTableSize = true;
                break;

            case 6: // get nearest enemy
                calculateClosestEnemy();
                log->insertPlainText(" ->  X: " + QString::number(xClosestEnemy) + " Y: " + QString::number(yClosestEnemy) + "\n");
                break;

            case 7: // get nearest healthpack
                calculateClosestHealthpack();
                log->insertPlainText(" ->  X: " + QString::number(xClosesthealthpack) + " Y: " + QString::number(yClosestHealthpack) + "\n");
                break;

            case 8: // check out
                log->insertPlainText (" -> Insert x value: \n");
                command->setCheckOut(true);
                break;
            case 9: // zoom max
                TERMINAL_WIDTH = colsTerminal;
                TERMINAL_HEIGHT = rowsTerminal;
                log->insertPlainText(" ->  Width: " + QString::number(TERMINAL_WIDTH) + " | Height: " + QString::number(TERMINAL_HEIGHT) + "\n");
                newTableSize = true;
                autoFill(x,y, false, false, true);
                fillTable(x,y);
            }
        }
        else if(newTableSize){
            command->setInputMask("D9");
            QValidator * validator = new QIntValidator(0, 30, this);
            command->setValidator(validator);
            if(heightInserted){
                if(commandstring.toInt() != TERMINAL_WIDTH){
                    if(commandstring.toInt() > colsTerminal){
                        log->insertPlainText(" -> The maximum width is: " + QString::number(colsTerminal) + ". Try again. \n");
                        newTableSize = false;
                    }
                    else{
                        TERMINAL_WIDTH = commandstring.toInt();
                        log->insertPlainText(" -> new table width: " + QString::number(TERMINAL_WIDTH) + "\n");
                        autoFill(x,y, false, false, false);
                        fillTable(x,y);
                    }
                }
                else{
                    log->insertPlainText("!!This was the previous size!! \n");
                    newTableSize = false;
                }
            }
            else{
                if(commandstring.toInt() != TERMINAL_HEIGHT){
                    if(commandstring.toInt() > rowsTerminal){
                        log->insertPlainText(" -> The maximum height is: " + QString::number(rowsTerminal) + ". Try again. \n");
                        newTableSize = false;
                    }
                    else{
                        TERMINAL_HEIGHT = commandstring.toInt();
                        log->insertPlainText(" -> new table height: " + QString::number(TERMINAL_HEIGHT) + "\n");
                        heightInserted = true;
                        log->insertPlainText(" -> Insert a new table width: \n");
                    }
                }
                else{
                    log->insertPlainText("!!This was the previous size!! \n");
                    newTableSize = false;
                }
            }

            command->setInputMask("");
            command->setValidator(nullptr);
        }

        else if(command->getCheckOut()){
            command->setInputMask("D9");
            QValidator * validator;
            switch(command->getCheckoutIndex()){
            case 0:
                validator = new QIntValidator(0, colsTerminal, this);
                command->setValidator(validator);
                xZoom = commandstring.toInt();
                log->insertPlainText(" -> insert new Y value: \n");
                command->setCheckoutIndex(1);
                command->setInputMask("");
                command->setValidator(nullptr);
                break;
            case 1:
                validator = new QIntValidator(0, rowsTerminal, this);
                command->setValidator(validator);
                yZoom = commandstring.toInt();
                log->insertPlainText(" -> new zoom = X: " + QString::number(xZoom) + " Y: " + QString::number(yZoom) + "\n");
                log->insertPlainText("<html><b>press enter to go back to location </b></html>\n");
                command->setCheckoutIndex(2);
                autoFill(xZoom,yZoom, false, false, true);
                fillTable(xZoom,yZoom);
                command->setInputMask("");
                command->setValidator(nullptr);
                break;
            }
        }
        command->clear();
        QScrollBar * bar = log->verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

/*  TERMINALVIEW    PRIVATE FUNCTIONS TABLE
 *  These functions are used to fill the table after updateTable is called
 *      1-  changeProt
 *      2-  fillItem
 *      3-  fillTable
 *      4-  calculateGrayValue
 *      5-  checkPoison
 *      6-  adjustTableSize
 *      7-  autoFill
 */

/* 1:   changeProt handles the switch between the table items on the old and new protagonist position */
void TerminalView::changeProt(int xOld, int yOld){
    QTableWidgetItem * newTile = fillItem(table->takeItem(y-yStart,x-xStart), x,y); //new protagonist TableWidgetItem
    QTableWidgetItem * oldTile = fillItem(table->takeItem(yOld-yStart, xOld-xStart), xOld, yOld);
    table->setItem(y-yStart,x-xStart,newTile);
    table->setItem(yOld-yStart,xOld-xStart,oldTile);
    checkPoison();
}

/* 2:   fillItem handles the text and background of the item depending on its game function (protagonist, enemy, healthpack, poison, wall, path) */
QTableWidgetItem * TerminalView::fillItem(QTableWidgetItem * tile, int xTile, int yTile){
    QString text;
    tile->setBackground(Qt::white);
    // Check if healthpack
    bool healthpack = false;
    for(int i = 0; i < (healthpacksLocations.size()) ; i ++){
        if(healthpacksLocations[i][0] == xTile && healthpacksLocations[i][1] == yTile)
            healthpack = true;
    }
    // Check if enemy
    bool enemy = false;
    bool dead = false;
    bool poisonous = false;
    bool ere = false;
    for(int i = 0; i < (enemiesLocations.size()) ; i ++){
        if(enemiesLocations[i][0] == xTile && enemiesLocations[i][1] == yTile){
            enemy = true;
            dead = enemiesDead[i];
            poisonous = enemiesPoisonous[i];
            ere = enemiesERE[i];
        }
    }
    //check if tile is poisoned
    bool poisoned = false;
    for(int i = 0; i < poisonLocations.size(); i++){
        if(poisonLocations[i][0] == xTile && poisonLocations[i][1] == yTile){
            poisoned = true;
        }
    }
    //check if protagonist
    bool protagonist = false;
    if(xTile == x && yTile == y)
        protagonist = true;

    //check if tile is on path
    bool onPath = false;
    for(int i = 0; i < pathLocations.size(); i++){
        if(pathLocations[i][0] == xTile && pathLocations[i][1] == yTile){
            onPath = true;
        }
    }
    // after all booleans are set, decide the text and background of the item
    if(onPath){ // Lowest privellige
        QColor color(204,255,255); //light blue
        tile->setBackground(color);
    }
    // check if Protagonist is on same tile as enemy
    else if(xTile == x && yTile == y && enemy){
        text = "P|E";
        tile->setBackground(Qt::red);
    }
    // Check if Protagonist
    else if(protagonist){
        text = "P";
        tile->setBackground(Qt::blue);
    }
    // If healthpack
    else if(healthpack){
        text = "H";
        QColor color(0,255,0); //lime
        tile->setBackground(color);
    }
    // If Enemy
    else if(enemy){
        if(dead){
            text = "Dead";
        }
        else {
            text = "E";
        }
        if(xClosestEnemy == xTile && yClosestEnemy == yTile){
            tile->setBackground(Qt::darkYellow);
        }
        else if(poisonous){
            tile->setBackground(Qt::magenta);
        }
        else if(ere){
            QColor color(150,75,0);
            tile->setBackground(color);
        }
        else
            tile->setBackground(Qt::yellow);
    }
    // If tile is poisonous
    if(poisoned && !enemy && !protagonist){
        QColor color;
        if(healthpack)
            color.setRgb(0,128,0);
        else
            color.setRgb(255,20,147);
        tile->setBackground(color);
    }
    // If zoomed on specific location
    else if(xTile == xZoom && yTile == yZoom){
        tile->setBackground(Qt::cyan);
    }
    // Add value of the tile to table item
    float grayValue = calculateGrayValue(tilesTerminal[(yTile*colsTerminal)+xTile]->getValue());
    if(grayValue == 0){
        tile->setBackground(Qt::black);
    }
    text += "(" + QString::number(grayValue) + ")";
    tile->setText(text);
    return tile;
}

/* 3:   fillTable holds the logic to adjust the table data to a certain coordinate (x,y)
 *      This function was seperated to allow the command: "CHECK OUT" to work
 */
void TerminalView::fillTable(int xFill, int yFill){

    // Initialize table
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(TERMINAL_WIDTH);
    bool ready = false;
    int row = 0;
    int column = 0;
    // Initialize fontsize
    int fontSize = innerLayout->contentsRect().width()/(4*table->columnCount());
    // Fill table
    for(auto &tile : tilesTerminal){
        // Initialize positions and vector for lookup
        int xTile = tile->getXPos();
        int yTile = tile->getYPos();
        // Check with boundaries of table
        if(xTile >= xStart && xTile <= xEnd && yTile >= yStart && yTile <= yEnd && !ready){
            // If all columns are filled add a new row
            if(yTile == yStart+row && row <= TERMINAL_HEIGHT){
                table->insertRow(table->rowCount());
                row++;
                column = 0;
            }
            // Initalize item and use fillItem to set text and background
            QTableWidgetItem * item = new QTableWidgetItem("");
            item = fillItem(item,xTile,yTile);
            QFont font;
            font.setPixelSize(fontSize);
            item->setFont(font);
            // Insert item in row (NOTE (int)row start at 1 because of pre-increment)
            table->setItem(row-1, column, item);
            column++;
        }
        // Check if loop has reached the end of the writable table
        if(xTile == xEnd && yTile == yEnd && !ready){
            adjustTableSize();
            qDebug() << "Table should be updated";
            ready = true;
        }
    }
}

/* 4:   Calculate grayvalue of tile */
float TerminalView::calculateGrayValue(float grayValue){
    float grayVal = 0;
    if (qIsInf(grayValue))
    {
        grayVal = 0.0f;
    }
    else {
        grayVal = grayValue*255.0f;
    }
    return grayVal;
}

/* 5:   Check if there is any poison that spread to the visible table items */
void TerminalView::checkPoison(){
    for( auto i : poisonLocations){
        int column = i[0];
        int row = i[1];
        if(column>= xStart && column < xEnd && row >= yStart && row < yEnd){
            QTableWidgetItem * newTile = fillItem(table->takeItem(row-yStart, column-xStart), column,row); //new protagonist TableWidgetItem
            table->setItem(row-yStart,column-xStart,newTile);
        }
    }
}

/* 6:   Adjust the new table items to the table size */
void TerminalView::adjustTableSize(){
    // Initialize headers
    QHeaderView * headerViewH = new QHeaderView(Qt::Horizontal);
    QHeaderView * headerViewV = new QHeaderView(Qt::Vertical);
    QFont fontH = headerViewH->font();
    QFont fontV = headerViewV->font();
    // Get the new section size if needed
    int width = 0;
    int height = 0;
    if(newTableSize){ // new size has been inserted (check size again)
        width = innerLayout->contentsRect().width();
        height = innerLayout->contentsRect().height() - log->contentsRect().height() - command->contentsRect().height();
        newTableSize = false;
        heightInserted = false;
    }
    else{
        width = table->width();
        height = table->height();
    }
    int sectionWidth = width/ table->columnCount();
    int sectionHeight = height/table->rowCount();
    table->setFixedSize(width, height);
    // Adjust headers
    headerViewH->setMaximumSectionSize(sectionWidth);
    headerViewV->setMaximumSectionSize(sectionHeight);
    fontH.setPixelSize(sectionWidth/4);
    fontV.setPixelSize(sectionHeight/4);
    headerViewH->setFont(fontH);
    headerViewV->setFont(fontV);
    headerViewH->setSectionResizeMode(QHeaderView::Stretch);
    headerViewV->setSectionResizeMode(QHeaderView::Stretch);
    table->setHorizontalHeader(headerViewH);
    table->setVerticalHeader(headerViewV);
}

/* 7:   Recalculate the boundaries of the table */
void TerminalView::autoFill(int x, int y, bool up, bool left, bool zoom){
    if(x < TERMINAL_WIDTH){
        xStart = 0;
        xEnd = TERMINAL_WIDTH-1;
    }
    else if( x >= colsTerminal - TERMINAL_WIDTH-1){
        xStart = colsTerminal-TERMINAL_WIDTH;
        xEnd = colsTerminal-1;
    }
    else{
        if(zoom){
            xStart = x -TERMINAL_WIDTH/2;
            xEnd = x + TERMINAL_WIDTH/2+1;
        }
        else if(left){
            xStart = x - TERMINAL_WIDTH-1;
            xEnd = x+1;
        }
        else{
            xStart = x;
            xEnd = x + TERMINAL_WIDTH-1;
        }
    }
    if(y < TERMINAL_HEIGHT){
        yStart =  0;
        yEnd = TERMINAL_HEIGHT-1;
    }
    else if(y >=  rowsTerminal-TERMINAL_HEIGHT-1){
        yStart = rowsTerminal - TERMINAL_HEIGHT;
        yEnd = rowsTerminal-1;
    }
    else{
        if(zoom){
            yStart = y -TERMINAL_HEIGHT/2;
            yEnd = y + TERMINAL_HEIGHT/2+1;
        }
        else if(up){
            yStart = y+TERMINAL_HEIGHT-1;
            yEnd = y+1;
        }
        else{
            yStart = y;
            yEnd = y+ TERMINAL_HEIGHT -1;
        }
    }
}

/* TERMINALVIEW     PRIVATE FUNCTIONS NEAREST CALCULATION
 * these functions are used to calculate the nearest object and display it if possible
 *      1-  calculateClosestEnemy
 *      2-  calculateClosestHealthpack
 */
void TerminalView::calculateClosestEnemy(){
    int manhattanDistance = rowsTerminal+colsTerminal;
    for(int i = 0; i < enemiesLocations.size(); i++){

        int xEn = enemiesLocations[i][0];
        int yEn = enemiesLocations[i][1];
        if(manhattanDistance > std::abs(xEn-x) + std::abs(yEn-y) && !enemiesDead[i]){
            manhattanDistance = std::abs(xEn-x) + std::abs(yEn-y);
            xClosestEnemy = xEn;
            yClosestEnemy = yEn;
        }
    }
    if(xClosestEnemy >= xStart && xClosestEnemy <= xEnd && yClosestEnemy >= yStart && yClosestEnemy <= yEnd){
        QTableWidgetItem * newTile = fillItem(table->takeItem(yClosestEnemy-yStart,xClosestEnemy-xStart), xClosestEnemy,yClosestEnemy);
        table->setItem(yClosestEnemy-yStart,xClosestEnemy-xStart,newTile);
    }
}

void TerminalView::calculateClosestHealthpack(){
    int manhattanDistance = rowsTerminal+colsTerminal;
    for(int i = 0; i < healthpacksLocations.size(); i++){
        int xhealth = healthpacksLocations[i][0];
        int yhealth = healthpacksLocations[i][1];
        if(manhattanDistance > std::abs(xhealth-x) + std::abs(yhealth-y)){
            manhattanDistance = std::abs(xhealth-x) + std::abs(yhealth-y);
            xClosesthealthpack = xhealth;
            yClosestHealthpack = yhealth;
        }
    }
    if(xClosesthealthpack >= xStart && xClosesthealthpack <= xEnd && yClosestHealthpack >= yStart && yClosestHealthpack <= yEnd){
        QTableWidgetItem * newTile = fillItem(table->takeItem(yClosestHealthpack-yStart,xClosesthealthpack-xStart), xClosesthealthpack,yClosestHealthpack);
        table->setItem(yClosestHealthpack-yStart,xClosesthealthpack-xStart,newTile);
    }
}

/* COMMAND  GETTERS AND SETTERS
 *      1-  bool move
 *      2-  bool checkOut
 *      3-  int checkoutIndex
 */

bool Command::getMove()
{
    return move;
}
void Command::setMove(bool move){
    this->move = move;
}

bool Command::getCheckOut() const
{
    return checkOut;
}
void Command::setCheckOut(bool value)
{
    checkOut = value;
}

int Command::getCheckoutIndex() const
{
    return checkoutIndex;
}
void Command::setCheckoutIndex(int value)
{
    checkoutIndex = value;
}


/* COMMAND: OVERRIDE KEYPRESSEVENT OF QLineEdit */
void Command::keyPressEvent(QKeyEvent * event){
    //Check if command "check out" is active
    if(checkOut && checkoutIndex == 2){
        if(event->key() == Qt::Key_Return){
            checkOut = false;
            emit this->returnPressed();
        }
    }
    // Check if command "move" is active
    if(getMove()){
        //qDebug() << "Key pressed: " << event->key();
        if(event->key() == Qt::Key_Up){
            emit protagChanged(MoveDirection::UP);
        }
        else if(event->key() == Qt::Key_Down){
            emit protagChanged(MoveDirection::DOWN);
        }
        else if(event->key() == Qt::Key_Left){
            emit protagChanged(MoveDirection::LEFT);
        }
        else if(event->key() == Qt::Key_Right){
            emit protagChanged(MoveDirection::RIGHT);
        }
        else if(event->key() == Qt::Key_Return)
        {
            emit this->returnPressed();
            this->setMove(false);
        }
    }
    else{
        // default handler for event
        QLineEdit::keyPressEvent(event);
    }
}

/*  TERMINALVIEW     OBJECT INTIALIZERS */
void TerminalView::setLogStandard()
{
    log->setReadOnly(true);
    log->setMinimumHeight(100);
    log->setMaximumHeight(250);
    QScrollBar * bar = log->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void TerminalView::setCommandStandard()
{
    newTableSize = false;
    command->setMove(false);
    command->setCheckOut(false);
    command->setCheckoutIndex(0);
    command->setPlaceholderText("insert command");
    QCompleter * completer = new QCompleter(commands, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    command->setCompleter(completer);
}

void TerminalView::setTableStandard()
{
    xZoom = 0;
    yZoom = 0;
    TERMINAL_HEIGHT = 15;
    TERMINAL_WIDTH = 15;
    QHeaderView * headerViewH = new QHeaderView(Qt::Horizontal);
    QHeaderView * headerViewV = new QHeaderView(Qt::Vertical);
    QFont fontH = headerViewH->font();
    QFont fontV = headerViewV->font();
    fontH.setPointSize(10);
    fontV.setPointSize(10);
    headerViewH->setFont(fontH);
    headerViewV->setFont(fontV);
    headerViewH->setSectionResizeMode(QHeaderView::Stretch);
    headerViewV->setSectionResizeMode(QHeaderView::Stretch);
    headerViewH->setMinimumSectionSize(10);
    table->setHorizontalHeader(headerViewH);
    table->setVerticalHeader(headerViewV);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}




