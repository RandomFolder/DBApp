#ifndef READERREGISTRATIONDIALOG_H
#define READERREGISTRATIONDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QLineEdit>
#include <QVariant>
#include <QList>
#include <QDateTime>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

namespace Ui {
class ReaderRegistrationDialog;
}

class ReaderRegistrationDialog : public QDialog
{
    Q_OBJECT

public:
    ReaderRegistrationDialog(QWidget *parent, bool * oper_success, int * reader_ticket_num);
    ~ReaderRegistrationDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ReaderRegistrationDialog *ui;
    QSqlDatabase db;
    bool * successful_operation;
    int * reader_tick_num;
};

#endif // READERREGISTRATIONDIALOG_H
