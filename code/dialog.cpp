#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->radioButton_add->setChecked(1);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_ok_clicked() {

}
