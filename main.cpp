/**
 * @file main.cpp
 * @brief Punkt wejścia aplikacji GoPoznań.
 * Uruchamia główne okno (MainWindow) oraz zarządza główną pętlą zdarzeń Qt.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
