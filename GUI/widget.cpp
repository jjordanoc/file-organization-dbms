#include "widget.h"
#include "parser.h"
#include "ExtendibleHashFile.h"
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
    tabla->setColumnCount(15);
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
    result->setText(consulta->text());
    parserResult queryResult;
    queryResult = parsero.query(consulta->text().toStdString());

    if(queryResult.queryType == "SELECT"){
        if(queryResult.selectedAttribute == "dataId"){
            std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
                return record.dataId;
            };
            ExtendibleHashFile<int, MovieRecord> extendible_hash_data_id{"movies_and_series.dat", "data_id", true, index};
            if(extendible_hash_data_id){
                auto res = extendible_hash_data_id.search(stoi(queryResult.atributos[queryResult.selectedAttribute]));
                for (auto &record: res) {
                    std::cout << record.to_string() << std::endl;
                }
            }

        }

    }
    else if(queryResult.queryType == "CREATE"){
        if(queryResult.selectedAttribute == "dataId"){
            if(queryResult.indexValue == "Hash"){
                std::cout << "Hola" << std::endl;
                std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
                    return record.dataId;
                };
                ExtendibleHashFile<int, MovieRecord> extendible_hash_data_id{"movies_and_series.dat", "data_id", true, index};
                extendible_hash_data_id.create_index();
            }
        }
    }
}


