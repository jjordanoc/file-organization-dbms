#include "widget.h"
#include <QHeaderView>
#include <QtConcurrent>
#include <QtGui>
#include "ExtendibleHashFile.hpp"
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
            this->displayRecords(res); \
        } else if (avl) { \
            std::cout << "Using AVL" << std::endl; \
            auto res = avl.search(attributeResult); \
            this->displayRecords(res); \
        } else { \
            std::cout << "Using linear search." << std::endl; \
            auto res = linear_search<attributeType, MovieRecord, decltype(index)>(FILENAME, \
                                                                                  attributeResult, \
                                                                                  index); \
            this->displayRecords(res); \
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
                           attributeCharSize, \
                           std::function<char *(MovieRecord &)>, \
                           std::function<bool(char[attributeCharSize], char[attributeCharSize])>, \
                           std::function<std::size_t(char[attributeCharSize])>> \
            extendible_hash_index{FILENAME, attribute2, isPrimaryKey, index, equal, hash}; \
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
            this->displayRecords(res); \
        } else { \
            std::cout << "Using linear search." << std::endl; \
            auto res = linear_search<char[attributeCharSize], \
                                     MovieRecord, \
                                     decltype(index), \
                                     decltype(equal)>(FILENAME, buf, index, equal); \
            this->displayRecords(res); \
        } \
    }

#define CREATE_INDEX_BY_ATTRIBUTE(attribute1, attribute2, attributeType, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        if (queryResult.indexValue == "Hash") { \
            std::function<attributeType(MovieRecord &)> index = [=](MovieRecord &record) { \
                return record.attribute1; \
            }; \
            ExtendibleHashFile<attributeType, MovieRecord> extendible_hash_index{FILENAME, \
                                                                                 attribute2, \
                                                                                 isPrimaryKey, \
                                                                                 index}; \
            if (extendible_hash_index) { \
                std::cout << attribute2 << " index already exists with Hash" << std::endl; \
            } else { \
                extendible_hash_index.create_index(); \
            } \
        } else if (queryResult.indexValue == "AVL") { \
            std::function<attributeType(MovieRecord &)> index = [](MovieRecord &movie) { \
                return movie.attribute1; \
            }; \
            AVLFile<attributeType, MovieRecord> avl(FILENAME, attribute2, isPrimaryKey, index); \
        } \
    }

#define CREATE_INDEX_BY_ATTRIBUTE_CHAR(attribute1, attribute2, attributeCharSize, isPrimaryKey) \
    if (queryResult.selectedAttribute == attribute2) { \
        if (queryResult.indexValue == "Hash") { \
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
                               attributeCharSize, \
                               std::function<char *(MovieRecord &)>, \
                               std::function<bool(char[attributeCharSize], char[attributeCharSize])>, \
                               std::function<std::size_t(char[attributeCharSize])>> \
                extendible_hash_index{FILENAME, attribute2, isPrimaryKey, index, equal, hash}; \
            extendible_hash_index.create_index(); \
        } \
    }

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("4^J DB");
    setWindowState(Qt::WindowMaximized);
    global = new QVBoxLayout(this);
    H1 = new QHBoxLayout();
    auto H2 = new QHBoxLayout();
    tabla = new QTableWidget();
    consulta = new QLineEdit();
    tiempoResult = new QLabel();
    result = new QLabel();
    boton = new QPushButton("Enviar");
    tabla->setColumnCount(11);
    tabla->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    consulta->setGeometry(0,0,200,200);
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
    connect(boton,SIGNAL(clicked()),this,SLOT(SetQuery()));
}

Widget::~Widget()
{
}

void Widget::SetQuery()
{
    QFuture<void> result = QtConcurrent::run([this]() {
        this->boton->setEnabled(false);
        this->tiempoResult->setText(tr("Ejecutando consulta..."));
        this->tiempoResult->setStyleSheet("color: blue;");
        auto operation = [&]() { this->execute_action(); };
        TimedResult r = time_function(operation);
        this->update_time(r);
        this->tiempoResult->setStyleSheet("color: black;");
        this->boton->setEnabled(true);
    });
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
    parserResult queryResult;
    try {
        queryResult = parsero.query(consulta->text().toStdString());
        result->setText(consulta->text());
        result->setStyleSheet("color: black;");
    } catch (std::runtime_error e) {
        result->setText("Sentencia SQL inválida.");
        result->setStyleSheet("color: red;");
    } catch (std::range_error e) {
        result->setText("Sentencia SQL inválida.");
        result->setStyleSheet("color: red;");
    }

    if(queryResult.queryType == "SELECT"){
        SELECT_BY_ATTRIBUTE(dataId, "dataId",int,true);
        SELECT_BY_ATTRIBUTE_CHAR(contentType, "contentType",16,false);
        SELECT_BY_ATTRIBUTE_CHAR(title, "title",256, false);
        SELECT_BY_ATTRIBUTE(length,"length",short, false);
        SELECT_BY_ATTRIBUTE(releaseYear, "releaseYear", short, false);
        SELECT_BY_ATTRIBUTE(endYear, "endYear", short, false);
        SELECT_BY_ATTRIBUTE(votes, "votes",int,false);
        SELECT_BY_ATTRIBUTE(rating, "rating",short,false);
        SELECT_BY_ATTRIBUTE(gross, "gross",int,false);
        SELECT_BY_ATTRIBUTE_CHAR(certificate, "certificate", 16, false);
        SELECT_BY_ATTRIBUTE_CHAR(description,"description", 512,false);
    }
    else if(queryResult.queryType == "CREATE"){
        CREATE_INDEX_BY_ATTRIBUTE(dataId, "dataId",int,true);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(contentType, "contentType",16,false);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(title, "title",256, false);
        CREATE_INDEX_BY_ATTRIBUTE(length,"length",short, false);
        CREATE_INDEX_BY_ATTRIBUTE(releaseYear, "releaseYear", short, false);
        CREATE_INDEX_BY_ATTRIBUTE(endYear, "endYear", short, false);
        CREATE_INDEX_BY_ATTRIBUTE(votes, "votes",int,false);
        CREATE_INDEX_BY_ATTRIBUTE(rating, "rating",short,false);
        CREATE_INDEX_BY_ATTRIBUTE(gross, "gross",int,false);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(certificate, "certificate", 16, false);
        CREATE_INDEX_BY_ATTRIBUTE_CHAR(description,"description", 512,false);
    }
}



template<typename T>
void Widget::update_time(TimedResult<T> &r)
{
    QString timeText = QString::number(r.duration / 1000) + " ms";
    tiempoResult->setText(timeText);
}
