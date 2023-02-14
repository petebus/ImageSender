#include <QtWidgets>
#include "MyServer.h"

// ----------------------------------------------------------------------
int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MyServer     server(5454);

    server.show();

    return app.exec();
}
