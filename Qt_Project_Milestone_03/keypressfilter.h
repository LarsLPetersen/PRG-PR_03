#ifndef KEYPRESSFILTER_H
#define KEYPRESSFILTER_H

#include <QObject>
#include <QtGui>
#include <QtCore>

class KeyPressFilter : public QObject
{
    Q_OBJECT

public:
    explicit KeyPressFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void keyPressed(const int&);

};

#endif // KEYPRESSFILTER_H
