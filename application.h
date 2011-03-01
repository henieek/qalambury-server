#ifndef APPLICATION_H
#define APPLICATION_H

#include <QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QVector>
#include <QMap>
#include <QTimer>

class Application : public QCoreApplication
{
    Q_OBJECT
private:
    QList<QTcpSocket*> drawQueue;
    QTcpServer *server;
    QMap<QTcpSocket*,QString> connections;
    QMap<QTcpSocket*,unsigned short int> pendingConnections;
    QTimer  *drawTimer;
    void sendToAllExceptSender(QTcpSocket*,QByteArray);
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
    Application(int argc, char *argv[], QHostAddress hostname = QHostAddress::Any, int port = 9001);
    ~Application();
signals:
public slots:

};

#endif // APPLICATION_H
