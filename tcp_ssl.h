#ifndef TCP_SSL_H
#define TCP_SSL_H

#include <QKeyEvent>
#include <QMainWindow>

namespace Ui
{
class tcp_ssl;
}

class tcp_ssl : public QMainWindow
{
    Q_OBJECT

public:
    explicit tcp_ssl(QWidget *parent = 0);
    ~tcp_ssl();

private slots:
    void on_lineEdit_eland_id_textChanged(const QString &arg1);

    void on_lineEdit_eland_model_textChanged(const QString &arg1);

    void on_comboBox_month_currentIndexChanged(int index);

    void on_comboBox_year_currentIndexChanged(int index);

    void on_lineEdit_serial_textChanged(const QString &arg1);

    void on_pushButton_post_clicked();

private:
    Ui::tcp_ssl *ui;

    void keyPressEvent(QKeyEvent *event);

    int eland_id;

    bool tcp_wait;
};

#endif  // TCP_SSL_H
