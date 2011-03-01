#include "application.h"

#include <QDebug>

Application::Application(int argc, char *argv[], QHostAddress hostname, int port) :
        QCoreApplication(argc,argv)
{
    this->server = new QTcpServer();
    this->drawTimer = new QTimer();
    connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    connect(drawTimer,SIGNAL(timeout()),this,SLOT(drawTimeout()));
    this->server->listen(hostname,port);
}

Application::~Application() {
    QMap<QTcpSocket*,QString>::const_iterator it;
    for(it=this->connections.begin();it!=this->connections.end();++it) {
        QTcpSocket *key = it.key();
        this->connections.remove(key);
        delete key;
    }
    this->server->close();
    delete server;
    delete drawTimer;
}

void Application::newConnection() {
    while(this->server->hasPendingConnections()) {
        QTcpSocket *newSock = this->server->nextPendingConnection();
        connect(newSock,SIGNAL(readyRead()),this,SLOT(someoneSentData()));
        connect(newSock,SIGNAL(disconnected()),this,SLOT(clientDisconnected()));
        this->pendingConnections.insert(newSock,0);
    }
}

void Application::someoneSentData() {
    QTcpSocket *senderSock = dynamic_cast<QTcpSocket*>(this->sender());
    bool login = false;
    while(senderSock->bytesAvailable()) {
        QByteArray byteArray;
        byteArray = senderSock->readLine();
        if(login) {
            this->registerLogin(senderSock,QString(byteArray.replace(QString("\n"),QByteArray(""))));
            login = false;
            continue;
        }
        if(byteArray == "log\n") {
            login = true;
            continue;
        }
        if(byteArray == "chat\n") {
            this->receiveChatMessage(senderSock,senderSock->readLine());
            continue;
        }
        if(byteArray == "toggle\n") {
            this->enqueueClient(senderSock);
            continue;
        }
        if(byteArray == "untoggle\n") {
            this->dequeueClient(senderSock);
            continue;
        }
        this->sendToAllExceptSender(senderSock,byteArray);
    }
}

void Application::registerLogin(QTcpSocket *sock, QString loginName) {
    this->pendingConnections.remove(sock);
    this->connections.insert(sock,loginName);
    this->sendNicknamesToClient(sock);
    this->sendToAllExceptSender(sock,QString("log\n" + loginName + "\n").toStdString().c_str());
}

void Application::sendToAllExceptSender(QTcpSocket *mySock, QByteArray data) {
    QMap<QTcpSocket*,QString>::const_iterator it;
    for(it = this->connections.begin();it!=this->connections.end();++it) {
        if(it.key() != mySock) {
            it.key()->write(data);
        }
    }
}

void Application::receiveChatMessage(QTcpSocket *sock, QByteArray byteArray) {
    QString nickname = this->connections[sock];
    QString message = "chat\n" + nickname + "\n" + QString(byteArray) + "\n";
    qDebug() << "Wiadomosc od: " + nickname;
    this->sendToAllExceptSender(sock,message.toStdString().c_str());
}

void Application::sendNicknamesToClient(QTcpSocket *sock) {
    sock->write("list start\n");
    QMap<QTcpSocket*,QString>::const_iterator it;
    for(it = this->connections.begin();it!=this->connections.end();++it) {
        sock->write(QString((*it) + "\n").toAscii());
    }
    sock->write("list end\n");
}

void Application::clientDisconnected() {
    QTcpSocket *senderSock = dynamic_cast<QTcpSocket*>(sender());
    QString nickname = this->connections[senderSock];
    this->sendToAllExceptSender(senderSock,QString("out\n" + nickname + "\n").toAscii());
    this->connections.remove(senderSock);
}

void Application::enqueueClient(QTcpSocket *sock) {
    this->drawQueue.push_back(sock);
    if(drawQueue.size() == 1) {
        this->drawTimeout();
    }
}

void Application::dequeueClient(QTcpSocket *sock) {
    this->drawQueue.removeAll(sock);
}

void Application::drawTimeout() {
    if(drawQueue.size() > 0) {
        static int number = 1; // tymczasowo
        this->drawTimer->setInterval(2000);
        QTcpSocket *drawSocket = this->drawQueue.takeFirst();
        number++; // tymczasowo
        // tymczasowo
        drawSocket->write(QString("drawstart\nslowo" + QString::number(number) + "\n").toAscii());
        this->drawQueue.push_back(drawSocket);
        this->drawTimer->start();
    }
}
