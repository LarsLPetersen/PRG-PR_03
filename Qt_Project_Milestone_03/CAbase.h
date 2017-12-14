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


private:
    int Ny;
    int Nx;
    int *world;
    int *worldNew;
    int *worldColor;
    int *worldColorNew;
    int *worldLifetime;
    int *worldLifetimeNew;
    bool nochanges;
    int snakeAction;
    int snakeLength;
};


inline void CAbase::resetWorldSize(int nx, int ny, bool del) {
    // creation or re-creation of current and new universe with default values (0 for non-border cell and -1 for border cell)
    Nx = nx;
    Ny = ny;

    if (!del) {
        delete[] world;
        delete[] worldNew;

        // color
        delete[] worldColor;
        delete[] worldColorNew;

        //lifetime
        delete[] worldLifetime;
        delete[] worldLifetimeNew;
    }
    // main universe
    world = new int[(Ny + 2) * (Nx + 2) + 1];
    worldNew = new int[(Ny + 2) * (Nx + 2) + 1];

    // color
    worldColor = new int[(Ny + 2) * (Nx + 2) + 1];
    worldColorNew = new int[(Ny + 2) * (Nx + 2) + 1];

    // lifetime
    worldLifetime = new int[(Ny + 2) * (Nx + 2) + 1];
    worldLifetimeNew = new int[(Ny + 2) * (Nx + 2) + 1];

    for (int i = 0; i <= (Ny + 2) * (Nx + 2); i++) {
        // set border cells to -1 (still involving modular arithmetic -> toric case
        if ( (i < (Nx + 2)) || (i >= (Ny + 1) * (Nx + 2)) || (i % (Nx + 2) == 0) || (i % (Nx + 2) == (Nx + 1)) ) {
            world[i] = -1;
            worldNew[i] = -1;

            // Color
            worldColor[i] = -1;
            worldColorNew[i] = -1;

            // Lifetime
            worldLifetime[i] = -1;
            worldLifetimeNew[i] = -1;
        }
        // default to 0
        else {
            // main universe
            world[i] = 0;
            worldNew[i] = 0;

            // color
            worldColor[i] = 0;
            worldColorNew[i] = 0;

            // cifetime
            worldLifetime[i] = 0;
            worldLifetimeNew[i] = 0;
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


#endif // CABASE_H
