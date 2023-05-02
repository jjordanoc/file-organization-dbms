
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include "parser.h"
#include "MovieRecord.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget

{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
signals:
    void fullScreenChanged(bool isFullScreen);

private slots:
    void SetQuery();
private:
    Ui::Widget *ui;
    QVBoxLayout* global;
    QHBoxLayout* H1;
    QTableWidget* tabla;
    QLineEdit* consulta;
    QLabel * result;
    QPushButton *boton;
    parserSQL parsero;
    void displayRecords(std::vector<MovieRecord> &);
};

#endif // WIDGET_H
