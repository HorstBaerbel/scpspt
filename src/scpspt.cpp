
#include <QtWidgets/QApplication>

#include "TerminalWidget.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(true);

    TerminalWidget testWidget;
    testWidget.show();

    return app.exec();
}
