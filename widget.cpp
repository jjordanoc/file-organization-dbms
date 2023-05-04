#include "widget.h"
#include <QHeaderView>
#include <QtConcurrent>
#include <QtGui>
#include "ExtendibleHashFile.hpp"
#include "ISAM.hpp"
#include "MovieRecord.h"
#include "avl.hpp"
#include "parser.hpp"
#include "utils.hpp"

const std::string FILENAME = "database/movies_and_series.dat";

#define SELECT_BY_ATTRIBUTE(attribute1, attribute2, attributeType, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        std::function<attributeType(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        ExtendibleHashFile<attributeType, MovieRecord> extendible_hash_index{FILENAME, \
                                                                             attribute2, \
                                                                             isPrimaryKey, \
                                                                             index}; \
        AVLFile<attributeType, MovieRecord> avl(FILENAME, attribute2, isPrimaryKey, index); \
        auto attributeResult = static_cast<attributeType>( \
            std::stof(queryResult.atributos[queryResult.selectedAttribute])); \
        if (extendible_hash_index) { \
            std::cout << "Using Hash" << std::endl; \
            auto res = extendible_hash_index.search(attributeResult); \
            queryRecords = res; \
        } else if (avl) { \
            std::cout << "Using AVL" << std::endl; \
            auto res = avl.search(attributeResult); \
            queryRecords = res; \
        } else { \
            std::cout << "Using linear search." << std::endl; \
            auto res = linear_search<attributeType, MovieRecord, decltype(index)>(FILENAME, \
                                                                                  attributeResult, \
                                                                                  index); \
            queryRecords = res; \
        } \
    }

#define SELECT_BY_ATTRIBUTE_CHAR(attribute1, attribute2, attributeCharSize, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        std::function<bool(char[attributeCharSize], char[attributeCharSize])> equal = \
            [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
            return std::string(a) == std::string(b); \
        }; \
        std::function<char *(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        std::hash<std::string> hasher; \
        std::function<std::size_t(char[attributeCharSize])> hash = \
            [&hasher](char key[attributeCharSize]) { return hasher(std::string(key)); }; \
        ExtendibleHashFile<char[attributeCharSize], \
                           MovieRecord, \
                           16, \
                           std::function<char *(MovieRecord &)>, \
                           std::function<bool(char[attributeCharSize], char[attributeCharSize])>, \
                           std::function<std::size_t(char[attributeCharSize])>> \
            extendible_hash_index{FILENAME, attribute2, isPrimaryKey, index, equal, hash}; \
        std::function<bool(char[attributeCharSize], char[attributeCharSize])> greater = \
            [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
            return std::string(a) > std::string(b); \
        }; \
        AVLFile<char[attributeCharSize], MovieRecord, decltype(index), decltype(greater)> \
            avl(FILENAME, attribute2, false, index, greater); \
        std::string attributeResult = queryResult.atributos[queryResult.selectedAttribute]; \
        std::cout << "Result from parser: " << attributeResult << std::endl; \
        char buf[attributeCharSize]; \
        int i = 0; \
        for (; i <= attributeResult.length() - 3; i++) { \
            buf[i] = attributeResult[i + 1]; \
        } \
        buf[i] = '\0'; \
        std::cout << "After removing things: " << buf << std::endl; \
        if (extendible_hash_index) { \
            std::cout << "Using Hash" << std::endl; \
            auto res = extendible_hash_index.search(buf); \
            queryRecords = res; \
        } else if (avl) { \
            std::cout << "Using AVL" << std::endl; \
            auto res = avl.search(buf); \
            queryRecords = res; \
        } else { \
            std::cout << "Using linear search." << std::endl; \
            auto res = linear_search<char[attributeCharSize], \
                                     MovieRecord, \
                                     decltype(index), \
                                     decltype(equal)>(FILENAME, buf, index, equal); \
            queryRecords = res; \
        } \
    }

#define SELECT_BY_RANGE(attribute1, attribute2, attributeType, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        std::function<attributeType(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        AVLFile<attributeType, MovieRecord> avl(FILENAME, attribute2, isPrimaryKey, index); \
        auto rangeStart = static_cast<attributeType>(std::stof(queryResult.range1)); \
        auto rangeEnd = static_cast<attributeType>(std::stof(queryResult.range2)); \
        if (avl) { \
            std::cout << "Using AVL" << std::endl; \
            auto res = avl.range_search(rangeStart, rangeEnd); \
            queryRecords = res; \
        } else { \
            std::cout << "Using linear search." << std::endl; \
            auto res = range_search<attributeType, MovieRecord, decltype(index)>(FILENAME, \
                                                                                 rangeStart, \
                                                                                 rangeEnd, \
                                                                                 index); \
            queryRecords = res; \
        } \
    }

#define CREATE_INDEX_BY_ATTRIBUTE(attribute1, attribute2, attributeType, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        if (queryResult.indexValue == "ISAM") { \
            std::function<int(MovieRecord &)> index = [=](MovieRecord &record) { \
                return record.dataId; \
            }; \
            ISAM<true, int, MovieRecord> isam{FILENAME, "dataId", index}; \
            if (!isam) { \
                isam.create_index(); \
            } else { \
                std::cout << "ISAM index already created." << std::endl; \
            } \
        } else if (queryResult.indexValue == "Hash") { \
            std::function<attributeType(MovieRecord &)> index = [=](MovieRecord &record) { \
                return record.attribute1; \
            }; \
            ExtendibleHashFile<attributeType, MovieRecord> extendible_hash_index{FILENAME, \
                                                                                 attribute2, \
                                                                                 isPrimaryKey, \
                                                                                 index}; \
            if (extendible_hash_index) { \
                std::cout << attribute2 << "Index already exists with Hash" << std::endl; \
            } else { \
                extendible_hash_index.create_index(); \
            } \
        } else if (queryResult.indexValue == "AVL") { \
            std::function<attributeType(MovieRecord &)> index = [](MovieRecord &movie) { \
                return movie.attribute1; \
            }; \
            AVLFile<attributeType, MovieRecord> avl(FILENAME, attribute2, isPrimaryKey, index); \
            if (avl) { \
                std::cout << attribute2 << "Index already exists with AVL" << std::endl; \
            } else { \
                avl.create_index(); \
            } \
        } \
    }

#define CREATE_INDEX_BY_ATTRIBUTE_CHAR(attribute1, attribute2, attributeCharSize, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        std::function<char *(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        if (queryResult.indexValue == "Hash") { \
            std::function<bool(char[attributeCharSize], char[attributeCharSize])> equal = \
                [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
                return std::string(a) == std::string(b); \
            }; \
            std::hash<std::string> hasher; \
            std::function<std::size_t(char[attributeCharSize])> hash = \
                [&hasher](char key[attributeCharSize]) { return hasher(std::string(key)); }; \
            ExtendibleHashFile<char[attributeCharSize], \
                               MovieRecord, \
                               16, \
                               std::function<char *(MovieRecord &)>, \
                               std::function<bool(char[attributeCharSize], char[attributeCharSize])>, \
                               std::function<std::size_t(char[attributeCharSize])>> \
                extendible_hash_index{FILENAME, attribute2, isPrimaryKey, index, equal, hash}; \
            if (extendible_hash_index) { \
                std::cout << attribute2 << "Index already exists with Hash" << std::endl; \
            } else { \
                extendible_hash_index.create_index(); \
            } \
        } else if (queryResult.indexValue == "AVL") { \
            std::function<bool(char[attributeCharSize], char[attributeCharSize])> greater = \
                [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
                return std::string(a) > std::string(b); \
            }; \
            AVLFile<char[attributeCharSize], MovieRecord, decltype(index), decltype(greater)> \
                avl(FILENAME, attribute2, false, index, greater); \
            if (avl) { \
                std::cout << attribute2 << "Index already exists with AVL" << std::endl; \
            } else { \
                avl.create_index(); \
            } \
        } \
    }

#define DELETE_BY_ATTRIBUTE(attribute1, attribute2, attributeType, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        std::function<attributeType(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        ExtendibleHashFile<attributeType, MovieRecord> extendible_hash_index{FILENAME, \
                                                                             attribute2, \
                                                                             isPrimaryKey, \
                                                                             index}; \
        AVLFile<attributeType, MovieRecord> avl(FILENAME, attribute2, isPrimaryKey, index); \
        auto attributeResult = static_cast<attributeType>( \
            std::stof(queryResult.atributos[queryResult.selectedAttribute])); \
        if (extendible_hash_index) { \
            std::cout << "Using Hash" << std::endl; \
            extendible_hash_index.remove(attributeResult); \
        } else if (avl) { \
            std::cout << "Using AVL" << std::endl; \
            avl.remove(attributeResult); \
        } else { \
            std::cout << "Couldn't delete due to lack of strategy information." << std::endl; \
        } \
    }

#define DELETE_BY_ATTRIBUTE_CHAR(attribute1, attribute2, attributeCharSize, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        std::function<bool(char[attributeCharSize], char[attributeCharSize])> equal = \
            [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
            return std::string(a) == std::string(b); \
        }; \
        std::function<char *(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        std::hash<std::string> hasher; \
        std::function<std::size_t(char[attributeCharSize])> hash = \
            [&hasher](char key[attributeCharSize]) { return hasher(std::string(key)); }; \
        ExtendibleHashFile<char[attributeCharSize], \
                           MovieRecord, \
                           16, \
                           std::function<char *(MovieRecord &)>, \
                           std::function<bool(char[attributeCharSize], char[attributeCharSize])>, \
                           std::function<std::size_t(char[attributeCharSize])>> \
            extendible_hash_index{FILENAME, attribute2, isPrimaryKey, index, equal, hash}; \
        std::function<bool(char[attributeCharSize], char[attributeCharSize])> greater = \
            [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
            return std::string(a) > std::string(b); \
        }; \
        AVLFile<char[attributeCharSize], MovieRecord, decltype(index), decltype(greater)> \
            avl(FILENAME, attribute2, false, index, greater); \
        std::string attributeResult = queryResult.atributos[queryResult.selectedAttribute]; \
        std::cout << "Result from parser: " << attributeResult << std::endl; \
        char buf[attributeCharSize]; \
        int i = 0; \
        for (; i <= attributeResult.length() - 3; i++) { \
            buf[i] = attributeResult[i + 1]; \
        } \
        buf[i] = '\0'; \
        std::cout << "After removing things: " << buf << std::endl; \
        if (extendible_hash_index) { \
            std::cout << "Using Hash" << std::endl; \
            extendible_hash_index.remove(buf); \
        } else if (avl) { \
            std::cout << "Using AVL" << std::endl; \
            avl.remove(buf); \
        } else { \
            std::cout << "Falta linear search." << std::endl; \
        } \
    }

#define INSERT_VALUE_BY_ATTRIBUTE(attribute1, \
                                  attribute2, \
                                  attributeType, \
                                  isPrimaryKey, \
                                  recordInserted, \
                                  pos) \
    if (true) { \
        std::function<attributeType(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        ExtendibleHashFile<attributeType, MovieRecord> extendible_hash_index{FILENAME, \
                                                                             attribute2, \
                                                                             isPrimaryKey, \
                                                                             index}; \
        AVLFile<attributeType, MovieRecord> avl(FILENAME, attribute2, isPrimaryKey, index); \
        if (extendible_hash_index) { \
            std::cout << "Inserting value in index " << attribute2 << " hash"; \
            extendible_hash_index.insert(recordInserted, pos); \
        } \
        if (avl) { \
            std::cout << "Inserting value in index " << attribute2 << " avl"; \
            avl.insert(index(recordInserted), pos); \
        } \
    }

#define INSERT_VALUE_BY_ATTRIBUTE_CHAR(attribute1, \
                                       attribute2, \
                                       attributeCharSize, \
                                       isPrimaryKey, \
                                       recordInserted, \
                                       pos) \
    if (true) { \
        std::function<char *(MovieRecord &)> index = [=](MovieRecord &record) { \
            return record.attribute1; \
        }; \
        std::function<bool(char[attributeCharSize], char[attributeCharSize])> equal = \
            [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
            return std::string(a) == std::string(b); \
        }; \
        std::hash<std::string> hasher; \
        std::function<std::size_t(char[attributeCharSize])> hash = \
            [&hasher](char key[attributeCharSize]) { return hasher(std::string(key)); }; \
        ExtendibleHashFile<char[attributeCharSize], \
                           MovieRecord, \
                           16, \
                           std::function<char *(MovieRecord &)>, \
                           std::function<bool(char[attributeCharSize], char[attributeCharSize])>, \
                           std::function<std::size_t(char[attributeCharSize])>> \
            extendible_hash_index{FILENAME, attribute2, isPrimaryKey, index, equal, hash}; \
        std::function<bool(char[attributeCharSize], char[attributeCharSize])> greater = \
            [](char a[attributeCharSize], char b[attributeCharSize]) -> bool { \
            return std::string(a) > std::string(b); \
        }; \
        AVLFile<char[attributeCharSize], MovieRecord, decltype(index), decltype(greater)> \
            avl(FILENAME, attribute2, false, index, greater); \
        if (extendible_hash_index) { \
            std::cout << "Inserting value in index " << attribute2 << " hash"; \
            extendible_hash_index.insert(recordInserted, pos); \
        } \
        if (avl) { \
            std::cout << "Inserting value in index " << attribute2 << " avl"; \
            avl.insert(index(recordInserted), pos); \
        } \
    }

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("4^J DB");
    setWindowState(Qt::WindowMaximized);
    global = new QVBoxLayout(this);
    H1 = new QHBoxLayout();
    H2 = new QHBoxLayout();
    tabla = new QTableWidget();
    consulta = new QLineEdit();
    tiempoResult = new QLabel();
    result = new QLabel();
    boton = new QPushButton("Enviar");
    tabla->setColumnCount(11);
    tabla->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tabla->setHorizontalHeaderLabels(QStringList{tr("dataId"),
                                                 tr("contentType"),
                                                 tr("title"),
                                                 tr("length"),
                                                 tr("releaseYear"),
                                                 tr("endYear"),
                                                 tr("votes"),
                                                 tr("rating"),
                                                 tr("gross"),
                                                 tr("certificate"),
                                                 tr("description")});
    consulta->setGeometry(0, 0, 200, 200);
    consulta->setPlaceholderText("Ingrese consulta: ");
    H1->addWidget(consulta);
    H1->addWidget(boton);
    H2->addWidget(result);
    H2->addStretch();
    H2->addWidget(tiempoResult);
    tiempoResult->setText("0 s");
    global->addLayout(H1);
    global->addWidget(tabla);
    global->addLayout(H2);
    connect(boton, SIGNAL(clicked()), this, SLOT(SetQuery()));
    connect(&this->futureWatcher, &QFutureWatcher<void>::finished, this, &Widget::onQueryFinished);
    connect(&this->futureWatcher, &QFutureWatcher<void>::started, this, &Widget::onQueryStarted);
    std::function<int(MovieRecord &)> index = [=](MovieRecord &record) { return record.dataId; };
    ExtendibleHashFile<int, MovieRecord> extendible_hash_data_id{FILENAME, "dataId", true, index};
    if (!extendible_hash_data_id) {
        extendible_hash_data_id.create_index();
    }
}

Widget::~Widget()
{
}

void Widget::SetQuery()
{
    try {
        queryResult = parsero.query(consulta->text().toStdString());
        std::cout << consulta->text().toStdString() << std::endl;
        std::cout << queryResult.selectedAttribute << std::endl;
        result->setText(consulta->text());
        result->setStyleSheet("color: black;");

    } catch (std::runtime_error e) {
        result->setText("Sentencia SQL inválida.");
        result->setStyleSheet("color: red;");
    }
    QFuture<TimedResult<void>> result = QtConcurrent::run([this]() -> TimedResult<void> {
        auto operation = [&]() { this->execute_action(); };
        TimedResult r = time_function(operation);
        return r;
    });
    futureWatcher.setFuture(result);
}

void Widget::onQueryFinished()
{
    TimedResult r = futureWatcher.result();
    this->update_time(r);
    this->displayRecords(queryRecords);
    this->tiempoResult->setStyleSheet("color: black;");
    this->boton->setEnabled(true);
    queryResult.killSelf();
}

void Widget::onQueryStarted()
{
    this->boton->setEnabled(false);
    this->tiempoResult->setText(tr("Ejecutando consulta..."));
    this->tiempoResult->setStyleSheet("color: blue;");
}

void Widget::displayRecords(std::vector<MovieRecord> &records)
{
    tabla->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        tabla->setItem(i, 0, new QTableWidgetItem(QString::number(records[i].dataId)));
        tabla->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(records[i].contentType)));
        tabla->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(records[i].title)));
        tabla->setItem(i, 3, new QTableWidgetItem(QString::number(records[i].length)));
        tabla->setItem(i, 4, new QTableWidgetItem(QString::number(records[i].releaseYear)));
        tabla->setItem(i, 5, new QTableWidgetItem(QString::number(records[i].endYear)));
        tabla->setItem(i, 6, new QTableWidgetItem(QString::number(records[i].votes)));
        tabla->setItem(i, 7, new QTableWidgetItem(QString::number(records[i].rating)));
        tabla->setItem(i, 8, new QTableWidgetItem(QString::number(records[i].gross)));
        tabla->setItem(i, 9, new QTableWidgetItem(QString::fromStdString(records[i].certificate)));
        tabla->setItem(i, 10, new QTableWidgetItem(QString::fromStdString(records[i].description)));
    }
}

void Widget::execute_action()
{
    if(queryResult.queryType == "SELECT"){
        if (queryResult.range1 == "") {
            SELECT_BY_ATTRIBUTE(dataId, "dataId", int, true);
            SELECT_BY_ATTRIBUTE_CHAR(contentType, "contentType", 16, false);
            SELECT_BY_ATTRIBUTE_CHAR(title, "title", 256, false);
            SELECT_BY_ATTRIBUTE(length, "length", short, false);
            SELECT_BY_ATTRIBUTE(releaseYear, "releaseYear", short, false);
            SELECT_BY_ATTRIBUTE(endYear, "endYear", short, false);
            SELECT_BY_ATTRIBUTE(votes, "votes", int, false);
            SELECT_BY_ATTRIBUTE(rating, "rating", float, false);
            SELECT_BY_ATTRIBUTE(gross, "gross", int, false);
            SELECT_BY_ATTRIBUTE_CHAR(certificate, "certificate", 16, false);
            SELECT_BY_ATTRIBUTE_CHAR(description, "description", 512, false);
        } else {
            SELECT_BY_RANGE(dataId, "dataId", int, true);
            SELECT_BY_RANGE(length, "length", short, false);
            SELECT_BY_RANGE(releaseYear, "releaseYear", short, false);
            SELECT_BY_RANGE(endYear, "endYear", short, false);
            SELECT_BY_RANGE(votes, "votes", int, false);
            SELECT_BY_RANGE(rating, "rating", float, false);
            SELECT_BY_RANGE(gross, "gross", int, false);
        }
    }
    else if(queryResult.queryType == "CREATE"){
        CREATE_INDEX_BY_ATTRIBUTE(dataId, "dataId",int,true);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(contentType, "contentType",16,false);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(title, "title",256, false);
        CREATE_INDEX_BY_ATTRIBUTE(length,"length",short, false);
        CREATE_INDEX_BY_ATTRIBUTE(releaseYear, "releaseYear", short, false);
        CREATE_INDEX_BY_ATTRIBUTE(endYear, "endYear", short, false);
        CREATE_INDEX_BY_ATTRIBUTE(votes, "votes",int,false);
        CREATE_INDEX_BY_ATTRIBUTE(rating, "rating", float, false);
        CREATE_INDEX_BY_ATTRIBUTE(gross, "gross",int,false);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(certificate, "certificate", 16, false);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(description,"description", 512,false);
    } else if (queryResult.queryType == "DELETE") {
        DELETE_BY_ATTRIBUTE(dataId, "dataId", int, true);
        DELETE_BY_ATTRIBUTE_CHAR(contentType, "contentType", 16, false);
        DELETE_BY_ATTRIBUTE_CHAR(title, "title", 256, false);
        DELETE_BY_ATTRIBUTE(length, "length", short, false);
        DELETE_BY_ATTRIBUTE(releaseYear, "releaseYear", short, false);
        DELETE_BY_ATTRIBUTE(endYear, "endYear", short, false);
        DELETE_BY_ATTRIBUTE(votes, "votes", int, false);
        DELETE_BY_ATTRIBUTE(rating, "rating", float, false);
        DELETE_BY_ATTRIBUTE(gross, "gross", int, false);
        DELETE_BY_ATTRIBUTE_CHAR(certificate, "certificate", 16, false);
        DELETE_BY_ATTRIBUTE_CHAR(description, "description", 512, false);

    } else if (queryResult.queryType == "INSERT") {
        MovieRecord record;
        record.dataId = stoi(queryResult.atributos["dataId"]);

        std::string tmp = queryResult.atributos["contentType"];
        char str[16];
        int i = 0;
        for (; i <= tmp.length() - 3; i++) {
            str[i] = tmp[i + 1];
        }
        str[i] = '\0';

        for (int i = 0; i <= 15; i++) {
            record.contentType[i] = str[i];
        }

        tmp = queryResult.atributos["title"];
        char str2[256];
        i = 0;
        for (; i <= tmp.length() - 3; i++) {
            str2[i] = tmp[i + 1];
        }
        str2[i] = '\0';

        for (int i = 0; i <= 255; i++) {
            record.title[i] = str2[i];
        }

        record.length = stoi(queryResult.atributos["length"]);
        record.releaseYear = stoi(queryResult.atributos["releaseYear"]);
        record.endYear = stoi(queryResult.atributos["endYear"]);
        record.votes = stoi(queryResult.atributos["votes"]);
        record.rating = stof(queryResult.atributos["rating"]);
        record.gross = stoi(queryResult.atributos["gross"]);
        record.removed = false;

        tmp = queryResult.atributos["certificate"];
        char str3[16];
        i = 0;
        for (; i <= tmp.length() - 3; i++) {
            str3[i] = tmp[i + 1];
        }
        str3[i] = '\0';

        for (int i = 0; i <= 15; i++) {
            record.certificate[i] = str3[i];
        }

        tmp = queryResult.atributos["description"];
        char str4[512];
        i = 0;
        for (; i <= tmp.length() - 3; i++) {
            str4[i] = tmp[i + 1];
        }
        str4[i] = '\0';

        for (int i = 0; i <= 511; i++) {
            record.description[i] = str4[i];
        }
        std::function<int(MovieRecord &)> index = [=](MovieRecord &record) { return record.dataId; };
        ExtendibleHashFile<int, MovieRecord> extendible_hash_data_id{FILENAME,
                                                                     "dataId",
                                                                     true,
                                                                     index};
        if (extendible_hash_data_id) {
            auto res = extendible_hash_data_id.search(index(record));
            if (!res.empty()) {
                std::cout << "Duplicate primary key." << std::endl;
                return;
            }
        } else {
            std::cout << "Error" << std::endl;
        }
        long position = insert_register_on_file<MovieRecord>(FILENAME, record);
        INSERT_VALUE_BY_ATTRIBUTE(dataId, "dataId", int, true, record, position);
        INSERT_VALUE_BY_ATTRIBUTE_CHAR(contentType, "contentType", 16, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE_CHAR(title, "title", 256, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE(length, "length", short, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE(releaseYear, "releaseYear", short, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE(endYear, "endYear", short, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE(votes, "votes", int, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE(rating, "rating", short, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE(gross, "gross", int, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE_CHAR(certificate, "certificate", 16, false, record, position);
        INSERT_VALUE_BY_ATTRIBUTE_CHAR(description, "description", 512, false, record, position);
    }
}

template<typename T>
void Widget::update_time(TimedResult<T> &r)
{
    QString timeText = QString::number(r.duration / 1000) + " ms";
    tiempoResult->setText(timeText);
}
