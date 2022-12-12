#include <QtConcurrent>
#include "videoproxyserver.h"

void VideoProxyServer::incomingConnection(qintptr socketDescriptor)
{
    QtConcurrent::run(this, &VideoProxyServer::processSocket, socketDescriptor);
}

VideoProxyServer::VideoProxyServer(QObject *parent)
    : QTcpServer{parent}
{

}

void VideoProxyServer::processSocket(int socket)
{
    auto tcpSocket = new QTcpSocket();
    if (!tcpSocket->setSocketDescriptor(socket)) {
        qWarning() << "Error while try read socket descriptor: " << tcpSocket->errorString();
        return;
    }

    QSslSocket* innerTcpSocket = nullptr;
    QString currentPath = "";
    QByteArray host = "";

    auto localAddress = "localhost:" + QString::number(serverPort());

    int64_t totalReaded = 0;

    while (true) {
        tcpSocket->waitForReadyRead(500);
        auto bytesCount = tcpSocket->bytesAvailable();
        totalReaded += bytesCount;
        if (bytesCount == 0 || tcpSocket->atEnd()) {
            if (totalReaded == 0) closeSocket(tcpSocket); // if total read bytes was zero I don't think we can make something
            break;
        }

        auto bytes = tcpSocket->read(bytesCount);

        if (currentPath.isEmpty()) {
            auto routeData = getRoute(bytes);
            if (routeData.isEmpty()) {
                qWarning() << "Error while try read request first line from socket " << socket;
                closeSocket(tcpSocket);
                tcpSocket = nullptr;
                break;
            }
            auto startIndex = routeData.indexOf("/");
            host = routeData.mid(startIndex + 1, routeData.indexOf("/", startIndex + 1) - 1);
            currentPath = routeData;

            // we need make 302 request in first place

            auto inner302Socket = new QSslSocket();
            inner302Socket->setPeerVerifyMode(QSslSocket::QueryPeer);

            inner302Socket->connectToHostEncrypted(host, 443);
            if (!inner302Socket->waitForEncrypted(1000)) {
                tcpSocket->write("HTTP/1.1 204 No Content\r\nServer: AnilibriaQtInnerVideoProxy\r\nConnection: close\r\n\r\n");
                tcpSocket->waitForBytesWritten(1000);
                closeSocket(tcpSocket);
                tcpSocket = nullptr;
                break;
            }


            inner302Socket->write(bytes.replace("/" + host, "").replace(localAddress.toUtf8(), host));
            inner302Socket->waitForBytesWritten(1000);

            // and after it real request

            auto redirectData = readAllAvailableBytesFromSocket(inner302Socket);
            auto redirectDataString = QString(redirectData);
            auto indexLocation = redirectDataString.indexOf("Location: ") + 10;
            redirectDataString = redirectDataString.mid(indexLocation);
            QUrl url = QUrl(redirectDataString.mid(0, redirectDataString.indexOf(QString("\r\n").toUtf8())));

            innerTcpSocket = new QSslSocket();
            innerTcpSocket->setPeerVerifyMode(QSslSocket::QueryPeer);
            innerTcpSocket->connectToHostEncrypted(url.host(), 443);
            if (!innerTcpSocket->waitForEncrypted(1000)) {
                tcpSocket->write("HTTP/1.1 204 No Content\r\nServer: AnilibriaQtInnerVideoProxy\r\nConnection: close\r\n\r\n");
                tcpSocket->waitForBytesWritten(1000);
                closeSocket(tcpSocket);
                tcpSocket = nullptr;
                break;
            }

            auto path = url.path() + "?" + url.query();
            auto replacedBytes = bytes.replace(currentPath.replace("/" + host, "").toUtf8(), path.toUtf8()).replace(host, url.host().toUtf8());
            innerTcpSocket->write(replacedBytes);
            innerTcpSocket->waitForBytesWritten(1000);

            break;
        }

        if (innerTcpSocket != nullptr) {
            innerTcpSocket->write(bytes);
            innerTcpSocket->waitForBytesWritten(1000);
        }
    }

    if (innerTcpSocket == nullptr || tcpSocket == nullptr) {
        qWarning() << "Error while try read request headers from socket " << socket;
        return;
    }

    qint64 totalLoopBytes = 0;
    int tryTimes = 0;
    int readDelay = 1000;
    //auto isPlaylist = currentPath.contains(".m3u8");
    while (true) {
        innerTcpSocket->waitForReadyRead(readDelay);
        auto bytesCount = innerTcpSocket->bytesAvailable();
        if (totalLoopBytes == 0 && bytesCount == 0 && tryTimes < 5) {
            tryTimes += 1;
            readDelay = 1000 * tryTimes;
            continue;
        }
        if (bytesCount == 0 || innerTcpSocket->atEnd()) {
            auto restBytes = readAllAvailableBytesFromSocket(tcpSocket);
            if (restBytes.length() == 0) break;

            innerTcpSocket->write(/*isPlaylist ? bytes.replace("https://" + host, "http://" + localAddress.toUtf8()) :*/ restBytes);
            innerTcpSocket->waitForBytesWritten(2000);
            continue;
        }

        totalLoopBytes += bytesCount;

        auto bytesPart = innerTcpSocket->read(bytesCount);

        tcpSocket->write(/*isPlaylist ? bytes.replace("https://" + host, "http://" + localAddress.toUtf8()) : */ bytesPart);
        tcpSocket->waitForBytesWritten();
    }

    auto maxAttempts = 10;
    while (true) {
        if (maxAttempts <= 0) break;
        tcpSocket->waitForBytesWritten(1000);
        if (tcpSocket->bytesToWrite() > 0) {
            maxAttempts -= 1;
        } else {
            break;
        }
    }

    innerTcpSocket->disconnectFromHost();
    delete innerTcpSocket;

    closeSocket(tcpSocket);

    return;
}

QByteArray VideoProxyServer::getRoute(QByteArray bytes)
{
    auto index = bytes.indexOf("\r\n");
    auto routeHeader = bytes.mid(0, index);

    auto parts = routeHeader.split(' ');
    return parts.count() >= 2 ? parts[1] : "";
}

void VideoProxyServer::closeSocket(QTcpSocket *socket)
{
    socket->waitForBytesWritten(2000);
    socket->disconnectFromHost();
    delete socket;
}

QByteArray VideoProxyServer::readAllAvailableBytesFromSocket(QTcpSocket * socket)
{
    QByteArray result;

    while (true) {
        socket->waitForReadyRead(1000);
        auto bytesCount = socket->bytesAvailable();
        if (bytesCount == 0 || socket->atEnd()) return result;

        result.append(socket->read(bytesCount));
    }

    return result;
}
