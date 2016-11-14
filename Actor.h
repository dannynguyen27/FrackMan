#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include <vector>

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;

const double DISCOVER_RANGE = 3.0;
const double VISIBLE_RANGE = 4.0;

const int SQUIRT_DAMAGE = 2;
const int PROTESTER_ANNOY_DAMAGE = 2;
const int SQUIRTS_PER_WATER = 5;
const int MAX_TRAVELING_DISTANCE = 10;
const int FOUND_WATER_POINTS = 100;
const int FOUND_SONAR_POINTS = 75;
const int FOUND_GOLD_POINTS = 10;
const int REGULARPROTESTER_HEALTH = 5;
const int HARDCOREPROTESTER_HEALTH = 20;
const int BOULDER_POINTS_OF_ANNOYANCE = 100;

const int SONAR_RADIUS = 12;
const int BOULDER_HIT_PROTESTER_POINTS = 500;
const int SQUIRT_KILL_PROTESTER_POINTS = 100;
const int SQUIRT_KILL_HC_PROTESTER_POINTS = 250;
const int BRIBED_REG_PROTESTER_POINTS = 25;
const int BRIBED_HC_PROTESTER_POINTS = 50;
///////////////////////////////////////////////////////////////////////////
//  Actor Class
///////////////////////////////////////////////////////////////////////////
class Actor : public GraphObject
{
public:
    Actor(StudentWorld* gp, int imageID, int startX, int startY, Direction startDirection, float size = 1, unsigned int depth = 0);
    virtual ~Actor();
    
    // Checks whether an actor object is still alive
    bool isAlive() const;
    
    // Sets the actor object to dead
    void setDead();
    
    // Actions per tick
    virtual void doSomething() = 0;
    
    // Returns the world that the actor is placed within
    StudentWorld* getWorld() const;
    
    // Annoys the actor
    virtual void getAnnoyed(int annoyingFactor);
    
    // Checks to see if the actor can move to a specific location
    bool canMove(int x, int y) const;
    
    // Checks to see if you can move in a specific cardinal direction
    bool canMoveSouth() const;
    bool canMoveEast() const;
    bool canMoveNorth() const;
    bool canMoveWest() const;
    
    // Checks to see if the actor is one of two particular classes
    virtual bool isBoulder() const;
    virtual bool isProtester() const;
    
private:
    StudentWorld* m_world;
    bool lifeStatus;
};

///////////////////////////////////////////////////////////////////////////
//  GameCharacter Class
///////////////////////////////////////////////////////////////////////////
class GameCharacter : public Actor
{
public:
    GameCharacter(StudentWorld* gp, int imageID, int startX, int startY, Direction startDirection, int startHealth);
    void decreaseHealth(int amtToLose);
    int getHealth() const;
private:
    int m_health;
};

///////////////////////////////////////////////////////////////////////////
//  Protester Class
///////////////////////////////////////////////////////////////////////////
class Protester : public GameCharacter
{
public:
    Protester(StudentWorld* gp, int imageID, int startHealth, int startX = 60,  int startY = 60, Direction startDirection = left);
    bool checkIfFacingFrackMan();
    bool canPerformHorizontalOrVerticalAction();
    bool canTurnCorner();
    void turnCorner();
    void switchDirection();
    void moveProtester();
    void getAnnoyed(int annoyingFactor);
    void doSomething();
    bool isLeavingField() const;
    bool isProtester() const;
    void setLeaveInState(bool shouldILeave);
    void setStunned(int amtStunned);
    virtual bool isHardCore();
    virtual void getBribed() = 0;

private:
    virtual bool canDoSomethingDifferent();
    
    int ticksAlive;
    int ticksNeededToWait;
    
    bool hasShoutedLast15Ticks;
    int ticksSinceLastShout;
    
    bool hasMadePerpendicularTurnLast200;
    int ticksSinceLastPerpendicular;
    
    bool isInLeaveTheOilState;
    int ticksStunned;
    int numSquaresToMoveInCurrentDirection;
    
};

///////////////////////////////////////////////////////////////////////////
//  FrackMan Class
///////////////////////////////////////////////////////////////////////////
class FrackMan : public GameCharacter
{

public:
    FrackMan(StudentWorld* gp);
    
    virtual void doSomething();
    virtual void getAnnoyed(int annoyingFactor);
    
    ~FrackMan();
    
    void FrackRemoveDirt(int x, int y);

    int getGold() const;
    int getSonarCharges() const;
    int getSquirts() const;
    
    void addGold();
    
    void addSonarCharges();
    
    void addWater();
private:
    int m_gold;
    int m_sonarCharges;
    int m_squirts;
};


///////////////////////////////////////////////////////////////////////////
//  Dirt Class
///////////////////////////////////////////////////////////////////////////
class Dirt : public Actor
{
public:
    Dirt(StudentWorld* gp, int startX, int startY);
    virtual void doSomething();
    ~Dirt();
private:
};

///////////////////////////////////////////////////////////////////////////
//  RegularProtester Class
///////////////////////////////////////////////////////////////////////////
class RegularProtester : public Protester
{
public:
    RegularProtester(StudentWorld* gp, int imageID = IID_PROTESTER, int startHealth = REGULARPROTESTER_HEALTH);
    virtual void getBribed();
private:
};

///////////////////////////////////////////////////////////////////////////
//  HardCoreProtester Class
///////////////////////////////////////////////////////////////////////////
class HardcoreProtester : public Protester
{
public:
    HardcoreProtester(StudentWorld* gp);
    virtual void getBribed();
    virtual bool isHardCore();
private:
    virtual bool canDoSomethingDifferent();
};

///////////////////////////////////////////////////////////////////////////
//  Squirt Class
///////////////////////////////////////////////////////////////////////////
class Squirt : public Actor
{
public:
    Squirt(StudentWorld* gp, int startX, int startY, Direction temp);
    void doSomething();
private:
    int m_travelDistance;
};

///////////////////////////////////////////////////////////////////////////
//  Goodie Class
///////////////////////////////////////////////////////////////////////////
class Goodie : public Actor
{
public:
    Goodie(StudentWorld* gp, int imageID, int startX, int startY, Direction direc, float size = 1, unsigned int depth = 2);

private:
};

///////////////////////////////////////////////////////////////////////////
//  WaterPool Class
///////////////////////////////////////////////////////////////////////////
class WaterPool : public Goodie
{
public:
    WaterPool(StudentWorld* gp, int startX, int startY);
    virtual void doSomething();
private:
    int ticksWaited;
};

///////////////////////////////////////////////////////////////////////////
//  SonarKit Class
///////////////////////////////////////////////////////////////////////////
class SonarKit : public Goodie
{
public:
    SonarKit(StudentWorld* gp);
    virtual void doSomething();
private:
    int ticksWaited;
};

///////////////////////////////////////////////////////////////////////////
//  GoldNugget Class
///////////////////////////////////////////////////////////////////////////
class GoldNugget : public Goodie
{
public:
    GoldNugget(StudentWorld*gp, int startX, int startY);
    
    void setDropped();
    
    virtual void doSomething();
    
private:
    int ticksWaited;
    bool m_droppedByFrackMan;
};

///////////////////////////////////////////////////////////////////////////
//  Boulder Class
///////////////////////////////////////////////////////////////////////////
class Boulder : public Actor
{
    const int MAX_TICKS_WAITED = 30;
    const int BOULDER_ANNOY = 100;
public:
    Boulder(StudentWorld* gp, int startX, int startY);
    
    virtual void doSomething();
    
    bool isBoulder() const;
    
private:
    std::string m_state;
    int ticksWaited;
};

///////////////////////////////////////////////////////////////////////////
//  OilBarrel Class
///////////////////////////////////////////////////////////////////////////
class OilBarrel : public Goodie
{
    const int FOUND_OIL = 1000;
public:
    OilBarrel(StudentWorld* gp, int startX, int startY);
    void doSomething();
private:
};


#endif // ACTOR_H_
