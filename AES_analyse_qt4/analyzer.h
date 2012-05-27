#ifndef ANALYZER_H
#define ANALYZER_H

#include <QMainWindow>

namespace Ui {
class analyzer;
}

class analyzer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit analyzer(QWidget *parent = 0);
    ~analyzer();
    
private:
    Ui::analyzer *ui;
};

#endif // ANALYZER_H
