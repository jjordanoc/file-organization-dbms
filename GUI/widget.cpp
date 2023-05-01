#include "widget.h"
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

}


