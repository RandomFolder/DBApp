#ifndef USERTYPESELECTDIALOG_H
#define USERTYPESELECTDIALOG_H

#include <QDialog>

namespace Ui {
class UserTypeSelectDialog;
}

class UserTypeSelectDialog : public QDialog
{
    Q_OBJECT

public:
    UserTypeSelectDialog(QWidget *parent, QString * userType, int * Reader_tick_num);
    ~UserTypeSelectDialog();

private slots:
    void on_readers_clicked();

    void on_library_worker_clicked();

    void on_administration_clicked();

private:
    Ui::UserTypeSelectDialog *ui;
    QString * userT;
    int * Reader_TN;
};

#endif // USERTYPESELECTDIALOG_H
