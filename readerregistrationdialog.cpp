#include "readerregistrationdialog.h"
#include "ui_readerregistrationdialog.h"
#include <QString>

ReaderRegistrationDialog::ReaderRegistrationDialog(QWidget *parent, bool * oper_success, int * reader_ticket_num) :
    QDialog(parent),
    ui(new Ui::ReaderRegistrationDialog)
{
    ui->setupUi(this);
    this->setFixedSize(200, 100);
    db = QSqlDatabase::addDatabase("QSQLITE", "only_once");
    db.setDatabaseName("./library.db");
    db.setConnectOptions("QSQLITE_ENABLE_REGEXP");

    if (db.open()) qDebug("база данных загружена");
    else qDebug("Произошла ошибка при загрузке базы данных");

    QSqlQuery query = QSqlQuery(db);
    query.exec("PRAGMA foreign_keys = ON;");

    successful_operation = oper_success;
    reader_tick_num = reader_ticket_num;
}

ReaderRegistrationDialog::~ReaderRegistrationDialog()
{
    delete ui;
}

void ReaderRegistrationDialog::on_buttonBox_accepted()
{
    QString ticket_num_text = ui->ticket_number->text();
    int ticket_num = ui->ticket_number->text().toInt();

    QSqlQuery query = QSqlQuery(db);
    query.exec("select exists(select * from Readers where [Номер читательского билета] = " + ticket_num_text + ");");
    query.first();
    int query_return_val = query.value(0).toInt();

    if (query_return_val)
    {
        *successful_operation = true;
        *reader_tick_num = ticket_num;
    }
    else
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Такого билета нет");
        mes_box.setFixedSize(500, 200);
    }
    db.removeDatabase("only_once");
}

