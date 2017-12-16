#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QColor>
#include <QWidget>
#include <QObject>
#include "CAbase.h"


class GameWidget : public QWidget {

    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = 0);
    ~GameWidget();
    CAbase getCA();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

signals:
    void universeModified(int, bool);
    void gameStarted(int, bool);
    void gameStopped(int, bool);
    void gameEnds(int, bool);


public slots:
    void startGame(const int &number = -1);
    void stopGame();
    void clearGame();

    int getUniverseSize();
    void setUniverseSize(const int &s);

    int getUniverseMode();
    void setUniverseMode(const int &m);

    int getCellMode();
    void setCellMode(const int &m);

    int getInterval();
    void setInterval(int msec);

    int getLifetime();
    void setLifetime(const int &l);

    QColor getMasterColor();
    void setMasterColor(const QColor &color);

    QColor getPredefinedColor(const int &color);

    QString dumpGame();
    void reconstructGame(const QString &data);

    //
    // snake
    //
    void calcDirectionSnake (int dS);

    void setDirectionSnake(int past, int future);

    void setPositionSnakeHead(int x, int y);

    void setPositionFood(int x, int y);

    void setSnakeLength(int l);

    void setSnakeAction(int a);

    //
    // predator-prey
    //

private slots:
    void paintGrid(QPainter &p);
    void paintUniverse(QPainter &p);
    void newGeneration();
    void newGenerationColor();

private:
    QColor masterColor;
    QTimer *timer;
    QTimer *timerColor;
    CAbase ca1;
    int universeSize;
    int universeMode;
    int cellMode;
    int lifeTime;
    int generations;
    // int randomMode;

};


#endif // GAMEWIDGET_H
