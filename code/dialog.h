#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QRadioButton>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

public:
    QPushButton* pushButton_ok;
    QRadioButton* radioButton_not_save;
    QRadioButton* radioButton_save;
    QRadioButton* radioButton_add;

public:
    Ui::Dialog *ui;
private slots:
    void on_pushButton_ok_clicked();
};

#endif // DIALOG_H
