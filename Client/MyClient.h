#pragma once

#include <QWidget>
#include <QTcpSocket>

class QTextEdit;
class QLineEdit;
class QScrollArea;
class QLabel;

// ======================================================================
class MyClient : public QWidget {
Q_OBJECT
private:
    QTcpSocket*     m_pTcpSocket;
    QTextEdit*      m_ptxtInfo;
    QLineEdit*      m_ptxtInput;
    QPixmap*        m_pImage;
    QByteArray      m_baType;
    QByteArray      m_baRR;
    QLabel*         m_pLabel;
    QScrollArea*    m_pScrollArea;

public:
    explicit MyClient   (const QString& strHost, int nPort, QWidget* pwgt = 0) ;
    ~MyClient           ();

private slots:
    void slotReadyRead      ();
    void slotError          (QAbstractSocket::SocketError);
    void slotSendToServer   ();
    void slotConnected      ();
    void slotDisconnected   ();
    void selectImage        ();
};
