#pragma once

#include <QWidget>

class QTcpServer;
class QTextEdit;
class QTcpSocket;
class QScrollArea;
class QLabel;

// ======================================================================
class MyServer : public QWidget {
Q_OBJECT
private:
    QTcpServer*         m_ptcpServer = nullptr;
    QTextEdit*          m_ptxt = nullptr;
    quint16             m_nNextBlockSize;
    QByteArray          m_baRR;
    QLabel*             m_pLabel = nullptr;
    QScrollArea*        m_pScrollArea = nullptr;
    QSet<QTcpSocket*>   m_pConnectionSet;

private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);

public:
    explicit MyServer    (int nPort, QWidget* pwgt = 0);
    ~MyServer   ();

public slots:
    virtual void slotNewConnection  ();
            void slotReadClient     ();
            void appendToSocketList (QTcpSocket* socket);
            void discardSocket      ();
};

