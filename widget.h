
#ifndef WIDGET_H
#define WIDGET_H

#include <QFuture>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "MovieRecord.h"
#include "parser.hpp"
#include "utils.hpp"

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
    void onQueryFinished();
    void onQueryStarted();

private:
    QFutureWatcher<void> futureWatcher{};
    Ui::Widget *ui;
    QVBoxLayout* global;
    QHBoxLayout* H1;
    QHBoxLayout *H2;
    QTableWidget* tabla;
    QLineEdit* consulta;
    QLabel * result;
    QLabel *tiempoResult;
    QPushButton *boton;
    parserSQL parsero;

    void displayRecords(std::vector<MovieRecord> &);
    void execute_action();
    template<typename T>
    void update_time(TimedResult<T>&);
};

#endif // WIDGET_H
