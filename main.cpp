#include "home.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Load Stylesheet
    QFile file("style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        a.setStyleSheet(stream.readAll());
        file.close();
    }
    else
    {
        qDebug() << "Failed to load style.qss";
    }

    home w;
    w.show();
    return a.exec();
}
