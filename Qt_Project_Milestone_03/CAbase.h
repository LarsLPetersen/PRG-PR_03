#ifndef CABASE_H
#define CABASE_H

#include <stdlib.h>
#include <ctime>
#include <qmath.h>
#include <QtDebug>

class CAbase {

public:
    CAbase() :
        Ny(10),
        Nx(10),
        nochanges(false)
        { resetWorldSize(Nx, Ny, 1); }

    CAbase(int nx, int ny) :
        Ny(ny),
        Nx(nx),
        nochanges(false)
        { resetWorldSize(Nx, Ny, 1); }

    ~CAbase() {
    }

    int getNy() {
        return Ny;
    }

    int getNx() {
        return Nx;
    }

    int getColor(int x, int y) {
        // get color of cell x, y
        return worldColor[y * (Nx + 2) + x];
    }

    void setColor(int x, int y, int c) {
        // set color c into cell x, y in current color universe
        worldColor[y * (Nx + 2) + x] = c;
    }

    void setColorNew(int x, int y, int c){
        // set color c into cell with coordinates x,y in evolution color universe
        worldColorNew[y * (Nx + 2) + x] = c;
    }

    int getLifetime(int x, int y) {
        // get lifetime of cell x, y
        return worldLifetime[y * (Nx + 2) + x];
    }

    void setLifetime(int x, int y, int l) {
        // set lifetime l into cell with coordinates x,y in current lifetime universe
        worldLifetime[y * (Nx + 2) + x] = l;
    }

    void setLifetimeNew(int x, int y, int l) {
        // set new lifetime l for x,y
        worldLifetimeNew[y * (Nx + 2) + x] = l;
    }

    int getValue(int x, int y) {
        return world[y * (Nx + 2) + x];
    }

    void setValue(int x, int y, int i) {
        // set number i into cell with coordinates x,y in current universe
        world[y * (Nx + 2) + x] = i;
    }

    void setValueNew(int x, int y, int i) {
        // set number i into cell with coordinates x,y in evolution universe
        worldNew[y * (Nx + 2) + x] = i;
    }

    int getDirection(int x, int y) {
        return worldDirection[y * (Nx + 2) + x];
    }

    void setDirection(int x, int y, int i) {
        worldDirection[y * (Nx + 2) + x] = i;
    }

    bool isNotChanged() {
        return nochanges;
    }

    void resetWorldSize(int nx, int ny, bool del = 0);

    //
    // game of life
    //
    int cellEvolutionLife(int x, int y);

    void worldEvolutionLife();

    //
    // snake
    //
    struct direction {
        int past;
        int future;
    } directionSnake;

    struct position {
        int x;
        int y;
    } positionSnakeHead, positionFood;

    position convert(int x, int y, int sD);

    int getSnakeLength() {
        return snakeLength;
    }

    void setSnakeLength(int l) {
        snakeLength = l;
    }

    int getSnakeAction() {
        return snakeAction;
    }

    void setSnakeAction(int a) {
        snakeAction = a;
    }

    void calcSnakeAction();

    void worldEvolutionSnake();

    void putNewFood();

    void putInitSnake();

    //
    // predator-prey
    //
    const int maxLifetime = __INT16_MAX__;

    int lifeTimeUI;

    void cellEvolutionConsistency(int x, int y);

    void cellEvolutionMove(int x, int y);

    void cellEvolutionDirection(int x, int y);

    void worldEvolutionPredator();


private:
    int Ny;
    int Nx;
    int *world;
    int *worldNew;
    int *worldColor;
    int *worldColorNew;
    int *worldLifetime;
    int *worldLifetimeNew;
    int *worldDirection;
    bool nochanges;
    int snakeAction;
    int snakeLength;
};


inline void CAbase::resetWorldSize(int nx, int ny, bool del) {
    // initialize randomization
    srand(time(NULL));

    // creation or re-creation of current and new universe with default values (0 for non-border cell and -1 for border cell)
    Nx = nx;
    Ny = ny;

    if (!del) {
        delete[] world;
        delete[] worldNew;

        delete[] worldColor;
        delete[] worldColorNew;

        delete[] worldLifetime;
        delete[] worldLifetimeNew;

        delete[] worldDirection;
    }

    world = new int[(Ny + 2) * (Nx + 2) + 1];
    worldNew = new int[(Ny + 2) * (Nx + 2) + 1];

    worldColor = new int[(Ny + 2) * (Nx + 2) + 1];
    worldColorNew = new int[(Ny + 2) * (Nx + 2) + 1];

    worldLifetime = new int[(Ny + 2) * (Nx + 2) + 1];
    worldLifetimeNew = new int[(Ny + 2) * (Nx + 2) + 1];

    worldDirection = new int[(Ny + 2) * (Nx + 2) + 1];

    for (int i = 0; i <= (Ny + 2) * (Nx + 2); i++) {
        // set border cells to -1 (still involving modular arithmetic -> toric case)
        if ( (i < (Nx + 2)) || (i >= (Ny + 1) * (Nx + 2)) || (i % (Nx + 2) == 0) || (i % (Nx + 2) == (Nx + 1)) ) {
            world[i] = -1;
            worldNew[i] = -1;

            worldColor[i] = -1;
            worldColorNew[i] = -1;

            worldLifetime[i] = -1;
            worldLifetimeNew[i] = -1;

            worldDirection[i] = -1;
        }
        else {
            world[i] = 0;
            worldNew[i] = 0;

            worldColor[i] = 0;
            worldColorNew[i] = 0;

            worldLifetime[i] = maxLifetime;
            worldLifetimeNew[i] = maxLifetime;

            worldDirection[i] = 0;
        }
    }
}


//
// game of life
//
inline int CAbase::cellEvolutionLife(int x, int y) {
    /* Rules
     *
     * Any living cell with fewer than two living neighbours dies, as if caused by underpopulation.
     * Any living cell with two or three living neighbours lives on to the next generation.
     * Any living cell with more than three living neighbours dies, as if by overpopulation.
     * Any dead cell with exactly three living neighbours becomes alive, as if by reproduction.
     */

    int n_sum = 0;
    int x1(0), y1(0);

    for (int ix = -1; ix <= 1; ix++) {
        for (int iy = -1; iy <= 1; iy++) {
            if (ix == 0 && iy == 0) continue;
            x1 = x + ix;
            y1 = y + iy;

            if (x1 < 1) x1 = Nx;
            if (x1 > Nx) x1 = 1;
            if (y1 < 1) y1 = Ny;
            if (y1 > Ny) y1 = 1;
            if (getValue(x1, y1) == 1) n_sum++;
        }
    }

    if (getValue(x, y) == 1) {
        if (n_sum == 2 || n_sum == 3) setValueNew(x, y, 1);
        else setValueNew(x, y, 0);
    }
    else {
        if (n_sum == 3) setValueNew(x, y, 1);
    }
    return 0;
}


inline void CAbase::worldEvolutionLife() {
    /* apply cell evolution to the universe */
    for (int ix = 1; ix <= Nx; ix++) {
        for (int iy = 1; iy <= Ny; iy++) {
            cellEvolutionLife(ix, iy);
        }
    }

    nochanges = true;
    /* copy new states to current states */
    for (int ix = 1; ix <= Nx; ix++) {
        for (int iy = 1; iy <= Ny; iy++) {
            if (world[iy * (Nx + 2) + ix] != worldNew[iy * (Nx + 2) + ix]) {
                nochanges = false;
            }
            world[iy * (Nx + 2) + ix] = worldNew[iy * (Nx + 2) + ix];
        }
    }
}


//
// snake
//
inline CAbase::position CAbase::convert(int x, int y, int sD) {
    /* map snakeDirection to array/grid coordinates */

    CAbase::position newCoord;
    switch (sD) {
    case 8: // up
       newCoord.x = x;
       newCoord.y = y - 1;
       break;
    case 2: // down
       newCoord.x = x;
       newCoord.y = y + 1;
       break;
    case 4: // left
       newCoord.x = x - 1;
       newCoord.y = y;
       break;
    case 6:  // right
       newCoord.x = x + 1;
       newCoord.y = y;
       break;
    default:
       break;
    }
    return newCoord;
}


inline void CAbase::putNewFood() {
    /* randomly put one piece of food on the field */

    srand(time(NULL));
    bool locationFound = false;
    int xFood, yFood;

    /* only put food in empty spots */
    while (!locationFound) {
        xFood = rand() % Nx + 1;
        yFood = rand() % Ny + 1;
        int v = getValue(xFood, yFood);
        if (v != 0) continue;
        locationFound = true;
    }
    positionFood.x = xFood;
    positionFood.y = yFood;
    setValue(xFood, yFood, 5);
}


inline void CAbase::putInitSnake(){
    /* put initial snake on the field */

    // set it up vertically in the center, lower sector
    int xSnakeHead, ySnakeHead;
    xSnakeHead = int(floor(double(Nx) / 2));
    ySnakeHead = int(floor(2 * double(Ny) / 3));

    // length 3
    setValue(xSnakeHead, ySnakeHead, 10);
    setValue(xSnakeHead, ySnakeHead + 1, 11);
    setValue(xSnakeHead, ySnakeHead + 2, 12);

    snakeLength = 3;
    positionSnakeHead.x = xSnakeHead;
    positionSnakeHead.y = ySnakeHead;

    // initially move up
    directionSnake.past = 8;
    directionSnake.future = 8;
}


inline void CAbase::calcSnakeAction(){
    /* calculate the next action of the snake (move / move and feed / die) */

    int neighborhood[3][3];
    int x1(0), y1(0);

    // calculate 3x3 neighborhood of snakehead ([1][1])
    for (int ix = -1; ix <= 1; ix++) {
        x1 = positionSnakeHead.x + ix;
        if ( (x1 <= 0) || (x1 >= Nx + 1) ) x1 = 0;

        for (int iy = -1; iy <= 1; iy++) {
            y1 = positionSnakeHead.y + iy;
            if ( (y1 <= 0) || (y1 >= Ny + 1) ) y1 = 0;
            neighborhood[ix + 1][iy + 1] = getValue(x1, y1);
        }
    }

#ifndef QT_DEBUG
    qDebug() << "positionSnakeHead: " << positionSnakeHead.x << " " << positionSnakeHead.y;
    qDebug("Neighborhood of SnakeHead");
    for (int i = 0; i <= 2; i++) {
        qDebug() << neighborhood[0][i] << " " <<  neighborhood[1][i] << " " << neighborhood[2][i];
    }
#endif

    // determine snake action relative to snake direction based on snakehead in its neighborhood
    switch (directionSnake.future) {
    case 8: // up
        if (neighborhood[1][0] == 5) {
            snakeAction = 1; // move and feed
        } else if (neighborhood[1][0] == -1 || neighborhood[1][0] >= 10) {
            snakeAction = 2; // move and die
        } else snakeAction = 0;
        break;

    case 2: // down
        if (neighborhood[1][2] == 5) {
            snakeAction = 1; // move and feed
        } else if (neighborhood[1][2] == -1 || neighborhood[1][2] >= 10 ) {
            snakeAction = 2; // move and die
        } else snakeAction = 0;
        break;

    case 4: // left
        if (neighborhood[0][1] == 5) {
            snakeAction = 1; // move and feed
        } else if (neighborhood[0][1] == -1 || neighborhood[0][1] >= 10) {
            snakeAction = 2; // move and die
        } else snakeAction = 0;
        break;

    case 6: // right
        if (neighborhood[2][1] == 5) {
            snakeAction = 1; // move and feed
        } else if (neighborhood[2][1] == -1 || neighborhood[2][1] >= 10) {
            snakeAction = 2; // move and die
        } else snakeAction = 0;
        break;

    default:
        break;
    }
}


inline void CAbase::worldEvolutionSnake() {
    /* Rules
     *
     * Snake may move horizontally or vertically.
     * It dies when it hits the boundary or itself.
     * It grows by feeding - one piece of food at a time.
     */

    // calculate upcoming snake action
    calcSnakeAction();
    int dS = directionSnake.future;

#ifndef QT_DEBUG
    qDebug() << "action: " << snakeAction;
    qDebug() << "future_dir: " << directionSnake.future << " " << "past_dir: " << directionSnake.past;
    qDebug() << "slen: " << snakeLength;
#endif

    // based on the global action each of the three cases is considered individually
    switch (snakeAction) {
    //
    // move
    //
    case 0:
        for (int x = 1; x <= Nx; x++) {
            for (int y = 1; y <= Ny; y++) {
                int v = getValue(x, y);
                // head
                if (v == 10) {
#ifndef QT_DEBUG
                    qDebug() << "(x, y) = (" << x << ", " << y << ")  -> (" << convert(x, y, dS).x << ", " << convert(x, y, dS).y << ")";
#endif
                    setValueNew(x, y, v + 1);
                    setValueNew(convert(x, y, dS).x, convert(x, y, dS).y, 10);
                    positionSnakeHead.x = convert(x, y, dS).x;
                    positionSnakeHead.y = convert(x, y, dS).y;
#ifndef QT_DEBUG
                    qDebug() << "sH: " << positionSnakeHead.x << " " << positionSnakeHead.y;
#endif
                // body
                } else if (v > 10 && v < 10 + snakeLength - 1) {
                    setValueNew(x, y, v + 1);
                // tail
                } else if (v == 10 + snakeLength - 1) {
                    setValueNew(x, y, 0);
                // food
                } else if (v == 5) {
                    setValueNew(x, y, v);
                }
                // otherwise values are initialized with 0
            }
        }

        // copy new state to current universe
        for (int x = 1; x <= Nx; x++) {
            for (int y = 1; y <= Ny; y++) {
                world[y * (Nx + 2) + x] = worldNew[y * (Nx + 2) + x];
            }
        }
        nochanges = false;
        directionSnake.past = directionSnake.future;
        break;

    //
    // move and feed
    //
    case 1:
        for (int x = 1; x <= Nx; x++) {
            for (int y = 1; y <= Ny; y++) {
                int v = getValue(x, y);
                if (v == 10) {
                    setValueNew(x, y, v + 1);
                    setValueNew(convert(x, y, dS).x, convert(x, y, dS).y, 10);
                    positionSnakeHead.x = convert(x, y, dS).x;
                    positionSnakeHead.y = convert(x, y, dS).y;
                } else if (v > 10) {
                    setValueNew(x, y, v + 1);
                } else {

                }
            }
        }

        // copy new state to current universe
        for (int x = 1; x <= Nx; x++) {
            for (int y = 1; y <= Ny; y++) {
                world[y * (Nx + 2) + x] = worldNew[y * (Nx + 2) + x];
            }
        }
        nochanges = false;
        snakeLength++;
        directionSnake.past = directionSnake.future;
        putNewFood();
        break;

    //
    // move and die
    //
    case 2:
        nochanges = true;
        break;

    default:
        break;
    }
}


//
// predator-prey
//
inline void CAbase::cellEvolutionConsistency(int x, int y) {
    /* rectify directions so that at most one neighbor is incoming */

    int neighborhoodDirections[5] {0};
    neighborhoodDirections[0] = getDirection(x, y);
    neighborhoodDirections[1] = getDirection(x, y + 1); // down
    neighborhoodDirections[2] = getDirection(x - 1, y); // left
    neighborhoodDirections[3] = getDirection(x + 1, y); // right
    neighborhoodDirections[4] = getDirection(x, y - 1); // up

    int neighborhoodLifetimes[5] {0};
    neighborhoodLifetimes[0] = getLifetime(x, y);
    neighborhoodLifetimes[1] = getLifetime(x, y + 1); // down
    neighborhoodLifetimes[2] = getLifetime(x - 1, y); // left
    neighborhoodLifetimes[3] = getLifetime(x + 1, y); // right
    neighborhoodLifetimes[4] = getLifetime(x, y - 1); // up

    int incomingNeighbors[5] {0};
    int incomingViableNeighbors[5] {0};
    int n_sum = 0;
    int nv_sum = 0;

    // compute incoming and "viable" incoming neighbors
    for (int i = 1; i < 5; i++) {
        if (neighborhoodDirections[i] + 2 * i == 10) {
            incomingNeighbors[i] = 1;
            n_sum++;
            if (neighborhoodLifetimes[i] > 0) { // a viable incoming neighbor must have a positive lifetime
                incomingViableNeighbors[i] = 1;
                nv_sum++;
            }
        }
    }

    // no viable incoming neighbor -> all incoming neighbors won't move
    if (nv_sum == 0) {
        for (int i = 1; i < 5; i++) {
            if (incomingNeighbors[i] == 1) {
                CAbase::position incomingCellCoordinates = CAbase::convert(x, y, 2 * i);
                setDirection(incomingCellCoordinates.x, incomingCellCoordinates.y, 0);
            }
        }
    }
    // exactly one viable incoming neighbor -> all other incoming neighbors won't move
    else if (nv_sum == 1) {
        for (int i = 1; i < 5; i++) {
            if (incomingNeighbors[i] == 1 && incomingViableNeighbors[i] != 1) {
                CAbase::position incomingCellCoordinates = CAbase::convert(x, y, 2 * i);
                setDirection(incomingCellCoordinates.x, incomingCellCoordinates.y, 0);
            }
        }
    }
    // more than one viable incoming neighbor -> randomly pick one; all other incoming neighbors won't move
    else {
        int r = rand() % nv_sum + 1;
        for (int i = 1; i < 5; i++) {
            if (incomingNeighbors[i] == 1) {
                CAbase::position incomingCellCoordinates = CAbase::convert(x, y, 2 * i);
                if (incomingViableNeighbors[i] == 1) {
                    r -= 1;
                    if (r != 0) { // these incoming viable neighbors won't move
                        setDirection(incomingCellCoordinates.x, incomingCellCoordinates.y, 0);
                    }
                }
                else { // these incoming non-viable neighbors won't move
                    setDirection(incomingCellCoordinates.x, incomingCellCoordinates.y, 0);
                }
            }
        }
    }
}


inline void CAbase::cellEvolutionMove(int x, int y) {
    /* compute new value and new lifetime of cell x, y */

    int lifeTime = getLifetime(x, y);
    int value = getValue(x, y);

    int neighborhoodDirections[5];
    neighborhoodDirections[0] = getDirection(x, y);
    neighborhoodDirections[1] = getDirection(x, y + 1); // down
    neighborhoodDirections[2] = getDirection(x - 1, y); // left
    neighborhoodDirections[3] = getDirection(x + 1, y); // right
    neighborhoodDirections[4] = getDirection(x, y - 1); // up

    int incomingNeighbors[5] {0};
    int n_sum = 0;
    for (int i = 0; i < 5; i++) {
        if (neighborhoodDirections[i] + 2 * i == 10) {
            incomingNeighbors[i] = 1;
            n_sum++;
        }
    }

    if (n_sum == 0) { // no neighbor aims at this cell
        if (neighborhoodDirections[0] == 0) { // cell itself has no aim
            if (lifeTime == maxLifetime) { // non-living cell
                setValueNew(x, y, getValue(x, y));
                setLifetimeNew(x, y, maxLifetime);
            } else { // living cell
                if (lifeTime > 0) { // living cell grows older
                    setValueNew(x, y, getValue(x, y));
                    setLifetimeNew(x, y, lifeTime - 1);
                }
                else { // living cell dies/disappears
                    setValueNew(x, y, 0);
                    setLifetimeNew(x, y, maxLifetime);
                }
            }

        } else { // cell itself aims at a legal position
            setValueNew(x, y, 0);
            setLifetimeNew(x, y, maxLifetime);
        }

    } else if (n_sum == 1) { // exactly one living neighbor aims at this cell
        int i = 1;
        while (incomingNeighbors[i] != 1) {
            i++;
        }
        position incomingCellCoordinates = convert(x, y, 2 * i);
        setValueNew(x, y, getValue(incomingCellCoordinates.x, incomingCellCoordinates.y));
        if (value == 2 || value == 5) { // cell is devoured
            setLifetimeNew(x, y, lifeTimeUI);
        } else {
            setLifetimeNew(x, y, getLifetime(incomingCellCoordinates.x, incomingCellCoordinates.y) - 1);
        }

    } else if (n_sum > 1) {
        qWarning() << "More than one neighbor aims at a cell!";
    }
}


inline void CAbase::cellEvolutionDirection(int x, int y) {
    /* set a preliminary moving direction for cell x, y */

    int neighbors[5];
    neighbors[0] = getValue(x, y);
    neighbors[1] = getValue(x, y + 1); // down
    neighbors[2] = getValue(x - 1, y); // left
    neighbors[3] = getValue(x + 1, y); // right
    neighbors[4] = getValue(x, y - 1); // up
    int n_sum = 0;

    //
    // predator
    //
    if (neighbors[0] == 1) {
        int preyNeighbors[5] {0};

        for (int i = 1; i <= 4; i++) {
            if (neighbors[i] == 2) {
                preyNeighbors[i] = 1;
                n_sum++;
            }
        }

        // NO PREY IN NEIGHBORHOOD
        if (n_sum == 0) {
            int allowedDirections[5] {0};
            int na_sum = 0;
            for (int i = 1; i < 5; i++) { // determine which directions are "allowed"
                if (neighbors[i] != 1 && neighbors[i] != -1 && neighbors[i] != 5) { // excluding neighboring predators, boundary and food
                    allowedDirections[i] = 1;
                    na_sum++;
                }
            }
            if (na_sum == 0) { // no allowed direction
                setDirection(x, y, 0);
            } else if (na_sum == 1) { // exactly one direction is allowed
                int i = 1;
                while (allowedDirections[i] != 1) {
                    i++;
                }
                setDirection(x, y, 2 * i);
            } else if (na_sum > 1) { // more than one direction is allowed
                int r = rand() % (na_sum) + 1;
                int i = 0;
                while (r > 0) {
                    i += 1;
                    if (allowedDirections[i] == 1) {
                        r -= 1;
                    }
                }
                setDirection(x, y, 2 * i);
            }

        // EXACTLY ONE PREY ITEM IN NEIGHBORHOOD
        } else if (n_sum == 1) {
            int i = 1;
            while (preyNeighbors[i] != 1) {
                i++;
            }
            setDirection(x, y, 2 * i);

        // MOVE TOWARDS A RANDOM PREY NEIGHBOR
        } else if (n_sum > 1) {
            int r = rand() % (n_sum) + 1;
            int i = 0;
            while (r > 0) {
                i += 1;
                if (preyNeighbors[i] == 1) {
                    r -= 1;
                }
            }
            setDirection(x, y, 2 * i);
        }
    //
    // prey
    //
    } else if (neighbors[0] == 2) {
        bool hasPredatorNeighbor {false};
        for (int i = 1; i <= 4; i++) {
            if (neighbors[i] == 1) {
                hasPredatorNeighbor = true;
                break;
            }
        }

        if (hasPredatorNeighbor) { // freeze
            setDirection(x, y, 0);
        }
        else {
            int foodNeighbors[5] {0};
            for (int i = 1; i <= 4; i++) {
                if (neighbors[i] == 5) {
                    foodNeighbors[i] = 1;
                    n_sum++;
                }
            }
            // NO FOOD NEIGBHOR
            if (n_sum == 0) {
                int allowedDirections[5] {0};
                int na_sum = 0;
                for (int i = 1; i < 5; i++) { // determine which directions are "allowed"
                    if (neighbors[i] != 2 && neighbors[i] != -1) { // excluding neighboring prey and boundary
                        allowedDirections[i] = 1;
                        na_sum++;
                    }
                }
                if (na_sum == 0) { // no allowed direction
                    setDirection(x, y, 0);
                } else if (na_sum == 1) { // exactly one direction is allowed
                    int i = 1;
                    while (allowedDirections[i] != 1) {
                        i++;
                    }
                    setDirection(x, y, 2 * i);
                } else if (na_sum > 1) { // more than one direction is allowed
                    int r = rand() % (na_sum) + 1;
                    int i = 0;
                    while (r > 0) {
                        i += 1;
                        if (allowedDirections[i] == 1) {
                            r -= 1;
                        }
                    }
                    setDirection(x, y, 2 * i);
                }
            // EXACTLY ONE FOOD NEIGHBOR
            } else if (n_sum == 1) { // move towards this food neighbor
                int i = 1;
                while (foodNeighbors[i] != 1) {
                    i++;
                }
                setDirection(x, y, 2 * i);
            // MORE THAN ONE FOOD NEIGHBOR
            } else if (n_sum > 1) { // randomly move towards a random food neighbor
                int r = rand() % (n_sum) + 1;
                int i = 0;
                while (r > 0) {
                    i++;
                    if (foodNeighbors[i] == 1) {
                        r -= 1;
                    }
                }
                setDirection(x, y, 2 * i);
            }
        }
    //
    // food or empty cell
    //
    } else {
        setDirection(x, y, 0);
    }
}


inline void CAbase::worldEvolutionPredator() {
    /* combine evolutionary functions on cell level to array level */

    // calculate a priori possible moving directions for each cell
    for (int ix = 1; ix <= Nx; ix++) {
        for (int iy = 1; iy <= Ny; iy++) {
            cellEvolutionDirection(ix, iy);
        }
    }

    // make sure there is at most one incoming viable neighbor for each cell
    for (int ix = 1; ix <= Nx; ix++) {
        for (int iy = 1; iy <= Ny; iy++) {
            cellEvolutionConsistency(ix, iy);
        }
    }

    // calculate new status and new lifetime for each cell
    for (int ix = 1; ix <= Nx; ix++) {
        for (int iy = 1; iy <= Ny; iy++) {
            cellEvolutionMove(ix, iy);
        }
    }

    nochanges = true;
    for (int ix = 1; ix <= Nx; ix++) {
        for (int iy = 1; iy <= Ny; iy++) {
            // game goes on while at least one cell has lifetime >=0 and less than maxLifetime, so this cell isn't food or empty
            if ((worldLifetimeNew[iy * (Nx + 2) + ix] >= 0) && (worldLifetimeNew[iy * (Nx + 2) + ix] < maxLifetime)) {
                nochanges = false;
            }
            // transfer array values from new to current
            world[iy * (Nx + 2) + ix] = worldNew[iy * (Nx + 2) + ix];
            worldLifetime[iy * (Nx + 2) + ix] = worldLifetimeNew[iy * (Nx + 2) + ix];
        }
    }
}


#endif // CABASE_H
