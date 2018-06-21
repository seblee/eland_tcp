#include "tcp_ssl.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    tcp_ssl w;
    w.show();

    return a.exec();
}
