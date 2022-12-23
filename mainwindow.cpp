#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usertypeselectdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    user_type = "";
    reader_ticket_num = -1;
    reader_hall_num = -1;
    UserTypeSelectDialog dialog = UserTypeSelectDialog(this, &user_type, &reader_ticket_num);
    dialog.exec();

    if (user_type == "") exit(0);

    ui->setupUi(this);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./library.db");
    db.setConnectOptions("QSQLITE_ENABLE_REGEXP");

    if (db.open()) qDebug("база данных загружена");
    else qDebug("Произошла ошибка при загрузке базы данных");

    QSqlQuery query = QSqlQuery(db);
    query.exec("PRAGMA foreign_keys = ON;");

    if (reader_ticket_num != -1)
    {
        QString reader_ticket_str;
        reader_ticket_str.setNum(reader_ticket_num);
        query.exec("select [№ зала] from Readers where [Номер читательского билета] = " + reader_ticket_str + ";");
        query.first();
        reader_hall_num = query.value(0).toInt();
    }

    model_books = new QSqlQueryModel(this);
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);
    model_readers = new QSqlQueryModel(this);
    model_readers->setQuery("select * from Readers;", db);
    model_halls = new QSqlQueryModel();
    model_halls->setQuery("select * from Halls;", db);
    model_search = new QSqlQueryModel(this);

    ui->tableView->setModel(model_books);
    ui->tableView_2->setModel(model_readers);
    ui->tableView_3->setModel(model_halls);
    ui->tableView_4->setModel(model_search);

    halls_lineedits = {ui->hall_number, ui->hall_name, ui->hall_capacity};
    books_lineedits = {ui->author, ui->bookname, ui->release_year, ui->code};
    bookshalls_lineedit = {ui->book_code, ui->book_hall_num, ui->book_count};
    readers_lineedits = {ui->ticket_number, ui->surname, ui->phone_number, ui->reader_hall_number};
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_addButton1_clicked() //Добавление книги
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString author = ui->author->text();
    QString book_name = ui->bookname->text();
    QString release_year = ui->release_year->text();
    QString code = ui->code->text();

    QSqlQuery query = QSqlQuery(db);

    query.prepare("insert into Books([Автор], [Название], [Год издания], [Шифр]) values(?, ?, ?, ?);");
    query.addBindValue(author);
    query.addBindValue(book_name);
    query.addBindValue(release_year);
    query.addBindValue(code);

    if (!query.exec())
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);
}


void MainWindow::on_addButton3_clicked()  //Добавление зала
{
    if (user_type != "Администрация")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    int hall_num = ui->hall_number->text().toInt();
    QString hall_name = ui->hall_name->text();
    int hall_cap = ui->hall_capacity->text().toInt();

    QSqlQuery query = QSqlQuery(db);

    query.prepare("insert into Halls([Номер зала], [Название], [Вместимость]) values(?, ?, ?);");
    query.addBindValue(hall_num);
    query.addBindValue(hall_name);
    query.addBindValue(hall_cap);

    query.exec();
    model_halls->setQuery("select * from Halls;", db);
}


void MainWindow::on_deleteButton3_clicked()  //Удаление зала
{
    if (user_type != "Администрация")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QList<QString> param_names = {};
    QList<QVariant> param_vals = {};
    QList<QVariant> lineedit_values = {ui->hall_number->text().toInt(), ui->hall_name->text(), ui->hall_capacity->text().toInt()};
    QList<int> param_indecies = {};

    for (int i = 0; i < halls_lineedits.size(); i++)
        if (!halls_lineedits[i]->text().isEmpty())
        {
            param_names.append(halls_lineedits[i]->placeholderText());
            param_indecies.append(i);
        }

    for (int i = 0; i < param_indecies.size(); i++)
        param_vals.append(lineedit_values[param_indecies[i]]);

    QString query_string = "delete from Halls where ";
    for (int i = 0; i < param_names.size(); i++)
    {
        QString condition = "";
        if (typeid(param_vals[i]) == typeid(int)) condition = "[" + param_names[i] + "]" + " = " + param_vals[i].toString();
        else condition = "[" + param_names[i] + "]" + " = '" + param_vals[i].toString() + "'";
        if (i != param_names.size() - 1) condition += " AND ";
        else condition += ";";

        query_string += condition;
    }

    qDebug() << query_string;

    QSqlQuery query = QSqlQuery(db);
    if (!query.exec(query_string))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }
    model_halls->setQuery("select * from Halls;", db);
    model_readers->setQuery("select * from Readers;", db);
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);

}


void MainWindow::on_deleteButton2_clicked()  //Удаление Читателя
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QList<QString> param_names = {};
    QList<QVariant> param_vals = {};
    QList<QVariant> lineedit_values = {ui->ticket_number->text().toInt(), ui->surname->text(), ui->phone_number->text(), ui->reader_hall_number->text().toInt()};
    QList<int> param_indecies = {};

    for (int i = 0; i < readers_lineedits.size(); i++)
        if (!readers_lineedits[i]->text().isEmpty())
        {
            param_names.append(readers_lineedits[i]->placeholderText());
            param_indecies.append(i);
        }

    for (int i = 0; i < param_indecies.size(); i++)
        param_vals.append(lineedit_values[param_indecies[i]]);

    QString query_string = "delete from Readers where ";
    for (int i = 0; i < param_names.size(); i++)
    {
        QString condition = "";
        if (typeid(param_vals[i]) == typeid(int)) condition = "[" + param_names[i] + "]" + " = " + param_vals[i].toString();
        else condition = "[" + param_names[i] + "]" + " = '" + param_vals[i].toString() + "'";
        if (i != param_names.size() - 1) condition += " AND ";
        else condition += ";";

        query_string += condition;
    }

    qDebug() << query_string;

    QSqlQuery query = QSqlQuery(db);
    if (!query.exec(query_string))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }
    model_readers->setQuery("select * from Readers;", db);
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);

}


void MainWindow::on_updateButton_clicked()  //Обновление шифра книги
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString code = ui->code->text();
    QString old_code = ui->old_code->text();
    QString query_text = "update Books set [Шифр] = '" + code + "' where [Шифр] = '" + old_code + "';";
    qDebug() << query_text;

    QSqlQuery query = QSqlQuery(db);

    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);
}


void MainWindow::on_deleteButton1_clicked()  //Удаление книги
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QList<QString> param_names = {};
    QList<QVariant> param_vals = {};
    QList<QVariant> lineedit_values = {ui->author->text(), ui->bookname->text(), ui->release_year->text(), ui->code->text()};
    QList<int> param_indecies = {};

    for (int i = 0; i < books_lineedits.size(); i++)
        if (!books_lineedits[i]->text().isEmpty())
        {
            param_names.append(books_lineedits[i]->placeholderText());
            param_indecies.append(i);
        }

    for (int i = 0; i < param_indecies.size(); i++)
        param_vals.append(lineedit_values[param_indecies[i]]);

    QString query_string = "delete from Books where ";
    for (int i = 0; i < param_names.size(); i++)
    {
        QString condition = "";
        if (typeid(param_vals[i]) == typeid(int)) condition = "[" + param_names[i] + "]" + " = " + param_vals[i].toString();
        else condition = "[" + param_names[i] + "]" + " = '" + param_vals[i].toString() + "'";
        if (i != param_names.size() - 1) condition += " AND ";
        else condition += ";";

        query_string += condition;
    }

    qDebug() << query_string;

    QSqlQuery query = QSqlQuery(db);
    if (!query.exec(query_string))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);
}


void MainWindow::on_addButton2_clicked()  //Добавление читателя
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    int hall_num = ui->reader_hall_number->text().toInt();
    int ticket_num = ui->ticket_number->text().toInt();
    QString surname = ui->surname->text();
    QString phonenumber = ui->phone_number->text();
    QDateTime cur_date = QDateTime::currentDateTime();
    QString date_str = cur_date.date().toString("yyyy-MM-dd");

    QSqlQuery query = QSqlQuery(db);

    QList<QVariant> params = {ticket_num, surname, phonenumber, date_str, hall_num};
    QString query_text = "insert into Readers values (";
    for (int i = 0; i < params.size(); i++)
    {
        if (typeid(params[i]) == typeid(int)) query_text += params[i].toString();
        else query_text += "'" + params[i].toString() + "'";
        if (i < params.size() - 1) query_text += ", ";
        else query_text += ");";
    }

    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }

    model_readers->setQuery("select * from Readers;", db);
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);

}


void MainWindow::on_addButton5_clicked()  //Добавление книги в зал
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString book_code = ui->book_code->text();
    int book_hall_num = ui->book_hall_num->text().toInt();
    int book_count = ui->book_count->text().toInt();

    QSqlQuery query = QSqlQuery(db);

    query.prepare("insert into BooksHalls([Шифр книги], [№ зала], [Количество экземпляров]) values(?, ?, ?);");
    query.addBindValue(book_code);
    query.addBindValue(book_hall_num);
    query.addBindValue(book_count);

    if (!query.exec())
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);
}


void MainWindow::on_deleteButton5_clicked()  //Удаление книги из зала
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QList<QString> param_names = {};
    QList<QVariant> param_vals = {};
    QList<QVariant> lineedit_values = {ui->book_code->text(), ui->book_hall_num->text().toInt(), ui->book_count->text().toInt()};
    QList<int> param_indecies = {};

    for (int i = 0; i < bookshalls_lineedit.size(); i++)
        if (!bookshalls_lineedit[i]->text().isEmpty())
        {
            param_names.append(bookshalls_lineedit[i]->placeholderText());
            param_indecies.append(i);
        }

    for (int i = 0; i < param_indecies.size(); i++)
        param_vals.append(lineedit_values[param_indecies[i]]);

    QString query_string = "delete from BooksHalls where ";
    for (int i = 0; i < param_names.size(); i++)
    {
        QString condition = "";
        if (typeid(param_vals[i]) == typeid(int)) condition = "[" + param_names[i] + "]" + " = " + param_vals[i].toString();
        else condition = "[" + param_names[i] + "]" + " = '" + param_vals[i].toString() + "'";
        if (i != param_names.size() - 1) condition += " AND ";
        else condition += ";";

        query_string += condition;
    }

    qDebug() << query_string;

    QSqlQuery query = QSqlQuery(db);
    if (!query.exec(query_string))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }
    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);

}


void MainWindow::on_addButton4_clicked()  //Бронирование книги читателем
{
    if (user_type != "Читатель")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не читатель");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString book_code = ui->readerbook_code->text();
    QString ticket_num_str, hall_num_str;
    ticket_num_str.setNum(reader_ticket_num);
    hall_num_str.setNum(reader_hall_num);
    QDateTime cur_date = QDateTime::currentDateTime();
    QString date_str = cur_date.date().toString("yyyy-MM-dd");

    QSqlQuery query = QSqlQuery(db);
    query.exec("select [Читательский билет] from ReadersBooks where [Шифр книги] = '" + book_code + "';");
    QList<QString> ticket_num_list = {};
    while (query.next())
        ticket_num_list.append(query.value(0).toString());

    query.exec("select [Количество экземпляров] from BooksHalls where [Шифр книги] = '" + book_code + "' AND [№ зала] = " + hall_num_str + ";");
    query.first();
    int book_count = query.value(0).toInt();  //Количество имеющихся в зале экземпляров

    QList<int> hall_num_list = {};
    for (int i = 0; i < ticket_num_list.size(); i++)
    {
        query.exec("select [№ зала] from Readers where [Номер читательского билета] = " + ticket_num_list[i] + ";");
        query.first();
        hall_num_list.append(query.value(0).toInt());
    }

    int count = 0;  //Количество забронированныз книг
    for (int i = 0; i < hall_num_list.size(); i++)
        if (hall_num_list[i] == reader_hall_num) count++;

    if (book_count == count)
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Эта книга не доступна");
        mes_box.setFixedSize(500, 200);
        return;
    }
    else
    {
        query.prepare("insert into ReadersBooks([Шифр книги], [Читательский билет], [Дата закрепления за читателем]) values (?, ?, ?);");
        query.addBindValue(book_code);
        query.addBindValue(reader_ticket_num);
        query.addBindValue(date_str);

        if (!query.exec())
        {
            QSqlError error = query.lastError();
            qDebug() << error.text();
        }
        model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);

    }
}


void MainWindow::on_deleteButton4_clicked()  //Возвращение забронированной книги
{
    if (user_type != "Читатель")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не читатель");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString book_code = ui->readerbook_code->text();
    QString ticket_num_str;
    ticket_num_str.setNum(reader_ticket_num);

    QSqlQuery query = QSqlQuery(db);
    QString query_text = "delete from ReadersBooks where [Шифр книги] = '" + book_code + "' AND [Читательский билет] = " + ticket_num_str + ";";
    qDebug() << query_text;
    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }

    model_books->setQuery("select Books.[Автор], Books.[Название], Books.[Год издания], Books.[Шифр], BooksHalls.[№ зала], BooksHalls.[Количество экземпляров], ReadersBooks.[Читательский билет], ReadersBooks.[Дата закрепления за читателем] from Books LEFT OUTER JOIN BooksHalls on Books.[Шифр] = BooksHalls.[Шифр книги] LEFT OUTER JOIN ReadersBooks on Books.[Шифр] = ReadersBooks.[Шифр книги] AND BooksHalls.[№ зала] = (SELECT Readers.[№ зала] FROM Readers WHERE Readers.[Номер читательского билета] = ReadersBooks.[Читательский билет]);", db);

}


void MainWindow::on_search_booksButton_clicked()  //Поиск закреплённых за читателем книг
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString reader_ticket = ui->ticket->text();
    QSqlQuery query = QSqlQuery(db);
    QString query_text = "select [Шифр книги] from ReadersBooks where [Читательский билет] = " + reader_ticket + ";";

    qDebug() << query_text;
    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }

    model_search->setQuery(query_text);
}


void MainWindow::on_search_booknamesButton_clicked()  //Поиск названия книги по автору
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString author = ui->Author->text();
    QSqlQuery query = QSqlQuery(db);
    QString query_text = "select [Название] from Books where [Автор] = '" + author + "';";

    qDebug() << query_text;
    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }

    model_search->setQuery(query_text);
}


void MainWindow::on_search_bookcodeButton_clicked()  //Поиск шифра по названию книги
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString book_name = ui->Bookname->text();
    QSqlQuery query = QSqlQuery(db);
    QString query_text = "select [Шифр] from Books where [Название] = '" + book_name + "';";

    qDebug() << query_text;
    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }

    model_search->setQuery(query_text);
}


void MainWindow::on_search_dateButton_clicked()  //Дата закрепления книги за читателем
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString ticket = ui->ticket2->text();
    QString book_code = ui->Bookcode->text();
    QSqlQuery query = QSqlQuery(db);
    QString query_text = "select [Дата закрепления за читателем] from ReadersBooks where [Шифр книги] = '" + book_code + "' AND [Читательский билет] = " + ticket + ";";

    qDebug() << query_text;
    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }

    model_search->setQuery(query_text);
}


void MainWindow::on_search_readers_countButton_clicked()  //Вывод числа читателей
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QSqlQuery query = QSqlQuery(db);
    QString query_text = "select COUNT(*) from Readers;";

    if (!query.exec(query_text))
    {
        QSqlError error = query.lastError();
        qDebug() << error.text();
    }

    model_search->setQuery(query_text);
}


void MainWindow::on_give_list_of_booksButton_clicked()  //Выдача справки о наличии книг Автора в зале
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QString author = ui->Author_2->text();
    QString hall_num = ui->Hall_num->text();

    QSqlQuery query = QSqlQuery(db);

    query.exec("select [Шифр книги] from BooksHalls where [№ зала] = " + hall_num + ";");
    QList<QString> book_code_list = {};
    while (query.next())
        book_code_list.append(query.value(0).toString());

    QList<QString> book_code_list2 = {};
    for (int i = 0; i < book_code_list.size(); i++)
    {
        query.exec("select [Автор] from Books where [Шифр] = '" + book_code_list[i] + "';");
        query.first();
        QString author_name = query.value(0).toString();
        if (author_name == author) book_code_list2.append(book_code_list[i]);
    }

    QList<QString> book_name_and_code_list = {};
    for (int i = 0; i < book_code_list2.size(); i++)
    {
        query.exec("select [Название] from Books where [Шифр] = '" + book_code_list2[i] + "';");
        query.first();
        QString name_and_code = query.value(0).toString() + " (" + book_code_list2[i] + ")";
        book_name_and_code_list.append(name_and_code);
    }

    if (book_name_and_code_list.size() == 0)
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Книг данного автора в данном зале нет");
        mes_box.setFixedSize(500, 200);
        return;
    }
    else
    {
        QFile saveFile("./" + author + " (книги).txt");

        if (!saveFile.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox::critical(0, "Ошибка", "Не удалось создать файл");
            return;
        }

        QTextStream out(&saveFile);
        out << "Книги автора '" + author + "':" << '\n';
        for (int i = 0; i < book_name_and_code_list.size(); i++) out << book_name_and_code_list[i] << '\n';

        saveFile.flush();
        saveFile.close();
    }
}


void MainWindow::on_month_reportButton_clicked()  //Отчёт за месяц
{
    if (user_type != "Библиотекарь")
    {
        QMessageBox mes_box;
        mes_box.critical(this, "Ошибка", "Вы не обладаете нужными правами");
        mes_box.setFixedSize(500, 200);
        return;
    }

    QDateTime cur_date = QDateTime::currentDateTime();
    QString date_str = cur_date.date().toString("yyyy-MM-dd");
    QFile saveFile("./" + date_str + ".txt");

    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(0, "Ошибка", "Не удалось создать файл");
        return;
    }

    QTextStream out(&saveFile);
    out.setCodec("UTF-8");

    QSqlQuery query(db);
    query.exec("select COUNT(*) from Readers;");
    query.first();
    out << "Number of readers as a whole: " + query.value(0).toString() << "\n\n";

    query.exec("select COUNT(*) from Readers where strftime('%m', [Дата записи]) = strftime('%m', date('now'));");
    query.first();
    out << "Readers joined this month: " + query.value(0).toString() << "\n\n";
    out << "Books taken this month:\n";

    query.exec("select [Шифр книги] from ReadersBooks where strftime('%m', [Дата закрепления за читателем]) = strftime('%m', date('now'));");
    QList<QString> code_list = {};
    while (query.next())
        code_list.append(query.value(0).toString());

    QMap<QString, int> count_of_books;
    for (int i = 0; i < code_list.size(); i++) count_of_books[code_list[i]]++;

    QMapIterator<QString, int> iterator(count_of_books);
    while (iterator.hasNext())
    {
        iterator.next();
        out << iterator.key() << ' ' << iterator.value() << '\n';
    }
    out.flush();

    out << "\nTicket numbers of those who didn't take books this month:" << '\n';
    out.flush();
    query.exec("select [Номер читательского билета] from Readers;");
    QList<QString> ticket_list = {};
    while (query.next()) ticket_list.append(query.value(0).toString());

    for (int i = 0; i < ticket_list.size(); i++)
    {
        query.exec("select " + ticket_list[i] + " IN (select [Читательский билет] from ReadersBooks where strftime('%m', [Дата закрепления за читателем]) = strftime('%m', date('now')))");
        query.first();
        if (query.value(0).toInt() == 0) out << ticket_list[i] << '\n';
    }

    saveFile.flush();
    saveFile.close();
}

