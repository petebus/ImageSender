#include <QtWidgets>
#include <QtGui>
#include "MyClient.h"

// ----------------------------------------------------------------------
MyClient::MyClient(const QString& strHost, int nPort, QWidget* pwgt /*=0*/) : QWidget(pwgt), m_pImage(nullptr), m_baType(""), m_nNextBlockSize(0)
{
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));

    m_ptxtInfo  = new QTextEdit;
    m_ptxtInfo->setReadOnly(true);

    if(m_pTcpSocket->waitForConnected())
        m_ptxtInfo->append("Connected to Server");
    else
    {
        QMessageBox::critical(this, "Error", QString("The following error occurred: %1.").arg(m_pTcpSocket->errorString()));
        exit(EXIT_FAILURE);
    }

    m_pLabel = new QLabel;
    m_pScrollArea = new QScrollArea(this);
    QPushButton* pimg = new QPushButton("&Select Image");
    connect(pimg, SIGNAL(clicked()), SLOT(selectImage()));

    QPushButton* pcmd = new QPushButton("&Send");
    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));

    QVBoxLayout* pvbxLayout = new QVBoxLayout;    
    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));
    pvbxLayout->addWidget(m_ptxtInfo);
    pvbxLayout->addWidget(pimg);
    pvbxLayout->addWidget(pcmd);
    pvbxLayout->addWidget(m_pScrollArea);
    setLayout(pvbxLayout);
}

// ----------------------------------------------------------------------
MyClient::~MyClient()
{
    if(m_pTcpSocket->isOpen())
        m_pTcpSocket->close();
}

// ----------------------------------------------------------------------
void MyClient::slotDisconnected()
{
    m_pTcpSocket->deleteLater();
    m_pTcpSocket = nullptr;

    QMessageBox::information(this, "Info", QString("Disconnected! You need to try to restart the client."));
    m_ptxtInfo->append("Disconnected! You need to try to restart the client.");
}

// ----------------------------------------------------------------------
void MyClient::slotReadyRead()
{
    QDataStream socketStream(m_pTcpSocket);
    socketStream.setVersion(QDataStream::Qt_6_4);

    socketStream.startTransaction();
    socketStream >> m_baRR;
    // here is we can ues 'mid' for split buffer on header and the other data
    if(!socketStream.commitTransaction())
    {
        m_ptxtInfo->append(QString("%1 :: Waiting for more data to come..").arg(m_pTcpSocket->socketDescriptor()));
        return;
    }

    QString res = QString("%1").arg(QString(m_baRR).split("|")[0]);
    m_ptxtInfo->append(res);
}

// ----------------------------------------------------------------------
void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError = 
        "Error: " + (err == QAbstractSocket::HostNotFoundError ? "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ? "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ? "The connection was refused." : QString(m_pTcpSocket->errorString()));
    m_ptxtInfo->append(strError);
}

// ----------------------------------------------------------------------
void MyClient::slotSendToServer()
{
    if(!m_pImage)
    {
        QMessageBox::critical(this, "Error", QString("The following error occurred: The image was not selected."));
        return;
    }

    if(m_pTcpSocket)
    {
        if(m_pTcpSocket->isOpen())
        {
            QDataStream socketStream(m_pTcpSocket);
            socketStream.setVersion(QDataStream::Qt_6_4);

            QBuffer buffer;
            QImageWriter writer(&buffer, m_baType);
            writer.write(m_pImage->toImage());

            QByteArray bytearray;
            QDataStream stream(&bytearray, QIODevice::WriteOnly);

            bytearray.append(buffer.data());

            socketStream << (quint32)bytearray.size();
            m_pTcpSocket->write(bytearray);
            m_pTcpSocket->waitForBytesWritten(500);
            m_pLabel->clear();
            delete m_pImage;
            m_pImage = nullptr;
        }
    }
}

// ------------------------------------------------------------------
void MyClient::slotConnected()
{
    m_ptxtInfo->append("Received the connected() signal");
}

// ----------------------------------------------------------------------
void MyClient::selectImage()
{
    QString m_sFilter = "*.jpg";
    QString str = QFileDialog::getOpenFileName(0, "Select image", QDir::currentPath(), "*.png ;; *.jpg ;; *.bmp", &m_sFilter);

    if (!str.isEmpty())
    {
        if (m_sFilter.contains("jpg") || m_sFilter.contains("bmp") || m_sFilter.contains("png"))
        {
            m_pLabel->clear();
            m_pImage = new QPixmap(str);
            m_pLabel->setPixmap(*m_pImage);
            m_pLabel->show();

            m_pScrollArea->setBackgroundRole(QPalette::Dark);
            m_pScrollArea->setWidget(m_pLabel);
            m_pScrollArea->setWidgetResizable(false);

            m_ptxtInfo->append("IMG selected: " + str);
            m_baType = m_sFilter.contains("jpg") ? "jpg" : m_sFilter.contains("png") ? "png" : "bmp";
        }
    }
}
