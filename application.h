#ifndef APPLICATION_H
#define APPLICATION_H

#include <QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QVector>
#include <QMap>
#include <QTimer>

#include "dictonary.h"

class Application : public QCoreApplication
{
    Q_OBJECT
private:
    int drawInterval;
    int port;
    QList<QTcpSocket*> drawQueue;
    QTcpServer *server;
    QMap<QTcpSocket*,QString> connections;
    QMap<QTcpSocket*,unsigned short int> pendingConnections;
    QTimer  *drawTimer;
    void sendToAllExceptSender(QTcpSocket*,QByteArray);
    bool argumentsPositive(int,int);
    void sendServerSettings(QTcpSocket*);
    Dictonary *dictonary;
private slots:
    void newConnection();
    void someoneSentData();
    void registerLogin(QTcpSocket*,QString);
    void receiveChatMessage(QTcpSocket*,QByteArray);
    void sendNicknamesToClient(QTcpSocket*);
    void clientDisconnected();
    void enqueueClient(QTcpSocket*);
    void dequeueClient(QTcpSocket*);
    void drawTimeout();
public:
    Application(int argc, char *argv[], QHostAddress hostname = QHostAddress::Any);
    ~Application();
signals:
public slots:

};

#endif // APPLICATION_H
