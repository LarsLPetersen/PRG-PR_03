#include "keypressfilter.h"
#include "gamewidget.h"


KeyPressFilter::KeyPressFilter(QObject *parent) : QObject(parent)
{ // default constructor
}

bool KeyPressFilter::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();

        /* act when one of the relevant keys 2, 4, 6, 8 is pressed */
        if (key == Qt::Key_2) {
            emit keyPressed(2);
        } else if (key == Qt::Key_8) {
            emit keyPressed(8);
        } else if (key == Qt::Key_4) {
            emit keyPressed(4);
        } else if (key == Qt::Key_6) {
            emit keyPressed(6);
        }
        return true;
    } else {
        /* standard event processing */
        return QObject::eventFilter(watched, event);
    }
}
