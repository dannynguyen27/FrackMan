// Version 1.06
#include "Actor.h"
#include "StudentWorld.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp


///////////////////////////////////////////////////////////////////////////
//  Actor implementation
///////////////////////////////////////////////////////////////////////////
Actor::Actor(StudentWorld* gp, int imageID, int startX, int startY, Direction startDirection, float size, unsigned int depth)
: GraphObject(imageID, startX, startY, startDirection, size, depth)
{
    m_world = gp;
    lifeStatus = true;
    setVisible(true);
}

void Actor::setDead()
{
    lifeStatus = false;
}

bool Actor::isAlive() const
{
    return lifeStatus;
}

bool Actor::isBoulder() const
{
    return false;
}

bool Actor::isProtester() const
{
    return false;
}

void Actor::getAnnoyed(int annoyingFactor)
{
    return;
}

// Checks to see if you can move in a specific cardinal direction
bool Actor::canMoveSouth() const
{
    StudentWorld* temp = getWorld();
    
    // No Boulders or Dirt Beneath, and can move to that spot
    if (canMove(getX(), getY() - 1) && !temp->dirtOrBoulderSouth(getX(), getY()))
        return true;
    else
        return false;
}
        
bool Actor::canMoveEast() const
{
    StudentWorld* temp = getWorld();
    
    if (canMove(getX() + 1, getY()) && !temp->dirtOrBoulderEast(getX(), getY()))
        return true;
    else
        return false;
}

bool Actor::canMoveNorth() const
{
    StudentWorld* temp = getWorld();
    
    if (canMove(getX(), getY() + 1) && !temp->dirtOrBoulderNorth(getX(), getY()))
        return true;
    else
        return false;
}
bool Actor::canMoveWest() const
{
    StudentWorld* temp = getWorld();
    
    if (canMove(getX() - 1, getY()) && !temp->dirtOrBoulderWest(getX(), getY()))
        return true;
    else
        return false;
}

// Checkes to see if movement is within boundaries of oil field
bool Actor::canMove(int x, int y) const
{
    if (getWorld()->boulderAt(x, y))
    {
            return false;
    }
    
    // Propsed move is within boundaries
    if (0 <= x && x <= 60 && 0 <= y && y <= 60)
        return true;
    return false;
}

Actor::~Actor()
{
    
}

StudentWorld* Actor::getWorld() const
{
    return m_world;
}

///////////////////////////////////////////////////////////////////////////
//  Dirt implementation
///////////////////////////////////////////////////////////////////////////

Dirt::Dirt(StudentWorld* gp, int startX, int startY)
: Actor(gp, IID_DIRT, startX, startY, right, 0.25, 3)
{
    
}

void Dirt::doSomething()
{
    return;
}

Dirt::~Dirt()
{
    return;
}
///////////////////////////////////////////////////////////////////////////
//  Goodie implementation
///////////////////////////////////////////////////////////////////////////

Goodie::Goodie(StudentWorld* gp, int imageID, int startX, int startY, Direction direc, float size, unsigned int depth)
: Actor(gp, imageID, startX, startY, direc, size, depth)
{
    
}


///////////////////////////////////////////////////////////////////////////
//  Boulder implementation
///////////////////////////////////////////////////////////////////////////
void Boulder::doSomething()
{
    if (!isAlive())
        return;
    
    int x = getX(), y = getY();
    StudentWorld* temp = getWorld();
    
    if (m_state == "stable")
    {
        // Check to see if there is still dirt beneath
        if (temp->dirtAt(x, y - 1) || temp->dirtAt(x +1, y - 1) || temp->dirtAt(x + 2, y - 1) || temp->dirtAt(x + 3, y - 1))
        {
            return;
        }
        else
        {
            m_state = "waiting";
            return;
        }
    }
    
    if (m_state == "waiting")
    {
        if (ticksWaited == MAX_TICKS_WAITED)
        {
            m_state = "falling";
            getWorld()->playSound(SOUND_FALLING_ROCK);
        }
        else
            ticksWaited++;
    }
    
    if (m_state == "falling")
    {
        if (getWorld()->distanceFromFracker(getX(), getY()) <= DISCOVER_RANGE)
        {
            getWorld()->annoyFracker(BOULDER_ANNOY);
        }
        
        getWorld()->attackProtestersWithinRadius(getX(), getY(), BOULDER_POINTS_OF_ANNOYANCE);
        
        if (getWorld()->isBoulderBeneath(x, y - 1))
        {

            m_state = "dead";
            setDead();
            return;
        }
        
        if (canMove(x, y - 1))
            moveTo(x, y -1);
        // Attempting to move outside of oil field boundary
        else
        {
            m_state = "dead";
            setDead();
            return;
        }
        
        y = getY();
        
        // If there is any dirt directly beneath the boulder
        if ((m_state != "dead") && (temp->dirtAt(x, y - 1) || temp->dirtAt(x +1, y - 1) || temp->dirtAt(x + 2, y - 1) || temp->dirtAt(x + 3, y - 1)))
        {
            m_state = "dead";
            setDead();
        }
    }
    
    // TODO: BOULDER HITS SOMETHING
}

bool Boulder::isBoulder() const
{
    return true;
}

Boulder::Boulder(StudentWorld* gp, int startX, int startY)
: Actor(gp, IID_BOULDER, startX, startY, down, 1, 1)
{
    m_state = "stable";
    ticksWaited = 0;
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            getWorld()->removeDirtAt(startX + i, startY + j);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//  OilBarrel implementation
///////////////////////////////////////////////////////////////////////////

OilBarrel::OilBarrel(StudentWorld* gp, int startX, int startY)
: Goodie(gp, IID_BARREL, startX, startY, right)
{
    setVisible(false);
}

void OilBarrel::doSomething()
{
    if (!isAlive())
        return;
    
    // Radius in which oil barrels are obtainable
    if (getWorld()->distanceFromFracker(getX(), getY()) <= DISCOVER_RANGE)
    {
        getWorld()->increaseScore(FOUND_OIL);
        getWorld()->playSound(SOUND_FOUND_OIL);
        getWorld()->decNumOilLeft();
        setDead();
        return;
    }
    
    // Radius in which oil barrels are made visible
    if (getWorld()->distanceFromFracker(getX(), getY()) <= VISIBLE_RANGE)
    {
        setVisible(true);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////
//  GoldNugget implementation
///////////////////////////////////////////////////////////////////////////

GoldNugget::GoldNugget(StudentWorld* gp, int startX, int startY)
: Goodie(gp, IID_GOLD, startX, startY, right)
{
    setVisible(false);
    m_droppedByFrackMan = false;
    ticksWaited = 0;
}

void GoldNugget::setDropped()
{
    setVisible(true);
    m_droppedByFrackMan = true;
}

void GoldNugget::doSomething()
{
    if (!isAlive())
    {
        return;
    }
    
    // Radius in which gold nuggets are obtainable by FrackMan
    if (!m_droppedByFrackMan && getWorld()->distanceFromFracker(getX(), getY()) <= DISCOVER_RANGE)
    {
        getWorld()->addGoldToFrackMan();
        
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(FOUND_GOLD_POINTS);
    }
    
    // Radius in which gold nuggets are made visible
    if (!m_droppedByFrackMan && getWorld()->distanceFromFracker(getX(), getY()) <= VISIBLE_RANGE)
    {
        setVisible(true);
        return;
    }
    
    // If the nugget was originally dropped by FrackMan
    if (m_droppedByFrackMan)
    {
        if (ticksWaited > 100)
        {
            setDead();
        }
        if (getWorld()->bribeProtester(getX(), getY()))
        {
            setDead();
            
        }
        ticksWaited++;
     // TODO: Radius for Protesters to grab gold nugget
    }
}

///////////////////////////////////////////////////////////////////////////
//  SonarKit implementation
///////////////////////////////////////////////////////////////////////////
SonarKit::SonarKit(StudentWorld* gp)
: Goodie(gp, IID_SONAR, 0, 60, right)
{
    ticksWaited = 0;
    setVisible(true);
}

void SonarKit::doSomething()
{
    if (!isAlive())
    {
        return;
    }
    
    // Radius in which sonar kits are obtainable by FrackMan
    if (getWorld()->distanceFromFracker(getX(), getY()) <= DISCOVER_RANGE)
    {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        
        // Increments the total sonar charges
        getWorld()->addSonarToFrackMan();
        getWorld()->increaseScore(FOUND_SONAR_POINTS);
        return;
    }
    int currentLevel = getWorld()->getLevel();
    // Deletes the sonar kit after a certain amount of time
    if (ticksWaited >= min(100, 300 - 10 * currentLevel))
    {
        setDead();
    }
    
    // Increments the number of ticks that this sonar kit has lasted
    ticksWaited++;
}


///////////////////////////////////////////////////////////////////////////
//  WaterPool implementation
///////////////////////////////////////////////////////////////////////////
WaterPool::WaterPool(StudentWorld* gp, int startX, int startY)
: Goodie(gp, IID_WATER_POOL, startX, startY, right)
{
    ticksWaited = 0;
    setVisible(true);
}

void WaterPool::doSomething()
{
    if (!isAlive())
    {
        return;
    }
    
    int currentLevel = getWorld()->getLevel();
    // If the water pool has existed for too long, it will set itself to dead
    if (ticksWaited >= min(100, 300 - 10 * currentLevel))
    {
        setDead();
    }
    
    // If within discoverable range
    if (getWorld()->distanceFromFracker(getX(), getY()) <= DISCOVER_RANGE)
    {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(FOUND_WATER_POINTS);
        
        getWorld()->addSquirtsToFrackMan();
    }
}

///////////////////////////////////////////////////////////////////////////
//  Squirt implementation
///////////////////////////////////////////////////////////////////////////
Squirt::Squirt(StudentWorld* gp, int startX, int startY, Direction temp)
: Actor(gp, IID_WATER_SPURT, startX, startY, temp)
{
    m_travelDistance = 0;
}

void Squirt::doSomething()
{
    // Sets itself dead if it has traveled too much
    if (m_travelDistance >= MAX_TRAVELING_DISTANCE)
    {
        setDead();
    }
    
    // Attacks any and all protesters within a given radius of the squirt object
    if (getWorld()->attackProtestersWithinRadius(getX(), getY(), SQUIRT_DAMAGE))
    {
        setDead();
    }
    
    Direction temp = getDirection();
    // Moves depending on the direction the squirt is facing
    switch (temp)
    {
        case up:
        {
            for (int i = 0; i <= 3; i++)
            {
                // Check one layer after edge of squirt
                for (int j = 0; j <= 4; j++)
                {
                    if (getWorld()->boulderAt(getX() + i, getY() + j) || getWorld()->dirtAt(getX() + i, getY() + j))
                    {
                        setDead();
                        break;
                    }
                }
            }
            moveTo(getX(), getY() +1);
            break;
        }
        case down:
        {
            if (getWorld()->boulderAt(getX(), getY() - 1) || getWorld()->dirtAt(getX(), getY() - 1))
            {
                setDead();
            }
            else
                moveTo(getX(), getY() - 1);
            
            break;
        }
        
        case left:
        {
            if (getWorld()->boulderAt(getX() - 1, getY()) || getWorld()->dirtAt(getX() - 1, getY()))
            {
                setDead();
            }
            else
                moveTo(getX() - 1, getY());
            
            break;
        }
        
        case right:
        {
            // 4 Because checks one additional layer after edge of squirt
            for (int i = 0; i <= 4; i++)
            {
                for (int j = 0; j <= 3; j++)
                {
                    if (getWorld()->boulderAt(getX() + i, getY() + j) || getWorld()->dirtAt(getX() + i, getY() + j))
                    {
                        setDead();
                        break;
                    }
                }
            }
            moveTo(getX() + 1, getY());
            break;
        }
        default:
            break;
    }
    m_travelDistance++;
}

///////////////////////////////////////////////////////////////////////////
//  GameCharacter implementation
///////////////////////////////////////////////////////////////////////////
GameCharacter::GameCharacter(StudentWorld* gp, int imageID, int startX, int startY, Direction startDirection, int startHealth)
: Actor(gp, imageID, startX, startY, startDirection)
{
    m_health = startHealth;
}

void GameCharacter::decreaseHealth(int amtToLose)
{
    m_health -= amtToLose;
}

int GameCharacter::getHealth() const
{
    return m_health;
}

///////////////////////////////////////////////////////////////////////////
//  FrackMan implementation
///////////////////////////////////////////////////////////////////////////

FrackMan::FrackMan(StudentWorld* gp)
: GameCharacter(gp, IID_PLAYER, 30, 60, right, 10)
{
    m_squirts = 5;
    m_sonarCharges = 1;
    m_gold = 0;
}

FrackMan::~FrackMan()
{
    return;
}
void FrackMan::doSomething()
{
    int ch;
    StudentWorld* temp = getWorld();
    if (temp->getKey(ch) == true)
    {
        switch (ch)
        {
            // Directional keys dictate where the FrackMan should Move
            case KEY_PRESS_LEFT:
            {
                if (getDirection() != left)
                {
                    setDirection(left);
                }
                
                else
                {
                    if (canMove(getX() - 1, getY()) && getWorld()->checkForBouldersWithinRadius(getX() - 1, getY()))
                    {
                        FrackRemoveDirt(getX() - 1, getY());
                        moveTo(getX() - 1, getY());
                        getWorld()->clearFrackerField();
                    }
                    
                    else
                    {
                        moveTo(getX(), getY());
                    }
                    
                }
            }
            break;
                
            case KEY_PRESS_RIGHT:
            {
                if (getDirection() != right)
                {
                    setDirection(right);
                }
                
                else
                {
                    if (canMove(getX() + 1, getY()) && getWorld()->checkForBouldersWithinRadius(getX() + 1, getY()))
                    {
                        FrackRemoveDirt(getX() + 1, getY());
                        moveTo(getX() + 1, getY());
                        getWorld()->clearFrackerField();
                    }
                    
                    else
                        moveTo(getX(), getY());
                }
            }
            break;
                
            case KEY_PRESS_DOWN:
            {
                if (getDirection() != down)
                {
                    setDirection(down);
                }
                
                else
                {
                    if (canMove(getX(), getY() - 1) && getWorld()->checkForBouldersWithinRadius(getX(), getY() - 1))
                    {
                        FrackRemoveDirt(getX(), getY() - 1);
                        moveTo(getX(), getY() - 1);
                        getWorld()->clearFrackerField();
                    }
                    else
                        moveTo(getX(), getY());
                }
            }
            break;
                
            case KEY_PRESS_UP:
            {
                if (getDirection() != up)
                {
                    setDirection(up);
                }
                
                else
                {
                    if (canMove(getX(), getY() + 1) && getWorld()->checkForBouldersWithinRadius(getX(), getY() + 1))
                    {
                        FrackRemoveDirt(getX(), getY() + 1);
                        moveTo(getX(), getY() + 1);
                        getWorld()->clearFrackerField();
                    }
                    else
                        moveTo(getX(), getY());
                }
            }
            break;
                
            // Tab allows gold nuggets to appear
            case KEY_PRESS_TAB:
            {
                if (getGold() > 0)
                {
                    GoldNugget* tempDrop = new GoldNugget(getWorld(), getX(), getY());
                    tempDrop->setDropped();
                    getWorld()->addFrackDrop(tempDrop);
                    m_gold--;
                }
            }
            break;
            
            // Squirts will appear if the user presses space
            case KEY_PRESS_SPACE:
            {
                if (getSquirts() <= 0)
                    return;
                m_squirts--;
                Direction temp = getDirection();
                switch (temp)
                {
                    // Three units up
                    case up:
                    {
                        getWorld()->generateSquirt(temp, getX(), getY() + 3);
                        break;
                    }
                    // Three units down
                    case down:
                    {
                        getWorld()->generateSquirt(temp, getX(), getY() - 3);
                        break;
                    }
                    // Three units right
                    case right:
                    {
                        getWorld()->generateSquirt(temp, getX() + 3, getY());
                        break;
                    }
                    case left:
                    {
                        getWorld()->generateSquirt(temp, getX() - 3, getY());
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            // Z activates any sonar charges that the player has
            case 'z':
            case 'Z':
            {
                if (getSonarCharges() > 0)
                {
                    getWorld()->activateSonar();
                    m_sonarCharges--;
                }
                break;
            }
            // Escape ends the current level
            case KEY_PRESS_ESCAPE:
            {
                getWorld()->decLives();
                setDead();
                break;
            }
        }
    }
}

void FrackMan::getAnnoyed(int annoyingFactor)
{
    // Decrement the health
    decreaseHealth(annoyingFactor);
    
    if (getHealth() <= 0)
    {
        getWorld()->decLives();
        setDead();
    }
    
}

// Removes the dirt objects that the FrackMan has excavated
void FrackMan::FrackRemoveDirt(int x, int y)
{
    StudentWorld* temp = getWorld();
    
    bool isDirtSomewhere = false;
    
    for (int i = 0; i <= 3; i++)
    {
        for (int j = 0; j <= 3; j++)
        {
            if (temp->dirtAt(x + i, y + j))
            {
                    temp->playSound(SOUND_DIG);
                isDirtSomewhere = true;
                goto nextStop;
            }
        }
    }
nextStop:
    
    for (int i = 0; i <= 3; i++)
    {
        for (int j = 0; j <= 3; j++)
        {
            temp->removeDirtAt(x + i, y + j);
        }
    }
    
}

int FrackMan::getGold() const
{
    return m_gold;
}

int FrackMan::getSonarCharges() const
{
    return m_sonarCharges;
}

int FrackMan::getSquirts() const
{
    return m_squirts;
}

void FrackMan::addGold()
{
    m_gold++;
}

void FrackMan::addSonarCharges()
{
    m_sonarCharges++;
}

void FrackMan::addWater()
{
    m_squirts += SQUIRTS_PER_WATER;
}

///////////////////////////////////////////////////////////////////////////
//  Protester implementation
///////////////////////////////////////////////////////////////////////////
Protester::Protester(StudentWorld* gp, int imageID, int startHealth, int startX,  int startY, Direction startDirection)
: GameCharacter(gp, imageID, startX, startY, startDirection, startHealth)
{
    int temp = getWorld()->getLevel();
    
    ticksNeededToWait = max(0, 3 - temp/4);
    ticksAlive = 0;
    
    
    hasShoutedLast15Ticks = false;
    ticksSinceLastShout = 0;
    
    numSquaresToMoveInCurrentDirection = rand() % 53 + 8;
    hasMadePerpendicularTurnLast200 = false;
    ticksSinceLastPerpendicular = 0;
    
    isInLeaveTheOilState = false;
    ticksStunned = 0;
}

bool Protester::isLeavingField() const
{
    return isInLeaveTheOilState;
}

void Protester::setStunned(int amtStunned)
{
    ticksStunned = amtStunned;
}
bool Protester::canTurnCorner()
{
    int protesterDirection = getDirection();
    int x = getX();
    int y = getY();
    
    StudentWorld* temp = getWorld();
    switch (protesterDirection)
    {
        case 1:
        case 2:
            if ((!temp->dirtOrBoulderEast(x, y) && canMoveEast()) || (!temp->dirtOrBoulderWest(x, y) && canMoveWest()))
                return true;
            else
                return false;
            
        case 3:
        case 4:
            if ((!temp->dirtOrBoulderNorth(x, y) && canMoveNorth()) || (!temp->dirtOrBoulderSouth(x, y) && canMoveSouth()))
                return true;
            else
                return false;
        default:
            return false;
    }
}

void Protester::turnCorner()
{
    int temp = getDirection();
    int x = getX();
    int y = getY();
    StudentWorld* world = getWorld();
    hasMadePerpendicularTurnLast200 = true;
    
    switch (temp)
    {
            // Facing Up or Down
        case 1:
        case 2:
            // If no boulders and the protester can change directions
            if (!world->dirtOrBoulderEast(x, y) && canMoveEast() && !world->dirtOrBoulderWest(x, y) && canMoveWest())
            {
                int randInt = rand() % 2 + 1;
                if (randInt == 1)
                    setDirection(right);
                else
                    setDirection(left);
                
                return;
            }
            else if (!world->dirtOrBoulderEast(x, y) && canMoveEast())
                setDirection(right);
            else
                setDirection(left);
            break;
            
            // Facing left or right
        case 3:
        case 4:
            // If no boulders and the protester can change directions
            if (!world->dirtOrBoulderNorth(x, y) && canMoveNorth() && !world->dirtOrBoulderSouth(x, y) && canMoveSouth())
            {
                int randInt = rand() % 2 + 1;
                if (randInt == 1)
                    setDirection(up);
                else
                    setDirection(down);
                
                return;
            }
            else if (!world->dirtOrBoulderNorth(x, y) && canMoveNorth())
                setDirection(up);
            else
                setDirection(down);
            break;
    }
}

void Protester::switchDirection()
{
    int x = getX();
    int y = getY();
    StudentWorld* temp = getWorld();
    
    bool hasSwitchedDirection = false;
    while (!hasSwitchedDirection)
    {
        int randDireciton = rand() % 4 + 1;
        switch (randDireciton)
        {
                // TODO : NEED TO CHECK IF SET DIRECTION IS VALID
            case 1:
                // Case up
                if (!temp->dirtOrBoulderNorth(x, y) || canMoveNorth())
                {
                    setDirection(up);
                    hasSwitchedDirection = true;
                }
                break;
            case 2:
                // Case down
                if (!temp->dirtOrBoulderSouth(x, y) || canMoveSouth())
                {
                    setDirection(down);
                    hasSwitchedDirection = true;
                }
                break;
            case 3:
                // Case left
                if (!temp->dirtOrBoulderWest(x, y) || canMoveWest())
                {
                    setDirection(left);
                    hasSwitchedDirection = true;
                }
                break;
            case 4:
                // Case right
                if (!temp->dirtOrBoulderEast(x, y) || canMoveEast())
                {
                    setDirection(right);
                    hasSwitchedDirection = true;
                }
                break;
            default:
                break;
        }
        numSquaresToMoveInCurrentDirection = rand() % 53 + 8;
    }
}

bool Protester::canPerformHorizontalOrVerticalAction()
{
    // In visible vertical range, but > 4 units away, protester beneath FrackMan
    if (getX() == getWorld()->getFrackX() && getY() < getWorld()->getFrackY() && getWorld()->distanceFromFracker(getX(), getY()) > VISIBLE_RANGE)
    {
        int isDirtBetween = false;
        for (int i = getY(); i < getWorld()->getFrackY(); i++)
        {
            for (int j = 0; j <= 3; j++)
            {
                if (getWorld()->dirtAt(getX() + j, i) || getWorld()->boulderAt(getX() + j, i))
                {
                    isDirtBetween = true;
                    goto NEXT_STEP;
                }
            }
        }
    NEXT_STEP:
        // No dirt between Protester and FrackMan
        if (!isDirtBetween)
        {
            numSquaresToMoveInCurrentDirection = 0;
            setDirection(up);
            moveTo(getX(), getY() + 1);
            return true;
        }
        else
            return false;
        
        // TODO: Else can't see FrackMan
    }
    
    // In visible vertical range, but > 4 units away, protester above FrackMan
    else if (getX() == getWorld()->getFrackX() && getY() > getWorld()->getFrackY() && getWorld()->distanceFromFracker(getX(), getY()) > VISIBLE_RANGE)
    {
        int isDirtBetween = false;
        for (int i = getWorld()->getFrackY(); i < getY(); i++)
        {
            for (int j = 0; j <= 3; j++)
            {
                if (getWorld()->dirtAt(getX() + j, i) || getWorld()->boulderAt(getX() + j, i))
                {
                    isDirtBetween = true;
                    goto NEXT_STEP1;
                }
            }
        }
    NEXT_STEP1:
        // No dirt between Protester and FrackMan
        if (!isDirtBetween)
        {
            setDirection(down);
            moveTo(getX(), getY() - 1);
            return true;
        }
        else
            return false;
        
        // TODO: Else can't see FrackMan
    }
    
    // In visible horizontal range, but > 4 units away, protester to the right of FrackMan
    else if (getY() == getWorld()->getFrackY() && getX() > getWorld()->getFrackX() && getWorld()->distanceFromFracker(getX(), getY()) > VISIBLE_RANGE)
    {
        int isDirtBetween = false;
        for (int i = getWorld()->getFrackX(); i < getX(); i++)
        {
            for (int j = 0; j <= 3; j++)
            {
                if (getWorld()->dirtAt(i, getY() + j) || getWorld()->boulderAt(i, getY() + j))
                {
                    isDirtBetween = true;
                    goto NEXT_STEP2;
                }
            }
        }
    NEXT_STEP2:
        // No dirt between Protester and FrackMan
        if (!isDirtBetween)
        {
            setDirection(left);
            moveTo(getX() - 1, getY());
            return true;
        }
        else
            return false;
        
        // TODO: Else can't see FrackMan
    }
    
    // In visible horizontal range, but > 4 units away, protester to the left of FrackMan
    else if (getY() == getWorld()->getFrackY() && getX() < getWorld()->getFrackX() && getWorld()->distanceFromFracker(getX(), getY()) > VISIBLE_RANGE)
    {
        int isDirtBetween = false;
        for (int i = getX(); i < getWorld()->getFrackX(); i++)
        {
            for (int j = 0; j <= 3; j++)
            {
                if (getWorld()->dirtAt(i, getY() + j) || getWorld()->boulderAt(i, getY() + j))
                {
                    isDirtBetween = true;
                    goto NEXT_STEP3;
                }
            }
        }
    NEXT_STEP3:
        // No dirt between Protester and FrackMan
        if (!isDirtBetween)
        {
            setDirection(right);
            moveTo(getX() + 1, getY());
            return true;
        }
        else
            return false;
    }
    return false;
}

bool Protester::checkIfFacingFrackMan()
{
    StudentWorld* temp = getWorld();
    int frackX = temp->getFrackX();
    int frackY = temp->getFrackY();
    
    Direction protesterDirection = getDirection();
    
    // In same vertical lane
    if (getX() == frackX)
    {
        // Below Frackman
        if (getY() < frackY)
        {
            if (protesterDirection == up)
                return true;
            else
                return false;
        }
        if (getY() > frackY)
        {
            if (protesterDirection == down)
                return true;
            else
                return false;
        }
        // Occupying same position
        if (getY() == frackY)
            return true;
    }
    
    // Same horizontal lane
    if (getY() == frackY)
    {
        if (getX() < frackX)
        {
            if (protesterDirection == right)
                return true;
            else
                return false;
        }
        if (getX() > frackX)
        {
            if (protesterDirection == left)
                return true;
            else
                return false;
        }
        
    }
    
    // Quadrant 1
    if (getX() < frackX && getY() < frackY)
    {
        if (protesterDirection == right || protesterDirection == up)
            return true;
        else
            return false;
    }
    
    // Quadrant 2
    if (getX() > frackX && getY() < frackY)
    {
        if (protesterDirection == left || protesterDirection == up)
            return true;
        else
            return false;
    }
    
    // Quadrant 3
    if (getX() > frackX && getY() > frackY)
    {
        if (protesterDirection == left || protesterDirection == down)
            return true;
        else
            return false;
    }
    
    // Quadrant 4
    if (getX() < frackX && getY() > frackY)
    {
        if (protesterDirection == right || protesterDirection == down)
            return true;
        else
            return false;
    }
    
    return false;
}

void Protester::moveProtester()
{
    int temp = getDirection();
    
    switch (temp)
    {
            // Up
        case 1:
            if (canMove(getX(), getY() + 1) && !getWorld()->dirtOrBoulderNorth(getX(), getY()) && getWorld()->checkForBouldersWithinRadius(getX(), getY() + 1))
                moveTo(getX(), getY() + 1);
            else
                numSquaresToMoveInCurrentDirection = 0;
            break;
            // Down
        case 2:
            if (canMove(getX(), getY() - 1) && !getWorld()->dirtOrBoulderSouth(getX(), getY()) && getWorld()->checkForBouldersWithinRadius(getX(), getY() - 1))
                moveTo(getX(), getY() - 1);
            else
                numSquaresToMoveInCurrentDirection = 0;
            break;
            // Left
        case 3:
            if (canMove(getX() - 1, getY()) && !getWorld()->dirtOrBoulderWest(getX(), getY()) && getWorld()->checkForBouldersWithinRadius(getX() - 1, getY()))
                moveTo(getX() - 1, getY());
            else
                numSquaresToMoveInCurrentDirection = 0;
            break;
            // Right
        case 4:
            if (canMove(getX() + 1, getY()) && !getWorld()->dirtOrBoulderEast(getX(), getY()) && getWorld()->checkForBouldersWithinRadius(getX() + 1, getY()))
                moveTo(getX() + 1, getY());
            else
                numSquaresToMoveInCurrentDirection = 0;
            break;
        default:
            break;
    }
    numSquaresToMoveInCurrentDirection--;
}

bool Protester::canDoSomethingDifferent()
{
    return false;
}

void Protester::doSomething()
{
    // Common to both RegularProtester and HarcoreProtester
    if (!isAlive())
        return;
    
    if (isInLeaveTheOilState)
    {
        if ((ticksNeededToWait != 0 && ticksAlive % ticksNeededToWait != 0))
        {
            ticksAlive++;
            return;
        }
        // TODO : MOVE TOWARDS 60, 60
        if (getX() == 60 && getY() == 60)
        {
            getWorld()->decNumProtesters();
            setDead();
        }
        ticksAlive++;
        getWorld()->generateLeavingField(getX(), getY());
        setDirection(getWorld()->getDirectionFromField(getX(), getY(), getWorld()->OIL_KEYWORD));
        moveProtester();
        return;
    }
    
    // Protester is stunned
    if (ticksStunned > 0)
    {
        ticksStunned--;
        return;
    }

    if (ticksSinceLastShout >= 15)
    {
        hasShoutedLast15Ticks = false;
        ticksSinceLastShout = 0;
    }
    
    if (ticksSinceLastPerpendicular > 200)
    {
        hasMadePerpendicularTurnLast200 = false;
        ticksSinceLastPerpendicular = 0;
    }
    
    if (hasShoutedLast15Ticks)
    {
        ticksSinceLastShout++;
    }
    
    // In Resting position
    if (ticksAlive == 0 || (ticksNeededToWait != 0 && ticksAlive % ticksNeededToWait != 0))
    {
        ticksAlive++;
        return;
    }
    else
    {
        ticksAlive++;
        
        // Increments in only non-resting ticks
        ticksSinceLastPerpendicular++;
        // Checks to see if in visible range (4 units) of Fracker
        
        if (getWorld()->distanceFromFracker(getX(), getY()) <= VISIBLE_RANGE && checkIfFacingFrackMan() && !hasShoutedLast15Ticks)
        {
            getWorld()->playSound(SOUND_PROTESTER_YELL);
            getWorld()->annoyFracker(PROTESTER_ANNOY_DAMAGE);
            hasShoutedLast15Ticks = true;
            return;
        }
        
        // Greater than 4 units away
        else if (getWorld()->distanceFromFracker(getX(), getY()) > VISIBLE_RANGE)
        {
            // HardCoreProtester specific
            if(canDoSomethingDifferent())
                return;
        }
        
        if (canPerformHorizontalOrVerticalAction())
        {
            
            return;
        }
        
        // Protester cannot directly see FrackMan
        
        if (numSquaresToMoveInCurrentDirection <= 0)
        {
            switchDirection();
        }
        
        // Protester hasn't turned in the last 200 ticks
        else if (!hasMadePerpendicularTurnLast200)
        {
            if (canTurnCorner())
            {
                turnCorner();
            }
        }
        
        moveProtester();
    }
}

bool Protester::isHardCore()
{
    return false;
}

void Protester::getAnnoyed(int annoyingFactor)
{
    // Killed by Boulder
    if (annoyingFactor == BOULDER_POINTS_OF_ANNOYANCE)
    {
        getWorld()->increaseScore(BOULDER_HIT_PROTESTER_POINTS);
    }
    
    decreaseHealth(annoyingFactor);
    
    ticksStunned = max(50, 100 - static_cast<int>(getWorld()->getLevel() * 10));
    
    if (getHealth() <= 0 && !isInLeaveTheOilState)
    {
        if (annoyingFactor == SQUIRT_DAMAGE && isHardCore())
        {
            getWorld()->increaseScore(SQUIRT_KILL_HC_PROTESTER_POINTS);
        }
        if (annoyingFactor == SQUIRT_DAMAGE)
        {
            getWorld()->increaseScore(SQUIRT_KILL_PROTESTER_POINTS);
        }
        
        isInLeaveTheOilState = true;
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
    }
    else
        getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
}

bool Protester::isProtester() const
{
    return true;
}

void Protester::setLeaveInState(bool shouldILeave)
{
    isInLeaveTheOilState = shouldILeave;
}

///////////////////////////////////////////////////////////////////////////
//  RegularProtester implementation
///////////////////////////////////////////////////////////////////////////

RegularProtester::RegularProtester(StudentWorld* gp, int imageID, int startHealth)
: Protester(gp, imageID, startHealth, 60, 60, left)
{
}

void RegularProtester::getBribed()
{
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(BRIBED_REG_PROTESTER_POINTS);
    setLeaveInState(true);
}

///////////////////////////////////////////////////////////////////////////
//  HardcoreProtester implementation
///////////////////////////////////////////////////////////////////////////
HardcoreProtester::HardcoreProtester(StudentWorld* gp)
: Protester(gp, IID_HARD_CORE_PROTESTER, HARDCOREPROTESTER_HEALTH, 60, 60, left)
{
}

bool HardcoreProtester::canDoSomethingDifferent()
{
    // Generates a vector field to go after player
    getWorld()->generateFieldToFracker(getX(), getY());
    
    int maxLegalDistance = 16 + getWorld()->getLevel() * 2;
    int distanceFromFracker = getWorld()->chaseFracker(getX(), getY(), maxLegalDistance);
    
    // Within distance of sensing FrackMan
    if (distanceFromFracker <= maxLegalDistance && distanceFromFracker >= 0)
    {
        Actor::Direction proposedDirection = getWorld()->getDirectionFromField(getX(), getY(), getWorld()->FRACKER_KEYWORD);
        setDirection(proposedDirection);
        moveProtester();
        return true;
    }
    return false;
}

bool HardcoreProtester::isHardCore()
{
    return true;
}
void HardcoreProtester::getBribed()
{
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(BRIBED_HC_PROTESTER_POINTS);
    int ticksStunned = min(50, static_cast<int>(100 - getWorld()->getLevel() * 10));
    setStunned(ticksStunned);
}