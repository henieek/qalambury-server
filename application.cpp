#include "application.h"

#include <QDebug>

Application::Application(int argc, char *argv[], QHostAddress hostname) :
        QCoreApplication(argc,argv)
{
    if(argc == 2 && argumentsPositive(atoi(argv[1]),atoi(argv[2]))) {
        this->port = atoi(argv[1]);
        this->drawInterval = atoi(argv[2]);
    }
    else {
        this->port = 9001;
        this->drawInterval = 10000; // 10s
    }

	this->dictonary = new Dictonary("dict.txt");

    this->server = new QTcpServer();
    this->drawTimer = new QTimer();
    connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    connect(drawTimer,SIGNAL(timeout()),this,SLOT(drawTimeout()));
    this->server->listen(hostname,this->port);
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
	delete dictonary;
}

bool Application::argumentsPositive(int port, int timeInterval) {
    return port > 0 && port < 65535 && timeInterval > 0;
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
    this->sendServerSettings(sock);
    this->sendNicknamesToClient(sock);
    this->sendToAllExceptSender(sock,QString("log\n" + loginName + "\n").toStdString().c_str());
}

void Application::sendServerSettings(QTcpSocket *sock) {
    sock->write(QString("settings\n" + QString::number(this->drawInterval) + "\n").toAscii());
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
    this->drawQueue.removeAll(senderSock);
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
		QString word = this->dictonary->nextWord();
        static int number = 1; // tymczasowo
        this->drawTimer->setInterval(this->drawInterval);
        QTcpSocket *drawSocket = this->drawQueue.takeFirst();
        number++; // tymczasowo
        // tymczasowo
        drawSocket->write(QString("drawstart\n" + word + "\n").toAscii());
        this->drawQueue.push_back(drawSocket);
        this->drawTimer->start();
    }
}
