#include "StudentWorld.h"
#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <map>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(std::string assetDir)
: GameWorld(assetDir)
{
    myFrackMan = nullptr;
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            dirtObjects[i][j] = nullptr;
            fieldRepresentation[i][j] = '.';
            leavingField[i][j] = Actor::none;
            frackerField[i][j] = Actor::none;
        }
    }
}

StudentWorld::~StudentWorld()
{
    std::vector<Actor*>::iterator itr = activeObjects.begin();
    
    while (itr != activeObjects.end())
    {
        delete (*itr);
        itr = activeObjects.erase(itr);
    }
    
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            delete dirtObjects[i][j];
            dirtObjects[i][j] = nullptr;
        }
    }
    
    delete myFrackMan;
}

void StudentWorld::printFieldRepresentation()
{
    
    for (int i = 0; i < 64; i++)
    {

        for (int j = 0; j < 64; j++)
            cout << frackerField[i][j];
        cout << " " << abs(63 - i);
        cout << endl;
    }
    cout << endl;
}

void StudentWorld::printLeavingField()
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            if (leavingField[i][j] == Actor::up)
                cout << "u";
            else if (leavingField[i][j] == Actor::down)
                cout << "d";
            else if (leavingField[i][j] == Actor::left)
                cout << "l";
            else if (leavingField[i][j] == Actor::right)
                cout << "r";
            else
                cout << ".";
        }
        cout << endl;
    }
    cout << endl;   
    
}
            
void StudentWorld::printFrackerField()
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            if (frackerField[i][j] == Actor::up)
                cout << "u";
            else if (frackerField[i][j] == Actor::down)
                cout << "d";
            else if (frackerField[i][j] == Actor::left)
                cout << "l";
            else if (frackerField[i][j] == Actor::right)
                cout << "r";
            else
                cout << ".";
        }
        cout << endl;
    }
    cout << endl;
    
}
            
int StudentWorld::init()
{
    myFrackMan = new FrackMan(this);
    oilLeftToFind = min(2 + static_cast<int>(getLevel()), 20);
    
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            dirtObjects[i][j] = nullptr;
            fieldRepresentation[i][j] = '.';
        }
    }
    ticksSinceLastProtesterAdded = max(25, 200 - static_cast<int>(getLevel()));
    numProtesters = 0;
    
    addDirt();
    addBoulder();
    addOil();
    addWaterOrSonar();
    addGoldNuggets();
    addProtesters();
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    setDisplayText();
    
    addProtesters();
    addWaterOrSonar();
    
    myFrackMan->doSomething();
    
    
    std::vector<Actor*>::iterator itr = activeObjects.begin();
    while (itr != activeObjects.end())
    {
        if (!(*itr)->isAlive())
        {
            delete (*itr);
            (*itr) = nullptr;
            itr = activeObjects.erase(itr);
        }
        
        else
        {
            (*itr)->doSomething();
            if (!myFrackMan->isAlive())
                return GWSTATUS_PLAYER_DIED;
            
            else if (oilLeftToFind == 0)
                return GWSTATUS_FINISHED_LEVEL;
            
            itr++;
        }
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    delete myFrackMan;
    
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            delete dirtObjects[i][j];
            dirtObjects[i][j] = nullptr;
        }
    }
    
    std::vector<Actor*>::iterator itr = activeObjects.begin();
    
    while (itr != activeObjects.end())
    {
        delete (*itr);
        itr = activeObjects.erase(itr);
    }
}

void StudentWorld::decNumOilLeft()
{
    oilLeftToFind--;
}

bool StudentWorld::isBoulderBeneath(int x, int y) const
{
    vector<Actor*>::const_iterator itr = activeObjects.begin();
    
    while (itr != activeObjects.end())
    {
        if ((*itr)->isBoulder())
        {
            // Need to check for overlap
            int boulderX = (*itr)->getX();
            int boulderY = (*itr)->getY();
            
            if ((x >= boulderX && x < boulderX + 3) && y == boulderY + 3)
                return true;
            else if ((x >= boulderX - 3 && x <= boulderX) && y == boulderY + 3)
            {
                return true;
            }
        }
        itr++;
    }

    return false;
}

bool StudentWorld::removeDirtAt(int x, int y)
{
    if (dirtObjects[x][y] == nullptr)
        return false;
    
    delete dirtObjects[x][y];
    dirtObjects[x][y] = nullptr;
    
    fieldRepresentation[63 - y][x] = '.';
    return true;
}

double StudentWorld::distanceFromFracker(int x, int y) const
{
    int frackX = myFrackMan->getX();
    int frackY = myFrackMan->getY();
    int xDistance = x - frackX;
    int yDistance = y - frackY;
    
    return sqrt((xDistance * xDistance) + (yDistance * yDistance));
}

double StudentWorld::distanceGeneral(int x1, int y1, int x2, int y2) const
{
    int xDistance = x1 - x2;
    int yDistance = y1 - y2;
    
    return sqrt((xDistance * xDistance) + (yDistance * yDistance));
}

bool StudentWorld::canSpawn(int x, int y) const
{
    vector<Actor*>::const_iterator itr;
    bool canSpawnObject = true;
    
    for (itr = activeObjects.begin(); itr != activeObjects.end(); itr++)
    {
        if (distanceGeneral(x, y, (*itr)->getX(), (*itr)->getY()) <= SPAWNING_DISTANCE)
        {
            canSpawnObject = false;
            break;
        }
    }
    
    return canSpawnObject;
}

int StudentWorld::getFrackX() const
{
    return myFrackMan->getX();
}

int StudentWorld::getFrackY() const
{
    return myFrackMan->getY();
}

bool StudentWorld::dirtAt(int x, int y)
{
    // Outside the boundaries of the field
    if (x >= 64 || x < 0 || y >= 64 || y < 0)
        return false;
    
    if (dirtObjects[x][y] == nullptr)
        return false;
    else
        return true;
}

bool StudentWorld::dirtOrBoulderAt(int x, int y)
{
    if (dirtAt(x, y) || boulderAt(x, y))
    {
        return true;
    }
    return false;
}

// Returns true if there is at least one dirt or boulder within the cell
bool StudentWorld::dirtOrBoulderInCell(int x, int y)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (dirtOrBoulderAt(x + i, y + j))
                return true;
        }
    }
    return false;
}

// Returns true if and only the cell contains dirt or boulders
bool StudentWorld::allDirtOrBoulderInCell(int x, int y)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (!dirtOrBoulderAt(x + i, y + j))
                return false;
        }
    }
    return true;
}

bool StudentWorld::dirtOrBoulderNorth(int x, int y)
{
    if(!dirtOrBoulderAt(x, y + 1 + 3) && !dirtOrBoulderAt(x + 1, y + 1 + 3) && !dirtOrBoulderAt(x + 2, y + 1 + 3) && !dirtOrBoulderAt(x + 3, y + 1 + 3))
        return false;
    else
        return true;
}



bool StudentWorld::dirtOrBoulderSouth(int x, int y)
{
    if (!dirtOrBoulderAt(x, y - 1) && !dirtOrBoulderAt(x + 1, y - 1) && !dirtOrBoulderAt(x + 2, y - 1) && !dirtOrBoulderAt(x + 3, y - 1))
        return false;
    else
        return true;
}

bool StudentWorld::dirtOrBoulderWest(int x, int y)
{
    if (!dirtOrBoulderAt(x - 1, y) && !dirtOrBoulderAt(x - 1, y + 1) && !dirtOrBoulderAt(x - 1, y + 2) && !dirtOrBoulderAt(x - 1, y + 3))
        return false;
    else
        return true;
}

bool StudentWorld::dirtOrBoulderEast(int x, int y)
{
    if (!dirtOrBoulderAt(x + 1 + 3, y) && !dirtOrBoulderAt(x + 1 + 3, y + 1) && !dirtOrBoulderAt(x + 1 + 3, y + 2) && !dirtOrBoulderAt(x + 1 + 3, y + 3))
        return false;
    else
        return true;
}


bool StudentWorld::boulderAt(int x, int y)
{
    vector<Actor*>::iterator itr = activeObjects.begin();
    
    while (itr != activeObjects.end())
    {
        if ((*itr)->isBoulder())
        {
            if ((*itr)->getX() == x && (*itr)->getY() == y)
            {
                return true;
            }
        }
        
        itr++;
    }
    return false;
}

void StudentWorld::addDirt()
{
    // Rows
    for (int i = 0; i < 60; i++)
    {
        // Columns
        for (int j = 0; j < 64; j++)
        {
            if ((j == 30 || j == 31 || j == 32 || j == 33) && (i != 0 && i != 1 && i != 2 && i != 3))
                continue;
            
            else
            {
                Dirt* nextDirt = new Dirt(this, j, i);
                dirtObjects[j][i] = nextDirt;
                
                int temp = abs(i - 63);
                fieldRepresentation[temp][j] = 'x';
            }
        }
    }
}
// Adds boulders to the StudentWorld game
void StudentWorld::addBoulder()
{
    int level = getLevel();
    int numBoulders = min(level / 2 + 2, 6);
    
    int bouldersCreated = 0;
    
    while (bouldersCreated < numBoulders)
    {
        // Generate random numbers to spawn boulders
        int randX = rand() % 61;
        int randY = rand() % 37 + 20;
    
        // Invalid spawn location
        if (!allDirtOrBoulderInCell(randX, randY))
            continue;
        if (!canSpawn(randX, randY))
            continue;
        
        Boulder* temp = new Boulder(this, randX, randY);
        activeObjects.push_back(temp);
        bouldersCreated++;
    }
}

void StudentWorld::generateSquirt(Actor::Direction path, int x, int y)
{
    if (dirtAt(x, y) || boulderAt(x, y))
    {
        return;
    }

    // UP
    if (path == 1)
    {
        for (int i = 0; i <= 3; i++)
        {
            for (int j = 0; j <= 4; j++)
            {
                if (boulderAt(x + i, y + j) || dirtAt(x + i, y + j))
                {
                    return;
                }
            }
        }
    }
    // DOWN
    if (path == 2)
    {
        for (int i = 0; i <= 3; i++)
        {
            for (int j = 0; j <= 4; j++)
            {
                if (boulderAt(x + i, y - j) || dirtAt(x + i, y - j))
                {
                    return;
                }
            }
        }
    }
    // Left
    if (path == 3)
    {
        for (int i = 0; i <= 3; i++)
        {
            for (int j = 0; j <= 4; j++)
            {
                if (boulderAt(x - i, y + j) || dirtAt(x - i, y + j))
                {
                    return;
                }
            }
        }
    }
    
    // RIGHT
    if (path == 4)
    {
        for (int i = 0; i <= 4; i++)
        {
            for (int j = 0; j <= 3; j++)
            {
                if (boulderAt(x + i, y + j) || dirtAt(x + i, y + j))
                {
                    return;
                }
            }
        }
    }
    Squirt* temp = new Squirt(this, x, y, path);
    activeObjects.push_back(temp);
}

bool StudentWorld::checkForBouldersWithinRadius(int x, int y) const
{
    vector<Actor*>::const_iterator itr = activeObjects.begin();
    
    // Iterate through all acitve objects
    while (itr != activeObjects.end())
    {
        if (!(*itr)->isBoulder())
        {
            itr++;
        }
        else
        {
            int bX = (*itr)->getX();
            int bY = (*itr)->getY();
            
            // Calculate distance between object in vector and this object
            double distance = (*itr)->getWorld()->distanceGeneral(bX, bY, x, y);
            
            if (distance <= DISCOVER_RANGE)
                return false;
            else
                itr++;
        }
    }
    return true;
}

bool StudentWorld::bribeProtester(int x, int y)
{
    vector<Actor*>::const_iterator itr = activeObjects.begin();
    while (itr != activeObjects.end())
    {
        if (!(*itr)->isProtester())
        {
            itr++;
        }
        else
        {
            int bX = (*itr)->getX();
            int bY = (*itr)->getY();
            
            double distance = distanceGeneral(x, y, bX, bY);
            if (distance <= DISCOVER_RANGE)
            {
                Protester* character = static_cast<Protester*>(*itr);
                if (character != nullptr)
                {
                    character->getBribed();
                    return true;
                }
            }
            else
                itr++;
        }
    }
    return false;
}

bool StudentWorld::attackProtestersWithinRadius(int x, int y, int annoyingFactor)
{
    bool hitProtester = false;
    
    vector<Actor*>::const_iterator itr = activeObjects.begin();
    
    while (itr != activeObjects.end())
    {
        if (!(*itr)->isProtester())
        {
            itr++;
        }
        else
        {
            int bX = (*itr)->getX();
            int bY = (*itr)->getY();
            
            double distance = distanceGeneral(bX, bY, x, y);
            
            if (distance <= DISCOVER_RANGE)
            {
                (*itr)->getAnnoyed(annoyingFactor);
                itr++;
                hitProtester = true;
            }
            else
                itr++;
        }
    }
    return hitProtester;
}

// Adds Oil Barrels to the StudentWorld game
void StudentWorld::addOil()
{
    int level = getLevel();
    int numBarrels = min(2 + level, 20);
    
    int barrelsCreated = 0;
    
    while (barrelsCreated < numBarrels)
    {
        // Generate random numbers to spawn boulders
        int randX = rand() % 61;
        int randY = rand() % 37 + 20;
        
        // Invalid spawn location
        if (!allDirtOrBoulderInCell(randX, randY))
            continue;
        if (!canSpawn(randX, randY))
            continue;
        
        OilBarrel* temp = new OilBarrel(this, randX, randY);
        activeObjects.push_back(temp);
        barrelsCreated++;
    }
}

// Adds Water Pools to the StudentWorld game
void StudentWorld::addWaterOrSonar()
{
    int probability = getLevel() * 25 + 300;
    int randInt = rand() % probability + 1;
    
    // There is a 1 in 'probability' chance
    if (randInt == probability)
    {
        // Randint is between 1 and 5
        randInt = rand() % 5 + 1;
        // 1 in 5 chance of spawning sonar kit
        if (randInt == 1)
        {
            SonarKit* temp = new SonarKit(this);
            activeObjects.push_back(temp);
            return;
        }
        // 4 in 5 chance of spawning water pool
        else
        {
            while (true)
            {
                int randX = rand() % 61;
                int randY = rand() % 61;
                
                // Invalid spawn location - Cannot spawn on top of dirt
                if (dirtOrBoulderInCell(randX, randY))
                    continue;
                else
                {
                    WaterPool* temp = new WaterPool(this, randX, randY);
                    activeObjects.push_back(temp);
                    return;
                }
            }
        }
    }
}

void StudentWorld::decNumProtesters()
{
    numProtesters--;
}

void StudentWorld::addProtesters()
{
    int maxProtesters = min(15, 2 + static_cast<int>(1.5 * getLevel()));
    
    int ticksBetweenProtesters = max(25, 200 - static_cast<int>(getLevel()));
    
    if (numProtesters < maxProtesters && ticksSinceLastProtesterAdded >= ticksBetweenProtesters)
    {
        // Probability of spawning hardcore protester
        int probHardCore = min(90, static_cast<int>(getLevel()) * 10 + 30);
        
        // randInt has a value between 1 to 100
        int randInt = rand() % 100 + 1;
        
        // Numbers beteween 1 to probHardCore means spawn HardCoreProtester
        if (randInt <= probHardCore)
        {
            HardcoreProtester* temp = new HardcoreProtester(this);
            activeObjects.push_back(temp);
        }
        else
        {
            RegularProtester* temp = new RegularProtester(this);
            activeObjects.push_back(temp);
        }

        ticksSinceLastProtesterAdded = 0;
        numProtesters++;
        return;
    }
    ticksSinceLastProtesterAdded++;
}


void StudentWorld::addGoldNuggets()
{
    int level = getLevel();
    int numNuggets = max(5 - level / 2, 2);
    
    int nuggetsCreated = 0;
    
    while (nuggetsCreated < numNuggets)
    {
        // Generate random numbers to spawn boulders
        int randX = rand() % 61;
        int randY = rand() % 37 + 20;
        
        // Invalid spawn location
        if (!allDirtOrBoulderInCell(randX, randY))
            continue;
        if (!canSpawn(randX, randY))
            continue;
        
        GoldNugget* temp = new GoldNugget(this, randX, randY);
        activeObjects.push_back(temp);
        nuggetsCreated++;
    }
}

void StudentWorld::addFrackDrop(GoldNugget *temp)
{
    activeObjects.push_back(temp);
}

int StudentWorld::getCurrentScore() const
{
    return getScore();
}

int StudentWorld::getCurrentGameLevel() const
{
    return getLevel();
}

int StudentWorld::getCurrentHealth() const
{
    return myFrackMan->getHealth();
}

int StudentWorld::getNumLivesLeft() const
{
    return getLives();
}

int StudentWorld::getSquirtsLeftInSquirtGun() const
{
    return myFrackMan->getSquirts();
}

int StudentWorld::getPlayerGoldCount() const
{
    return myFrackMan->getGold();
}

int StudentWorld::getPlayerSonarChargeCount() const
{
    return myFrackMan->getSonarCharges();
}

int StudentWorld::getNumberOfBarrelsRemainingToBePickedUp() const
{
    return oilLeftToFind;
}

Actor::Direction StudentWorld::getFrackerDirection()
{
    return myFrackMan->getDirection();
}


void StudentWorld::setDisplayText()
{
    int score = getCurrentScore();
    int level = getCurrentGameLevel();
    int lives = getNumLivesLeft();
    int health = getCurrentHealth();
    int squirts = getSquirtsLeftInSquirtGun();
    int gold = getPlayerGoldCount();
    int sonar = getPlayerSonarChargeCount();
    int barrelsLeft = getNumberOfBarrelsRemainingToBePickedUp();
    
    string statistics = formatString(score, level, lives, health, squirts, gold, sonar, barrelsLeft);
    
    setGameStatText(statistics);
}

string StudentWorld::formatString(int score, int level, int lives, int health, int squirts, int gold, int sonar, int barrelsLeft)
{
    // TODO: CREATE A FUNCTION TO FORMAT TEXT CORRECTLY
    string displayText = "Scr: ";

    // Leading zeroes for score text (Max 8)
    string nextText = "000000";
    string valueGiven = to_string(score);
    nextText = nextText.substr(0, nextText.length() - valueGiven.length()) + valueGiven;
    
    displayText += nextText + "  Lvl: ";
    
    // Leading spaces for level text (Max 2)
    nextText = "  ";
    valueGiven = to_string(level);
    nextText = nextText.substr(0, nextText.length() - valueGiven.length()) + valueGiven;
    
    displayText += nextText + "  Lives: ";
    

    // No leading space for lives text
    nextText = to_string(lives);
    
    displayText += nextText + "  Hlth: ";

    // Leading spaces for health text (Max 3)
    nextText = "   ";
    valueGiven = to_string(health * 10);
    
    nextText = nextText.substr(0, nextText.length() - valueGiven.length()) + valueGiven;
    
    displayText += nextText + "%  Wtr: ";
    
    // Leading spaces for Water text (Max 2)
    nextText = "  ";
    valueGiven = to_string(squirts);
    
    nextText = nextText.substr(0, nextText.length() - valueGiven.length()) + valueGiven;
    
    displayText += nextText + "  Gld: ";
    // Leading spaces for Gold text (Max 2)
    nextText = "  ";
    valueGiven = to_string(gold);
    
    nextText = nextText.substr(0, nextText.length() - valueGiven.length()) + valueGiven;
    
    displayText += nextText + "  Sonar: ";
    
    // Leading spaces for Sonar text (Max 2)
    nextText = "  ";
    valueGiven = to_string(sonar);

    nextText = nextText.substr(0, nextText.length() - valueGiven.length()) + valueGiven;
    
    displayText += nextText + "  Oil Left: ";
    
    // Leading spaces for Oil Left text (Max 2)
    nextText = "  ";
    valueGiven = to_string(barrelsLeft);
    
    nextText = nextText.substr(0, nextText.length() - valueGiven.length()) + valueGiven;
    
    displayText += nextText;
    return displayText;
}

void StudentWorld::activateSonar()
{
    playSound(SOUND_SONAR);
    vector<Actor*>::iterator itr = activeObjects.begin();
    
    while (itr != activeObjects.end())
    {
        if (distanceFromFracker((*itr)->getX(), (*itr)->getY()) <= SONAR_RADIUS)
        {
            (*itr)->setVisible(true);
        }
        itr++;
    }
}

void StudentWorld::annoyFracker(int annoyingFactor)
{
    myFrackMan->getAnnoyed(annoyingFactor);
}

void StudentWorld::addSquirtsToFrackMan()
{
    myFrackMan->addWater();
}

void StudentWorld::addSonarToFrackMan()
{
    myFrackMan->addSonarCharges();
}

void StudentWorld::addGoldToFrackMan()
{
    myFrackMan->addGold();
}

//////////////////////////////////////////////////////////////
//  Extra Stuff - Coordinate
//////////////////////////////////////////////////////////////

Coordinate::Coordinate()
{
    x = -1;
    y = -1;
}

Coordinate::Coordinate(int x1, int y1)
{
    x = x1;
    y = y1;
}

int Coordinate::getX() const
{
    return x;
}

int Coordinate::getY() const
{
    return y;
}

bool Coordinate::operator==(const Coordinate& src)
{
    return (x == src.x && y == src.y);
}

Coordinate& Coordinate::operator=(const Coordinate& src)
{
    if (this == &src)
        return *this;
    else
    {
        x = src.x;
        y = src.y;
        return *this;
    }
}

bool operator<(const Coordinate& point1, const Coordinate& point2)
{
    if (point1.getX() < point2.getX())
    {
        return true;
    }
    if (point1.getX() == point2.getX() && point1.getY() < point2.getY())
    {
        return true;
    }

    return false;
}

bool operator!=(const Coordinate& point1, const Coordinate& point2)
{
    if (point1.getX() != point2.getX())
        return true;
    
    if (point1.getY() != point2.getY())
        return true;
    
    return false;
}

void StudentWorld::generateLeavingField(int x, int y)
{
    // NOTE: Game Coordinate (x, y) is represented in array by:
    // int temp = 63 - y);
    // fieldRepresentation[temp][x] = ".";
    // (3, 60) is exit
    
    int tempX = 63 - y;
    int tempY = x;
    
    string tempMaze[64][64];
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            tempMaze[i][j] = fieldRepresentation[i][j];
        }
    }
    
    isPathSolvable(tempMaze, tempX, tempY, 3, 60, OIL_KEYWORD);
}

void StudentWorld::generateFieldToFracker(int x, int y)
{
    // NOTE: Game Coordinate (x, y) is represented in array by:
    // int temp = 63 - y);
    // fieldRepresentation[temp][x] = ".";
    // (3, 60) is exit
    
    int tempX = 63 - y;
    int tempY = x;
    
    string tempMaze[64][64];
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            tempMaze[i][j] = fieldRepresentation[i][j];
        }
    }
    
    // Convert Fracker coordinates to array coordinates
    int frackX = 63 - getFrackY();
    int frackY = getFrackX();
    
    isPathSolvable(tempMaze, tempX, tempY, frackX, frackY, FRACKER_KEYWORD);
}

// Precondition: sr and sc have been converted from game coordinates to array coordinates
// Helper Function for generateFieldToFracker / generateField to see if maze is solvable
bool StudentWorld::isPathSolvable(string maze[][64], int sr, int sc, int er, int ec, string keyWord)
{
    queue<Coordinate> points;
    
    // Initial checking to make sure parameters are valid
    
    // Incorrect Boundaries
    if (!withinBoundary(sr, sc))
        return false;
    
    // At least one of the corresponding 16 squares has a dirt
    if (dirtOrBoulderInCell(sc, 63 - sr))
    {
        return false;
    }

    // Invalid stating position
    if (maze[sr][sc] != ".")
        return false;
    
    Coordinate startingPoint(sr, sc);
    points.push(startingPoint);
    
    // Mark as discovered
    maze[sr][sc] = 'O';
    
    Coordinate temp(-1, -1);
    map<Coordinate, Coordinate> pointTrail;
    
    while (!points.empty())
    {
        
        // Grab first point
        temp = points.front();
        points.pop();
        
        bool thereIsDirt = false;
        bool inBoundary = true;
        
        
        // Not within valid boundaries
        if (!withinBoundary(temp.getX(), temp.getY()))
        {
            maze[temp.getX()][temp.getY()] = 'O';
            inBoundary = false;
        }
        
        // At least one of the corresponding 16 squares has a dirt
        if (dirtOrBoulderInCell(temp.getY(), 63 - temp.getX()))
        {
            maze[temp.getX()][temp.getY()] = 'O';
            thereIsDirt = true;
        }
        
        // Found pathway - Temp now represents the ending point
        if (temp.getX() == er && temp.getY() == ec)
        {
            // Reconstructing pathway to store correct directions
            
            map<Coordinate, Coordinate>::iterator itr;
            
            itr = pointTrail.find(temp);
            
            vector<Coordinate> holdPath;
            
            // Saves last point
            holdPath.push_back(temp);
            // Saves what the last coordinate is connected to
            holdPath.push_back(itr->second);
            
            while (itr->second != startingPoint)
            {
                // Finds what tempBack is currently connected to
                itr = pointTrail.find(itr->second);
                
                if (itr == pointTrail.end())
                {
                    return false;
                }
                
                else
                {
                    // Stores the path of points in a vector; at the end of the loop, startingPoint should be the last element
                    holdPath.push_back(itr->second);
                }
            }
            
            // Reverse the vector
            reverse(holdPath.begin(),holdPath.end());
            
            Actor::Direction lastDirec = Actor::none;
            
            while (!holdPath.empty())
            {
                // Only point in the array is the end point itself
                if (holdPath.size() == 1 && keyWord == FRACKER_KEYWORD)
                {
                    Coordinate first = holdPath.back();
                    frackerField[first.getX()][first.getY()] = lastDirec;
                    break;
                }
                else if (holdPath.size() == 1 && keyWord == OIL_KEYWORD)
                {
                    Coordinate first = holdPath.back();
                    leavingField[first.getX()][first.getY()] = lastDirec;
                    break;
                }
                
                Coordinate first = holdPath.back();
                holdPath.pop_back();
                Coordinate second = holdPath.back();
                
                if (keyWord == FRACKER_KEYWORD)
                {
                    // left
                    if (first.getY() < second.getY())
                    {
                        
                        frackerField[first.getX()][first.getY()] = Actor::left;
                        lastDirec = Actor::left;
                    }
                    
                    
                    // right
                    else if (first.getY() > second.getY())
                    {
                        frackerField[first.getX()][first.getY()] = Actor::right;
                        lastDirec = Actor::right;
                    }
                    
                    // up
                    else if (first.getX() < second.getX())
                    {
                        frackerField[first.getX()][first.getY()] = Actor::up;
                        lastDirec = Actor::up;
                    }
        
                    // down
                    else if (first.getX() > second.getX())
                    {
                        frackerField[first.getX()][first.getY()] = Actor::down;
                        lastDirec = Actor::down;
                    }
                }
                
                else if (keyWord == OIL_KEYWORD)
                {
                    // left
                    if (first.getY() < second.getY())
                    {
                        leavingField[first.getX()][first.getY()] = Actor::left;
                        lastDirec = Actor::left;
                    }
                    
                    // right
                    else if (first.getY() > second.getY())
                    {
                        leavingField[first.getX()][first.getY()] = Actor::right;
                        lastDirec = Actor::right;
                    }
                    
                    // up
                    else if (first.getX() < second.getX())
                    {
                        leavingField[first.getX()][first.getY()] = Actor::up;
                        lastDirec = Actor::up;
                    }
                    
                    // down
                    else if (first.getX() > second.getX())
                    {
                        leavingField[first.getX()][first.getY()] = Actor::down;
                        lastDirec = Actor::down;
                    }
                }
            }
            return true;
        }
        
        // North
        // If there is no dirt and the point is within boundary, and the next point to move to is within boundary & undiscovered
        if (!thereIsDirt && inBoundary && (temp.getX() - 1 >= 0 && temp.getX() - 1 < 64 && temp.getY() >= 0 && temp.getY() < 64) && maze[temp.getX() - 1][temp.getY()] == ".")
        {
            int newX = temp.getX() - 1;
            int newY = temp.getY();
            Coordinate tempNorth(newX, newY);
            // Adds valid point to queue
            points.push(tempNorth);
            // Marks point as discovered
            maze[newX][newY] = 'O';
            // Maps the next point to the one before, thereby effectively creating a trail
            pointTrail[tempNorth] = temp;
        }
        
        // East
        if (!thereIsDirt && inBoundary && (temp.getX() >= 0 && temp.getX() < 64 && temp.getY() + 1 >= 0 && temp.getY() + 1 < 64) && maze[temp.getX()][temp.getY() + 1] == ".")
        {
            int newX = temp.getX();
            int newY = temp.getY() + 1;
            Coordinate tempEast(newX, newY);
            points.push(tempEast);
            maze[newX][newY] = 'O';
            
            pointTrail[tempEast] = temp;
        }

        // South
        if (!thereIsDirt && inBoundary && (temp.getX() + 1 >= 0 && temp.getX() + 1 < 64 && temp.getY() >= 0 && temp.getY() < 64) && maze[temp.getX() + 1][temp.getY()] == ".")
        {
            int newX = temp.getX() + 1;
            int newY = temp.getY();
            Coordinate tempSouth(newX, newY);
            points.push(tempSouth);
            maze[newX][newY] = 'O';
            
            pointTrail[tempSouth] = temp;
        }
        // West
        if (!thereIsDirt && inBoundary && (temp.getX() >= 0 && temp.getX() < 64 && temp.getY() - 1 >= 0 && temp.getY() - 1< 64) && maze[temp.getX()][temp.getY() - 1] == ".")
        {
            int newX = temp.getX();
            int newY = temp.getY() - 1;
            Coordinate tempWest(newX, newY);
            points.push(tempWest);
            maze[newX][newY] = 'O';
            
            pointTrail[tempWest] = temp;
        }
    }
    return false;
}

void StudentWorld::clearFrackerField()
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
            frackerField[i][j] = Actor::none;
    }
}

// Returns 1 more than maxLegalDistance if not possible to chase FrackMan
int StudentWorld::chaseFracker(int x, int y, int maxLegalDistance)
{
    int stepsTaken = 0;
    
    while (x != getFrackX() && y != getFrackY() && stepsTaken <= maxLegalDistance)
    {
        // Converion to array coordinates
        int tempX = 63 - y;
        int tempY = x;
        
        Actor::Direction tempDirection = frackerField[tempX][tempY];
        
        switch (tempDirection)
        {
            case 0:
                return -1;
                break;
            // Up
            case 1:
                y += 1;
                break;
            // Down
            case 2:
                y -= 1;
                break;
            // Left
            case 3:
                x += 1;
                break;
            // Right
            case 4:
                x -= 1;
                break;
        }
        stepsTaken++;
        if (tempDirection == 0)
            break;
    }
    return stepsTaken;
}

// Precondition: x and y are the game character's coordinates
Actor::Direction StudentWorld::getDirectionFromField(int x, int y, string keyWord)
{
    // Converting to array coordinates
    int tempX = 63 - y;
    int tempY = x;
    
    if (keyWord == OIL_KEYWORD)
        return leavingField[tempX][tempY];
    else if (keyWord == FRACKER_KEYWORD)
    {
        return frackerField[tempX][tempY];
    }
    else
        return Actor::none;
}

bool StudentWorld::withinBoundary(int x, int y)
{
    if (x < 3 || x > 63 || y < 0 || y > 60)
        return false;
    else
        return true;
}