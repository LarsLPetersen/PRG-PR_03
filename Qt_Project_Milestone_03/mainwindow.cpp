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
    ui->universeModeControl->addItem("Predator");

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
    connect(ui->lifetimeControl, SIGNAL(valueChanged(int)), game, SLOT(setLifetime(int)));

    /* combo boxes */
    connect(ui->universeModeControl, SIGNAL(currentIndexChanged(int)), game, SLOT(setUniverseMode(int)));
    connect(ui->universeModeControl, SIGNAL(currentIndexChanged(int)), this, SLOT(globalButtonControl(int)));
    connect(ui->cellModeControl, SIGNAL(currentIndexChanged(int)), game, SLOT(setCellMode(int)));

    /* enable/disable interaction during the game */
    connect(game, SIGNAL(gameStarted(int, bool)), this, SLOT(disableControls(int, bool)));
    connect(game, SIGNAL(universeModified(int, bool)), this, SLOT(disableControls(int, bool)));
    connect(game, SIGNAL(gameStopped(int, bool)), SLOT(enableControls(int, bool)));
    connect(game, SIGNAL(gameEnds(int, bool)), this, SLOT(enableControls(int,bool)));

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

    globalButtonControl(game->getUniverseMode());

    /* send keystrokes to snake game */
    KeyPressFilter *keyPressFilter = new KeyPressFilter(this->game);
    this->installEventFilter(keyPressFilter);
    connect(keyPressFilter, SIGNAL(keyPressed(int)), game, SLOT(calcDirectionSnake(int)));
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::globalButtonControl(int uM) {
    if (uM < 2) {
        ui->cellModeControl->clear();
        ui->cellModeControl->setDisabled(true);
        ui->lifetimeControl->clear();
        ui->lifetimeControl->setDisabled(true);
        ui->colorRandomButton->setDisabled(false);
        ui->colorSelectButton->setDisabled(false);
    }
    else {
        // cell mode choices
        ui->cellModeControl->addItem("Predator");
        ui->cellModeControl->addItem("Prey");
        ui->cellModeControl->addItem("Food");
        ui->cellModeControl->setEnabled(true);

        ui->lifetimeControl->setMinimum(1);
        ui->lifetimeControl->setMaximum(99);
        ui->lifetimeControl->setValue(50);
        ui->lifetimeControl->setEnabled(true);

        ui->colorRandomButton->setDisabled(true);
        ui->colorSelectButton->setDisabled(true);

    }
}


void MainWindow::enableControls(int uM, bool b) {
    ui->loadButton->setEnabled(b);
    ui->saveButton->setEnabled(b);
    ui->intervalControl->setEnabled(b);
    ui->universeSizeControl->setEnabled(b);
    ui->universeModeControl->setEnabled(b);

    if (uM == 2) {
        ui->cellModeControl->setEnabled(true);
        ui->lifetimeControl->setEnabled(true);
    }
}


void MainWindow::disableControls(int uM, bool b) {
    ui->loadButton->setDisabled(b);
    ui->saveButton->setDisabled(b);
    ui->intervalControl->setDisabled(b);
    ui->universeSizeControl->setDisabled(b);
    ui->universeModeControl->setDisabled(b);

    if (uM == 2) {
        ui->lifetimeControl->setDisabled(true);
    }
}


void MainWindow::saveGame() {
    int uM = game->getUniverseMode();
    QString filename, size, buffer;
    QColor color;
    QFile file;

    switch (uM) {

    // GAME OF LIFE
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

    //  SNAKE
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
                  QString::number(game->getCA().getSnakeLength()) + "\n" +
                  QString::number(game->getCA().getSnakeAction()) + "\n" +

                  QString::number(game->getCA().positionSnakeHead.x) + " " + QString::number(game->getCA().positionSnakeHead.y) + "\n" +
                  QString::number(game->getCA().positionFood.x) + " " + QString::number(game->getCA().positionFood.y) + "\n";
        file.write(buffer.toUtf8());
        buffer.clear();

        file.write(game->dumpGame().toUtf8());
        file.close();

        break;

    // PREDATOR
    case 2:
        filename = QFileDialog::getSaveFileName(this, tr("Save current game"),
                                                QDir::homePath(), tr("Predator *.predator Files (*.predator)"));
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

        buffer +=QString::number(ui->cellModeControl->currentIndex()) + "\n";

        file.write(buffer.toUtf8());
        buffer.clear();

        file.write(game->dumpGame().toUtf8());

        buffer +=QString::number(ui->lifetimeControl->value()) + "\n";

        file.write(buffer.toUtf8());
        buffer.clear();

        file.write(game->dumpGame('l').toUtf8());

        file.close();

        break;
    }
}


void MainWindow::loadGame() {

    int uM = game->getUniverseMode();
    QString filename;
    QFile file;

    switch (uM) {

    // GAME OF LIFE
    case 0:
        filename = QFileDialog::getOpenFileName(this, tr("Open saved game"),
                                                QDir::homePath(), tr("Game of Life File (*.game_of_life)"));
        break;

    // SNAKE
    case 1:
        filename = QFileDialog::getOpenFileName(this, tr("Open saved game"),
                                                QDir::homePath(), tr("Snake File (*.snake)"));
        break;

    // PREDATOR
    case 2:
        filename = QFileDialog::getOpenFileName(this, tr("Open saved game"),
                                                QDir::homePath(), tr("Predator File (*.predator)"));
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

    // GAME OF LIFE
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

    // SNAKE
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

    // PREDATOR
    case 2:
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

        file_input_stream >> r;
        ui->cellModeControl->setCurrentIndex(r);

        // reconstruct world
        dump = "";
        for (int k = 0; k != stream_value; k++) {
            file_input_stream >> tmp;
            dump.append(tmp + "\n");
        }
        game->reconstructGame(dump);

        file_input_stream >> r;
        ui->lifetimeControl->setValue(r);
        game->setLifetime(r);

        // reconstruct lifetime
        dump = "";
        for (int k = 0; k != stream_value; k++) {
            file_input_stream >> tmp;
            dump.append(tmp + "\n");
        }
        game->reconstructGame(dump, 'l');

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

    // display specific color as icon on color button
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

    // set cell color to specific rbg value
    QColor color(r, g, b);
    if (!color.isValid())
        return;
    game->setMasterColor(color);

    // display specific color as icon on color button
    QPixmap icon(16, 16);
    icon.fill(color);
    ui->colorSelectButton->setIcon(QIcon(icon));
}
