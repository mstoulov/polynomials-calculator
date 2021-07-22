#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

struct Letter_List {
    char letter;
    Letter_List* next;
};

struct Terms_List {
    std::vector<int> degrees;
    Terms_List* next;
};

struct List_Of_Polynomials {
    int serial_number;
    int max_degree_sum;
    Terms_List* terms_list;
    int letter_number;
    Letter_List* letter_list;
    List_Of_Polynomials* next;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:

    void on_pushButton_save_res_1_clicked();

    void on_pushButton_plus_clicked();

    void on_pushButton_minus_clicked();

    void on_pushButton_multiply_clicked();

    void on_pushButton_divide_clicked();

    void on_pushButton_value_in_point_clicked();

    void on_pushButton_roots_clicked();

    void on_pushButton_derivative_clicked();

    void on_pushButton_save_res_2_clicked();

    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_pushButton_make_polynom_out_of_existing_clicked();

    void on_pushButton_clear_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_delete_all_clicked();

    void on_pushButton_import_clicked();

public:
    QWidget *centralwidget;
    QListWidget *listWidget;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_multiply;
    QPushButton *pushButton_divide;
    QPushButton *pushButton_roots;
    QPushButton *pushButton_derivative;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_plus;
    QPushButton *pushButton_minus;
    QPushButton *pushButton_value_in_point;
    QPushButton *pushButton_delete;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_make_polynom_out_of_existing;
    QPushButton *pushButton_delete_all;
    QWidget *layoutWidget1;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QHBoxLayout *horizontalLayout_5;
    QLineEdit *lineEdit;
    QPushButton *pushButton_save_res_1;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *lineEdit_2;
    QPushButton *pushButton_save_res_2;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QPushButton *pushButton_import;
    QPushButton *pushButton_clear;

public:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
