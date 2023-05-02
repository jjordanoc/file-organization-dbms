#include "widget.h"
#include "parser.h"
#include "ExtendibleHashFile.hpp"
#include "MovieRecord.h"
#include <QtGui>
#include <QHeaderView>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Proyecto");
    setWindowState(Qt::WindowMaximized);
    global = new QVBoxLayout(this);
    H1 = new QHBoxLayout();
    tabla = new QTableWidget();
    consulta = new QLineEdit();
    result = new QLabel();
    boton = new QPushButton("Enviar");
    tabla->setColumnCount(11);
    tabla->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //------------------------------
    consulta->setGeometry(0,0,200,200);
    consulta->setPlaceholderText("Ingrese consulta: ");
    H1->addWidget(consulta);
    H1->addWidget(boton);
    global->addLayout(H1);
    global->addWidget(tabla);
    global->addWidget(result);
    //-------------------------------
    //Setear el numero de filas
    //tabla->setRowCount()
    //-------------------------------
    connect(boton,SIGNAL(clicked()),this,SLOT(SetQuery()));
}

Widget::~Widget()
{
}

void Widget::SetQuery(){
    parserResult queryResult;
    try {
        queryResult = parsero.query(consulta->text().toStdString());
        result->setText(consulta->text());
        result->setStyleSheet("color: black");
    }
    catch(std::runtime_error e) {
        result->setText("Sentencia SQL inválida.");
        result->setStyleSheet("color: red;");
    }

    if(queryResult.queryType == "SELECT"){
        if(queryResult.selectedAttribute == "dataId"){
            std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
                return record.dataId;
            };
            ExtendibleHashFile<int, MovieRecord> extendible_hash_data_id{"movies_and_series.dat", "data_id", true, index};
            if(extendible_hash_data_id){
                auto res = extendible_hash_data_id.search(stoi(queryResult.atributos[queryResult.selectedAttribute]));
                this->displayRecords(res);
                for (auto &record: res) {
                    std::cout << record.to_string() << std::endl;
//                    tabla->insertRow()
                }
            }

        }

    }
    else if(queryResult.queryType == "CREATE"){
        if(queryResult.selectedAttribute == "dataId"){
            if(queryResult.indexValue == "Hash"){
                std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
                    return record.dataId;
                };
                ExtendibleHashFile<int, MovieRecord> extendible_hash_data_id{"movies_and_series.dat", "data_id", true, index};
                extendible_hash_data_id.create_index();
            }
        }
    }
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


