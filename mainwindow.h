#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QList>
#include <QVariant>
#include <QLineEdit>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QMapIterator>
#include <QIODevice>
#include <QTextCodec>

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

    void on_addButton1_clicked();

    void on_addButton3_clicked();

    void on_deleteButton3_clicked();

    void on_deleteButton2_clicked();

    void on_updateButton_clicked();

    void on_deleteButton1_clicked();

    void on_addButton2_clicked();

    void on_addButton5_clicked();

    void on_deleteButton5_clicked();

    void on_addButton4_clicked();

    void on_deleteButton4_clicked();

    void on_search_booksButton_clicked();

    void on_search_booknamesButton_clicked();

    void on_search_bookcodeButton_clicked();

    void on_search_dateButton_clicked();

    void on_search_readers_countButton_clicked();

    void on_give_list_of_booksButton_clicked();

    void on_month_reportButton_clicked();

private:
    QString user_type;
    int reader_ticket_num, reader_hall_num;
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlQueryModel * model_books;
    QSqlQueryModel * model_readers;
    QSqlQueryModel * model_halls;
    QSqlQueryModel * model_search;
    QList<QLineEdit*> halls_lineedits;
    QList<QLineEdit*> books_lineedits;
    QList<QLineEdit*> bookshalls_lineedit;
    QList<QLineEdit*> readers_lineedits;
};
#endif // MAINWINDOW_H
