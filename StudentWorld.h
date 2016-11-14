#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include <iostream>
#include "Actor.h"

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class Dirt;
class FrackMan;

struct Coordinate
{
    Coordinate& operator=(const Coordinate& src);
    bool operator==(const Coordinate& src);
    Coordinate();
    Coordinate(int x1, int y1);
    int x;
    int y;
    int getX() const;
    int getY() const;
};

class StudentWorld : public GameWorld
{
public:
    const std::string OIL_KEYWORD = "oil";
    const std::string FRACKER_KEYWORD = "fracker";
    const int SPAWNING_DISTANCE = 6;
    
    StudentWorld(std::string assetDir);
    
    ~StudentWorld();
    
    // Removes the dirt at a specific location
    bool removeDirtAt(int x, int y);

    // Adds dirt to the game
    void addDirt();
    // Adds Boulders to the game
    void addBoulder();
    
    // Checks to see if there are any boulders
    bool checkForBouldersWithinRadius(int x, int y) const;
    
    bool attackProtestersWithinRadius(int x, int y, int annoyingFactor);
    
    // Bribes Protesters
    bool bribeProtester(int x, int y);
    
    // Checks to see if there is a boulder beneath
    bool isBoulderBeneath(int x, int y) const;

    // Adds Oil Barrels to the game
    void addOil();
    
    // Adds Water Pools to the game
    void addWaterOrSonar();

    // Adds regular protesters
    void addProtesters();
    
    // Adds ammunition to FrackMan
    void addSquirtsToFrackMan();
    
    // Generates squirt objects after pressing space
    void generateSquirt(Actor::Direction temp, int x, int y);
    
    // Adds sonar charges to FrackMan
    void addSonarToFrackMan();
    
    // Adds Gold Nuggets to the game
    void addGoldNuggets();
    
    // Gives FrackMan more gold
    void addGoldToFrackMan();
    
    void addFrackDrop(GoldNugget* temp);
    
    void decNumOilLeft();
    
    void decNumProtesters();
    Actor::Direction getFrackerDirection();
    
    // Returns object's distance from FrackMan
    double distanceFromFracker(int x, int y) const;
    double distanceGeneral(int x1, int y1, int x2, int y2) const;
    bool canSpawn(int x, int y) const;
    
    int getFrackX() const;
    
    int getFrackY() const;

    bool withinBoundary(int x, int y);
    
    // Display Text information
    int getCurrentScore() const;
    int getCurrentGameLevel() const;
    int getCurrentHealth() const;
    int getNumLivesLeft() const;
    int getSquirtsLeftInSquirtGun() const;
    int getPlayerGoldCount() const;
    int getPlayerSonarChargeCount() const;
    int getNumberOfBarrelsRemainingToBePickedUp() const;
    std::string formatString(int score, int level, int lives, int health, int squirts, int gold, int sonar, int barrelsLeft);
    
    // Sets Game Text
    void setDisplayText();
    
    // Activates the sonar to display invisible objects
    void activateSonar();
    
    // Annoys FrackMan
    void annoyFracker(int annoyingFactor);
    
    // Checks to see if there is dirt at a certain location
    bool dirtAt(int x, int y);
    
    // Checks to see if there is a boulder at a certain location
    bool boulderAt(int x, int y);
    bool dirtOrBoulderAt(int x, int y);
    bool allDirtOrBoulderInCell(int x, int y);

    // Checks for boulders or dirt in the four cardinal direction
    bool dirtOrBoulderNorth(int x, int y);
    bool dirtOrBoulderSouth(int x, int y);
    bool dirtOrBoulderWest(int x, int y);
    bool dirtOrBoulderEast(int x, int y);
    
    // Checks for dirt or boulder in a 4x4 cell starting at the lower-leftmost corner
    bool dirtOrBoulderInCell(int x, int y);
    
    // Generates the vector field to
    void generateLeavingField(int x, int y);
    Actor::Direction getDirectionFromField(int x, int y, std::string keyWord);
    
    void clearFrackerField();
    void generateFieldToFracker(int x, int y);
    int chaseFracker(int x, int y, int maxDistance);
    bool isPathSolvable(std::string maze[][64], int sr, int sc, int er, int ec, std::string keyWord);
    
    virtual int init();

    virtual int move();

    virtual void cleanUp();

    void printFrackerField();
    void printFieldRepresentation();
    void printLeavingField();
private:
    std::vector<Actor*> activeObjects;
    Dirt* dirtObjects[64][64];
    FrackMan* myFrackMan;
    int oilLeftToFind;
    std::string fieldRepresentation[64][64];
    Actor::Direction leavingField[64][64];
    Actor::Direction frackerField[64][64];
    int ticksSinceLastProtesterAdded;
    int numProtesters;
    
};

#endif // STUDENTWORLD_H_
