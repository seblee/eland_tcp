#include "tcp_ssl.h"
#include <QJsonObject>
#include "ui_tcp_ssl.h"

tcp_ssl::tcp_ssl(QWidget *parent) : QMainWindow(parent), ui(new Ui::tcp_ssl) { ui->setupUi(this); }

tcp_ssl::~tcp_ssl() { delete ui; }

void tcp_ssl::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
}

void tcp_ssl::on_lineEdit_eland_id_textChanged(const QString &arg1)
{
    QString eland_id_string;
    bool ok;
    eland_id = arg1.toInt(&ok, 10);
    eland_id_string = QString("%1").arg(eland_id, 6, 10, QLatin1Char('0'));
    ui->label_9->setText(QString::number(eland_id, 10));
    ui->lineEdit_serial->setText(eland_id_string);
}

void tcp_ssl::on_lineEdit_eland_model_textChanged(const QString &arg1)
{
    QString model_str = arg1;
    QString year_str = QString::number(((ui->comboBox_year->currentIndex() + 8) % 10), 10);
    QString month_str = QChar(ui->comboBox_month->currentIndex() + 'A');
    QString serial_str = ui->lineEdit_serial->text();
    ui->label_7->setText(model_str + month_str + year_str + serial_str);
}

void tcp_ssl::on_comboBox_month_currentIndexChanged(int index)
{
    QString model_str = ui->lineEdit_eland_model->text();
    QString year_str = QString::number(((ui->comboBox_year->currentIndex() + 8) % 10), 10);
    QString month_str = QChar(index + 'A');
    QString serial_str = ui->lineEdit_serial->text();
    ui->label_7->setText(model_str + month_str + year_str + serial_str);
}

void tcp_ssl::on_comboBox_year_currentIndexChanged(int index)
{
    QString model_str = ui->lineEdit_eland_model->text();
    QString year_str = QString::number(((index + 8) % 10), 10);
    QString month_str = QChar(ui->comboBox_month->currentIndex() + 'A');
    QString serial_str = ui->lineEdit_serial->text();
    ui->label_7->setText(model_str + month_str + year_str + serial_str);
}

void tcp_ssl::on_lineEdit_serial_textChanged(const QString &arg1)
{
    QString model_str = ui->lineEdit_eland_model->text();
    QString year_str = QString::number(((ui->comboBox_year->currentIndex() + 8) % 10), 10);
    QString month_str = QChar(ui->comboBox_month->currentIndex() + 'A');
    QString serial_str = arg1;
    ui->label_7->setText(model_str + month_str + year_str + serial_str);
}

void tcp_ssl::on_pushButton_post_clicked() {}
