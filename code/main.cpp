#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <map>
#include <fstream>
#include <cmath>
#include <unordered_map>

#include "mainwindow.h"
#include <QApplication>
#include <QFile>

using std::string;
using std::cin;
using std::cout;
using std::vector;
using std::pair;
using std::map;

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "Russian");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
