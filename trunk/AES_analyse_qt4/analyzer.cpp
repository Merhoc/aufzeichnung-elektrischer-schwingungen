#include "analyzer.h"
#include "ui_analyzer.h"

analyzer::analyzer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::analyzer)
{
    ui->setupUi(this);
}

analyzer::~analyzer()
{
    delete ui;
}
