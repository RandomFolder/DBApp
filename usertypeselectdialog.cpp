#include "usertypeselectdialog.h"
#include "ui_usertypeselectdialog.h"
#include "readerregistrationdialog.h"

UserTypeSelectDialog::UserTypeSelectDialog(QWidget *parent, QString * userType, int * Reader_tick_num) :
    QDialog(parent),
    ui(new Ui::UserTypeSelectDialog)
{
    ui->setupUi(this);
    this->setFixedSize(400, 300);
    userT = userType;
    Reader_TN = Reader_tick_num;
}

UserTypeSelectDialog::~UserTypeSelectDialog()
{
    delete ui;
}

void UserTypeSelectDialog::on_readers_clicked()
{
    *userT = "Читатель";
    bool success = false;

    ReaderRegistrationDialog d = ReaderRegistrationDialog(0, &success, Reader_TN);
    d.exec();

    if (success)
        this->close();
    else *userT = "";
}


void UserTypeSelectDialog::on_library_worker_clicked()
{
    *userT = "Библиотекарь";
    this->close();
}


void UserTypeSelectDialog::on_administration_clicked()
{
    *userT = "Администрация";
    this->close();
}

