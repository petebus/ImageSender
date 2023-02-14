#include <QtNetwork>
#include <QtWidgets>
#include "MyServer.h"

// ----------------------------------------------------------------------
MyServer::MyServer(int nPort, QWidget* pwgt /*=0*/) : QWidget(pwgt), m_nNextBlockSize(0)
{
    m_ptcpServer = new QTcpServer(this); 
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort))
    {
        QMessageBox::critical(0, "Server Error", "Unable to start the server:" + m_ptcpServer->errorString());
        m_ptcpServer->close();
        return;
    }
    connect(m_ptcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

    m_ptxt = new QTextEdit;
    m_ptxt->setReadOnly(true);
    m_pLabel = new QLabel;
    m_pScrollArea = new QScrollArea(this);

    QVBoxLayout* pvbxLayout = new QVBoxLayout;    
    pvbxLayout->addWidget(new QLabel("<H1>Server</H1>"));
    pvbxLayout->addWidget(m_ptxt);
    pvbxLayout->addWidget(m_pScrollArea);
    setLayout(pvbxLayout);
}

// ----------------------------------------------------------------------
MyServer::~MyServer()
{
    foreach (QTcpSocket* pSocket, m_pConnectionSet)
    {
        pSocket->close();
        pSocket->deleteLater();
    }

    m_ptcpServer->close();
    m_ptcpServer->deleteLater();
}

// ----------------------------------------------------------------------
void MyServer::slotNewConnection()
{
    while (m_ptcpServer->hasPendingConnections())
        appendToSocketList(m_ptcpServer->nextPendingConnection());
}

// ----------------------------------------------------------------------
void MyServer::appendToSocketList(QTcpSocket* pSocket)
{
    m_pConnectionSet.insert(pSocket);
    connect(pSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(discardSocket()));
    m_ptxt->append(QString::number(pSocket->socketDescriptor()));
    sendToClient(pSocket, QString("Client with socket:%1 has just entered the room").arg(pSocket->socketDescriptor()));
}

// ----------------------------------------------------------------------
void MyServer::discardSocket()
{
    QTcpSocket* pSocket = reinterpret_cast<QTcpSocket*>(sender());
    auto it = m_pConnectionSet.find(pSocket);
    if (it != m_pConnectionSet.end())
    {
        m_ptxt->append(QString("A client has just left the room"));
        m_pConnectionSet.remove(*it);
    }

    pSocket->deleteLater();
}

// ----------------------------------------------------------------------
void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = reinterpret_cast<QTcpSocket*>(sender());

    QDataStream socketStream(pClientSocket);
    socketStream.setVersion(QDataStream::Qt_6_4);

    socketStream.startTransaction();
    socketStream >> m_baRR;

    if(!socketStream.commitTransaction())
    {
        m_ptxt->append(QString("%1 :: Waiting for more data to come..").arg(pClientSocket->socketDescriptor()));
        return;
    }

    QPixmap pixmap;
    pixmap.loadFromData(m_baRR);
    m_ptxt->append(QString("%1 :: Image received w/h = %2/%3").arg(pClientSocket->socketDescriptor()).arg(pixmap.width()).arg(pixmap.height()));
    m_pLabel->clear();
    m_pLabel->setPixmap(pixmap);
    m_pLabel->show();

    m_pScrollArea->setBackgroundRole(QPalette::Dark);
    m_pScrollArea->setWidget(m_pLabel);
    m_pScrollArea->setWidgetResizable(false);

    m_nNextBlockSize = 0;
    m_baRR.clear();

    sendToClient(pClientSocket, "Server received your image");
}

// ----------------------------------------------------------------------
void MyServer::sendToClient(QTcpSocket* pSocket, const QString& str)
{
    if(pSocket)
    {
        if(pSocket->isOpen())
        {
            QDataStream socketStream(pSocket);
            socketStream.setVersion(QDataStream::Qt_6_4);

            QByteArray header;
            header.prepend(QString("%1").arg(QString(str + "|")).toUtf8());
            header.resize(64);

            QByteArray barr;
            QDataStream stream(&barr, QIODevice::WriteOnly);
            barr.prepend(header);

            socketStream << (quint32)barr.size();
            pSocket->write(barr);
            pSocket->waitForBytesWritten(500);
        }
    }
}
