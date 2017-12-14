#include <QTextStream>
#include <QFileDialog>
#include <QDebug>
#include <QColor>
#include <QMessageBox>
#include <QColorDialog>
#include <ctime>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "keypressfilter.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentColor(QColor(0, 0, 0)),
    game(new GameWidget(this))
{
    ui->setupUi(this);

    /* game choices */
    ui->universeModeControl->addItem("Game of Life");
    ui->universeModeControl->addItem("Snake");

    /* cell mode choices */
    ui->cellModeControl->addItem("Classic");

    /* color icons for color buttons */
    QPixmap icon(16, 16);
    icon.fill(currentColor);
    ui->colorSelectButton->setIcon(QIcon(icon));

    /* game control buttons */
    connect(ui->startButton, SIGNAL(clicked()), game, SLOT(startGame()));
    connect(ui->stopButton, SIGNAL(clicked()), game, SLOT(stopGame()));
    connect(ui->clearButton, SIGNAL(clicked()), game, SLOT(clearGame()));

    /* spin boxes */
    connect(ui->intervalControl, SIGNAL(valueChanged(int)), game, SLOT(setInterval(int)));
    connect(ui->universeSizeControl, SIGNAL(valueChanged(int)), game, SLOT(setUniverseSize(int)));

    /* combo boxes */
    connect(ui->universeModeControl, SIGNAL(currentIndexChanged(int)), game, SLOT(setUniverseMode(int)));
    connect(ui->cellModeControl, SIGNAL(currentIndexChanged(int)), game, SLOT(setCellMode(int)));

    /* enable/disable interaction during the game */
    connect(game, SIGNAL(gameStarted(bool)), ui->universeSizeControl, SLOT(setDisabled(bool)));
    connect(game, SIGNAL(gameStarted(bool)), ui->universeModeControl, SLOT(setDisabled(bool)));
    connect(game, SIGNAL(gameStarted(bool)), ui->intervalControl, SLOT(setDisabled(bool)));
    connect(game, SIGNAL(gameStarted(bool)), ui->saveButton, SLOT(setDisabled(bool)));
    connect(game, SIGNAL(gameStarted(bool)), ui->loadButton, SLOT(setDisabled(bool)));
    //connect(game, SIGNAL(gameStarted(bool)), ui->cellModeControl, SLOT(setDisabled(bool)));

    connect(game, SIGNAL(universeModified(bool)), ui->universeSizeControl, SLOT(setDisabled(bool)));
    connect(game, SIGNAL(universeModified(bool)), ui->universeModeControl, SLOT(setDisabled(bool)));
    connect(game, SIGNAL(universeModified(bool)), ui->intervalControl, SLOT(setDisabled(bool)));
    //connect(game, SIGNAL(universeModified(bool)), ui->cellModeControl, SLOT(setDisabled(bool)));

    connect(game, SIGNAL(gameStopped(bool)), ui->universeSizeControl, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameStopped(bool)), ui->universeModeControl, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameStopped(bool)), ui->intervalControl, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameStopped(bool)), ui->saveButton, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameStopped(bool)), ui->loadButton, SLOT(setEnabled(bool)));
    //connect(game, SIGNAL(gameStopped(bool)), ui->cellModeControl, SLOT(setEnabled(bool)));

    connect(game, SIGNAL(gameEnds(bool)), ui->universeSizeControl, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameEnds(bool)), ui->universeModeControl, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameEnds(bool)), ui->intervalControl, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameEnds(bool)), ui->saveButton, SLOT(setEnabled(bool)));
    connect(game, SIGNAL(gameEnds(bool)), ui->loadButton, SLOT(setEnabled(bool)));
    //connect(game, SIGNAL(gameEnds(bool)), ui->cellModeControl, SLOT(setEnabled(bool)));

    /* color choices */
    connect(ui->colorSelectButton, SIGNAL(clicked()), this, SLOT(selectMasterColor()));
    connect(ui->colorRandomButton, SIGNAL(clicked()), this, SLOT(selectRandomColor()));

    /* load/save game */
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(saveGame()));
    connect(ui->loadButton, SIGNAL(clicked()), this, SLOT(loadGame()));

    /* stretch layout for better looks */
    ui->mainLayout->setStretchFactor(ui->gameLayout, 8);
    ui->mainLayout->setStretchFactor(ui->settingsLayout, 3);

    /* add gamewidget to gameLayout */
    ui->gameLayout->addWidget(game);

    /* send keystrokes to snake game */
    KeyPressFilter *keyPressFilter = new KeyPressFilter(this->game);
    this->installEventFilter(keyPressFilter);
    connect(keyPressFilter, SIGNAL(keyPressed(int)), game, SLOT(calcDirectionSnake(int)));
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::saveGame() {
    int uM = game->getUniverseMode();
    QString filename, size, buffer;
    QColor color;
    QFile file;

    switch (uM) {
    //
    // game of life
    //
    case 0:

        filename = QFileDialog::getSaveFileName(this, tr("Save current game"),
                                                QDir::homePath(), tr("Game of Life *.game Files (*.game_of_life)"));
        if (filename.length() < 1)
            return;

        file.setFileName(filename);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::warning(this,
                                 tr("File Not Saved"),
                                 tr("For some reason the game could not be written to the chosen file."),
                                 QMessageBox::Ok);
            return;
        }

        size = QString::number(game->getUniverseSize()) + "\n";
        file.write(size.toUtf8());
        file.write(game->dumpGame().toUtf8());

        color = game->getMasterColor();
        buffer = QString::number(color.red()) + " " +
                         QString::number(color.green()) + " " +
                         QString::number(color.blue()) + "\n";
        file.write(buffer.toUtf8());
        buffer.clear();
        buffer = QString::number(ui->intervalControl->value()) + "\n";
        file.write(buffer.toUtf8());
        file.close();
        break;

    //
    //  snake
    //
    case 1:
        filename = QFileDialog::getSaveFileName(this, tr("Save current game"),
                                                QDir::homePath(), tr("Snake *.snake Files (*.snake)"));
        if (filename.length() < 1)
            return;

        file.setFileName(filename);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::warning(this,
                                 tr("File Not Saved"),
                                 tr("For some reason the game could not be written to the chosen file."),
                                 QMessageBox::Ok);
            return;
        }

        color = game->getMasterColor();

        buffer = QString::number(game->getUniverseSize()) + "\n";

        buffer += QString::number(color.red()) + " " +
                  QString::number(color.green()) + " " +
                  QString::number(color.blue()) + "\n";

        buffer += QString::number(ui->intervalControl->value()) + "\n";

        buffer += QString::number(game->getCA().directionSnake.past) + "\n" +
                  QString::number(game->getCA().directionSnake.future) + "\n" +
//                  QString::number(game->getCA().snakeLength) + "\n" +
//                  QString::number(game->getCA().snakeAction) + "\n" +
                    QString::number(game->getCA().getSnakeLength()) + "\n" +
                    QString::number(game->getCA().getSnakeAction()) + "\n" +

                  QString::number(game->getCA().positionSnakeHead.x) + " " + QString::number(game->getCA().positionSnakeHead.y) + "\n" +
                  QString::number(game->getCA().positionFood.x) + " " + QString::number(game->getCA().positionFood.y) + "\n";
        file.write(buffer.toUtf8());
        buffer.clear();

        file.write(game->dumpGame().toUtf8());
        file.close();

        break;
    }
}


void MainWindow::loadGame() {

    /* file dialog */
    int uM = game->getUniverseMode();
    QString filename;
    QFile file;

    switch (uM) {
    //
    // game of life
    //
    case 0:
        filename = QFileDialog::getOpenFileName(this, tr("Open saved game"),
                                                QDir::homePath(), tr("Game of Life File (*.game_of_life)"));
        break;
    //
    // snake
    //
    case 1:
        filename = QFileDialog::getOpenFileName(this, tr("Open saved game"),
                                                QDir::homePath(), tr("Snake File (*.snake)"));
        break;

    default:
        break;
    }

    if (filename.length() < 1)
        return;
    file.setFileName(filename);

    if (!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,
                             tr("File Not Loaded"),
                             tr("For some reason the chosen file could not be loaded."),
                             QMessageBox::Ok);
        return;
    }

    QTextStream file_input_stream(&file);
    QString dump, tmp;
    QPixmap icon(16, 16);
    int r, g, b;
    int stream_value;

    switch (uM) {
    //
    // game of life
    //
    case 0:
        file_input_stream >> stream_value;
        ui->universeSizeControl->setValue(stream_value);

        game->setUniverseSize(stream_value);
        dump = "";

        for (int k = 0; k != stream_value; k++) {
            file_input_stream >> tmp;
            dump.append(tmp + "\n");
        }
        game->reconstructGame(dump);

        /* import the (rgb) cell color */
        file_input_stream >> r >> g >> b;
        currentColor = QColor(r, g, b);
        game->setMasterColor(currentColor);

        /* display specific color as icon on color buttons */
        icon.fill(currentColor);
        ui->colorSelectButton->setIcon(QIcon(icon));

        /* import iteration interval */
        file_input_stream >> r;
        ui->intervalControl->setValue(r);
        game->setInterval(r);
        break;

    //
    // snake
    //
    case 1:
        file_input_stream >> stream_value;
        ui->universeSizeControl->setValue(stream_value);
        game->setUniverseSize(stream_value);

        file_input_stream >> r >> g >> b;
        currentColor = QColor(r, g, b);
        game->setMasterColor(currentColor);
        icon.fill(currentColor);
        ui->colorSelectButton->setIcon(QIcon(icon));

        file_input_stream >> r;
        ui->intervalControl->setValue(r);
        game->setInterval(r);

        file_input_stream >> r >> g;
        game->setDirectionSnake(r, g);

        file_input_stream >> r;
        game->setSnakeLength(r);

        file_input_stream >> r;
        game->setSnakeAction(r);

        file_input_stream >> r >> g;
        game->setPositionSnakeHead(r, g);

        file_input_stream >> r >> g;
        game->setPositionFood(r, g);

        dump = "";
        for (int k = 0; k != stream_value; k++) {
            file_input_stream >> tmp;
            dump.append(tmp + "\n");
        }
        //qDebug() << dump;
        game->reconstructGame(dump);
        break;

    default:
        break;
    }

}


void MainWindow::selectMasterColor() {
    /* set cell color to color chosen from color dialog */
    QColor color = QColorDialog::getColor(currentColor, this, tr("Select Cell Color"));
    if (!color.isValid())
        return;
    currentColor = color;
    game->setMasterColor(color);

    /* display specific color as icon on color button */
    QPixmap icon(16, 16);
    icon.fill(color);
    ui->colorSelectButton->setIcon(QIcon(icon));
}


void MainWindow::selectRandomColor() {
    /* generate random rgb values */

    srand(time(NULL));
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;

    /* set cell color to specific rbg value */
    QColor color(r, g, b);
    if (!color.isValid())
        return;
    game->setMasterColor(color);

    /* display specific color as icon on color button */
    QPixmap icon(16, 16);
    icon.fill(color);
    ui->colorSelectButton->setIcon(QIcon(icon));
}


void MainWindow::goGame() {
    /*
     *  mit entsprechendem Inhalt zu fuellen
     *
     */
}
