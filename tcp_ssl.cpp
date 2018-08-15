#include "tcp_ssl.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSslConfiguration>
#include "ui_tcp_ssl.h"

#define DEFAULT_TIMEZONE 32400
#define DEFAULT_BRIGHTNESS_NORMAL 80
#define DEFAULT_BRIGHTNESS_NIGHT 20
#define DEFAULT_LED_NORMAL 80
#define DEFAULT_LED_NIGHT 20
#define DEFAULT_VOLUME_NORMAL 50
#define DEFAULT_VOLUME_NIGHT 50
#define DEFAULT_NIGHT_MODE_ENABLE 0
#define DEFAULT_AREA_CODE 43
#define DEFAULT_NIGHT_BEGIN ("22:00:00")
#define DEFAULT_NIGHT_END ("06:00:00")

tcp_ssl::tcp_ssl(QWidget *parent) : QMainWindow(parent), ui(new Ui::tcp_ssl)
{
    eland_id = 1;
    tcp_wait = false;
    ui->setupUi(this);
}

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

void tcp_ssl::on_pushButton_post_clicked()
{
    if (tcp_wait) return;
    QJsonObject json;
    json.insert("eland_id", eland_id);
    json.insert("serial_number", ui->label_7->text());
    json.insert("eland_name", "eland");
    json.insert("timezone_offset_sec", 32400);

    json.insert("user_id", "");
    json.insert("dhcp_enabled", 1);
    json.insert("ip_address", "");
    json.insert("subnet_mask", "");
    json.insert("default_gateway", "");
    json.insert("primary_dns", "");
    json.insert("area_code", 43);

    QJsonDocument document;
    document.setObject(json);

    QByteArray bytearray = document.toJson(QJsonDocument::Compact);

    m_manager = new QNetworkAccessManager(this);

    QNetworkRequest request;
    request.setRawHeader("Content-Type", "application/json; charset=utf-8");
    request.setRawHeader("host", "192.168.0.1");
    request.setUrl(QUrl("http://192.168.0.1/setting"));

    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(reply_finished(QNetworkReply *)));
    connect(&tm, SIGNAL(timeout()), this, SLOT(handleTimeOut()));

    m_reply = m_manager->post(request, bytearray);
    tcp_wait = true;
    if (tm.isActive()) tm.stop();
    tm.start(2000);
    ui->statusBar->showMessage("posting eland data", 2000);

    on_pushButton_clicked();
}

void tcp_ssl::on_pushButton_get_clicked()
{
    if (tcp_wait) return;
    m_manager = new QNetworkAccessManager(this);
    QNetworkRequest request;

    request.setRawHeader("host", "192.168.0.1");
    request.setUrl(QUrl("http://192.168.0.1/setting"));

    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(reply_finished(QNetworkReply *)));
    connect(&tm, SIGNAL(timeout()), this, SLOT(handleTimeOut()));

    m_reply = m_manager->get(request);
    tcp_wait = true;
    if (tm.isActive()) tm.stop();
    tm.start(2000);
    ui->statusBar->showMessage("getting eland data", 2000);

    on_pushButton_clicked();
}

void tcp_ssl::tcp_ssl::handleTimeOut()
{
    ui->textEdit->setText(tr("time out"));
    QDateTime current_time = QDateTime::currentDateTime();
    QString current_time_str = current_time.toString("yyyy-MM-dd hh:mm:ss.zzz ddd t");
    ui->textEdit->append(current_time_str);
    m_reply->abort();
    m_reply->deleteLater();
}

void tcp_ssl::reply_finished(QNetworkReply *reply)
{
    bool ok;
    if (tm.isActive()) tm.stop();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QTextCodec *codec = QTextCodec::codecForName("utf-8");

        QString str = codec->toUnicode(data);

        ui->textEdit->setText(str);

        QJsonParseError jsonError;

        QJsonDocument json = QJsonDocument::fromJson(data, &jsonError);

        if (jsonError.error == QJsonParseError::NoError) {
            qDebug() << "QJsonParse OK";
            if (json.isObject()) {
                qDebug() << "json.isObject()";
                QJsonObject object = json.object();
                if (object.contains("device")) {
                    qDebug() << "object contains device";
                    QJsonValue value = object.value("device");
                    if (value.isObject()) {
                        qDebug() << "value is object";
                        QJsonObject obj = value.toObject();
                        /*********eland_id************/
                        if (obj.contains("eland_id")) {
                            QJsonValue id_value = obj.value("eland_id");
                            if (id_value.isDouble()) {
                                int get_id = id_value.toVariant().toInt();
                                qDebug() << "get_id:" << QString::number(get_id, 10);
                                if (get_id == ui->lineEdit_eland_id->text().toInt(&ok, 10))
                                    ui->checkBox_id->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_id->setCheckState(Qt::PartiallyChecked);
                            }
                        } else
                            qDebug() << "obt not contains eland_id";
                        /*********serial_number************/
                        if (obj.contains("serial_number")) {
                            QJsonValue serial_value = obj.value("serial_number");
                            if (serial_value.isString()) {
                                QString serial_get = serial_value.toString();
                                QString serial_current = ui->label_7->text();
                                qDebug() << serial_get;
                                if (serial_current.compare(serial_get) == 0)
                                    ui->checkBox_serial->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_serial->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********user_id************/
                        if (obj.contains("user_id")) {
                            QJsonValue user_id_value = obj.value("user_id");
                            if (user_id_value.isString()) {
                                QString user_id_get = user_id_value.toString();
                                qDebug() << user_id_get;
                                if (user_id_get.length() == 0)
                                    ui->checkBox_user_id->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_user_id->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********timezone_offset_sec************/
                        if (obj.contains("timezone_offset_sec")) {
                            QJsonValue zone_value = obj.value("timezone_offset_sec");
                            if (zone_value.isDouble()) {
                                int zone_id = zone_value.toVariant().toInt();
                                qDebug() << "zone_id:" << QString::number(zone_id, 10);
                                if (zone_id == DEFAULT_TIMEZONE)
                                    ui->checkBox_timezone->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_timezone->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********firmware_version************/
                        if (obj.contains("firmware_version")) {
                            QJsonValue firmware_value = obj.value("firmware_version");
                            if (firmware_value.isString()) {
                                QString firmware_get = firmware_value.toString();
                                QString firmware_current = ui->lineEdit_firmware->text();
                                qDebug() << firmware_get;
                                if (firmware_current.compare(firmware_get) == 0)
                                    ui->checkBox_firmware->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_firmware->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********dhcp_enabled************/
                        if (obj.contains("dhcp_enabled")) {
                            QJsonValue dhcp_value = obj.value("dhcp_enabled");
                            if (dhcp_value.isDouble()) {
                                int dhcp = dhcp_value.toVariant().toInt();
                                qDebug() << "dhcp:" << QString::number(dhcp, 10);
                                if (dhcp == 1)
                                    ui->checkBox_dhcp->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_dhcp->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********time_display_format************/
                        if (obj.contains("time_display_format")) {
                            QJsonValue time_display_format_value = obj.value("time_display_format");
                            if (time_display_format_value.isDouble()) {
                                int time_display_format = time_display_format_value.toVariant().toInt();
                                qDebug() << "time_display_format:" << QString::number(time_display_format, 10);
                                if (time_display_format == 1)
                                    ui->checkBox_time_formate->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_time_formate->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********brightness_normal************/
                        if (obj.contains("brightness_normal")) {
                            QJsonValue brightness_normal_value = obj.value("brightness_normal");
                            if (brightness_normal_value.isDouble()) {
                                int brightness_normal = brightness_normal_value.toVariant().toInt();
                                qDebug() << "brightness_normal:" << QString::number(brightness_normal, 10);
                                if (brightness_normal == DEFAULT_BRIGHTNESS_NORMAL)
                                    ui->checkBox_brightness_normal->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_brightness_normal->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********brightness_night************/
                        if (obj.contains("brightness_night")) {
                            QJsonValue brightness_night_value = obj.value("brightness_night");
                            if (brightness_night_value.isDouble()) {
                                int brightness_night = brightness_night_value.toVariant().toInt();
                                qDebug() << "brightness_night:" << QString::number(brightness_night, 10);
                                if (brightness_night == DEFAULT_BRIGHTNESS_NIGHT)
                                    ui->checkBox_brightness_night->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_brightness_night->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********led_normal************/
                        if (obj.contains("led_normal")) {
                            QJsonValue led_normal_value = obj.value("led_normal");
                            if (led_normal_value.isDouble()) {
                                int led_normal = led_normal_value.toVariant().toInt();
                                qDebug() << "led_normal:" << QString::number(led_normal, 10);
                                if (led_normal == DEFAULT_LED_NORMAL)
                                    ui->checkBox_led_normal->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_led_normal->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********led_night************/
                        if (obj.contains("led_night")) {
                            QJsonValue led_night_value = obj.value("led_night");
                            if (led_night_value.isDouble()) {
                                int led_night = led_night_value.toVariant().toInt();
                                qDebug() << "led_night:" << QString::number(led_night, 10);
                                if (led_night == DEFAULT_LED_NIGHT)
                                    ui->checkBox_led_night->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_led_night->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********notification_volume_normal************/
                        if (obj.contains("notification_volume_normal")) {
                            QJsonValue volume_normal_value = obj.value("notification_volume_normal");
                            if (volume_normal_value.isDouble()) {
                                int volume_normal = volume_normal_value.toVariant().toInt();
                                qDebug() << "volume_normal:" << QString::number(volume_normal, 10);
                                if (volume_normal == DEFAULT_VOLUME_NORMAL)
                                    ui->checkBox_volume_normal->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_volume_normal->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********notification_volume_night************/
                        if (obj.contains("notification_volume_night")) {
                            QJsonValue volume_night_value = obj.value("notification_volume_night");
                            if (volume_night_value.isDouble()) {
                                int volume_night = volume_night_value.toVariant().toInt();
                                qDebug() << "volume_night:" << QString::number(volume_night, 10);
                                if (volume_night == DEFAULT_VOLUME_NIGHT)
                                    ui->checkBox_volume_night->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_volume_night->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********night_mode_enabled************/
                        if (obj.contains("night_mode_enabled")) {
                            QJsonValue night_mode_enabled_value = obj.value("night_mode_enabled");
                            if (night_mode_enabled_value.isDouble()) {
                                int night_mode_enabled = night_mode_enabled_value.toVariant().toInt();
                                qDebug() << "night_mode_enabled:" << QString::number(night_mode_enabled, 10);
                                if (night_mode_enabled == DEFAULT_NIGHT_MODE_ENABLE)
                                    ui->checkBox_night_enable->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_night_enable->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********led_night************/
                        if (obj.contains("night_mode_enabled")) {
                            QJsonValue night_mode_enabled_value = obj.value("night_mode_enabled");
                            if (night_mode_enabled_value.isDouble()) {
                                int night_mode_enabled = night_mode_enabled_value.toVariant().toInt();
                                qDebug() << "night_mode_enabled:" << QString::number(night_mode_enabled, 10);
                                if (night_mode_enabled == DEFAULT_NIGHT_MODE_ENABLE)
                                    ui->checkBox_night_enable->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_night_enable->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********night_mode_begin_time************/
                        if (obj.contains("night_mode_begin_time")) {
                            QJsonValue begin_time_value = obj.value("night_mode_begin_time");
                            if (begin_time_value.isString()) {
                                QString begin_time_get = begin_time_value.toString();
                                qDebug() << begin_time_get;
                                if (begin_time_get.compare(DEFAULT_NIGHT_BEGIN) == 0)
                                    ui->checkBox_night_start->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_night_start->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********night_mode_end_time************/
                        if (obj.contains("night_mode_end_time")) {
                            QJsonValue end_time_value = obj.value("night_mode_end_time");
                            if (end_time_value.isString()) {
                                QString end_time_get = end_time_value.toString();
                                qDebug() << end_time_get;
                                if (end_time_get.compare(DEFAULT_NIGHT_END) == 0)
                                    ui->checkBox_night_end->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_night_end->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                        /*********area_code************/
                        if (obj.contains("area_code")) {
                            QJsonValue area_code_value = obj.value("area_code");
                            if (area_code_value.isDouble()) {
                                int area_code = area_code_value.toVariant().toInt();
                                qDebug() << "area_code:" << QString::number(area_code, 10);
                                if (area_code == DEFAULT_AREA_CODE)
                                    ui->checkBox_area_code->setCheckState(Qt::Checked);
                                else
                                    ui->checkBox_area_code->setCheckState(Qt::PartiallyChecked);
                            }
                        }
                    } else
                        qDebug() << "value is not object";
                } else
                    qDebug() << "object donot contains device";
            } else
                qDebug() << "json.is not Object()";
        } else
            qDebug() << "QJsonParse err ";
        ui->statusBar->showMessage("status ok", 2000);
    } else {
        ui->statusBar->showMessage("status err", 2000);
        ui->textEdit->append(tr("error = ") + QString::number(reply->error(), 10));
    }

    QDateTime current_time = QDateTime::currentDateTime();
    QString current_time_str = current_time.toString("yyyy-MM-dd hh:mm:ss.zzz ddd t");
    ui->textEdit->append(current_time_str);

    reply->deleteLater();
    tcp_wait = false;
}

void tcp_ssl::reply_ssl(QNetworkReply *reply)
{
    if (tm.isActive()) tm.stop();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QTextCodec *codec = QTextCodec::codecForName("utf-8");

        QString str = codec->toUnicode(data);

        ui->textEdit->setText(str);

        ui->statusBar->showMessage("status ok", 2000);
    } else {
        ui->statusBar->showMessage("status err", 2000);
        ui->textEdit->append(tr("error = ") + QString::number(reply->error(), 10));
    }

    QDateTime current_time = QDateTime::currentDateTime();
    QString current_time_str = current_time.toString("yyyy-MM-dd hh:mm:ss.zzz ddd t");
    ui->textEdit->append(current_time_str);

    reply->deleteLater();
    tcp_wait = false;
}

void tcp_ssl::on_pushButton_clicked()
{
    ui->checkBox_id->setCheckState(Qt::Unchecked);
    ui->checkBox_serial->setCheckState(Qt::Unchecked);
    ui->checkBox_user_id->setCheckState(Qt::Unchecked);
    ui->checkBox_timezone->setCheckState(Qt::Unchecked);
    ui->checkBox_firmware->setCheckState(Qt::Unchecked);
    ui->checkBox_dhcp->setCheckState(Qt::Unchecked);
    ui->checkBox_time_formate->setCheckState(Qt::Unchecked);
    ui->checkBox_brightness_normal->setCheckState(Qt::Unchecked);
    ui->checkBox_brightness_night->setCheckState(Qt::Unchecked);
    ui->checkBox_led_normal->setCheckState(Qt::Unchecked);
    ui->checkBox_led_night->setCheckState(Qt::Unchecked);
    ui->checkBox_volume_normal->setCheckState(Qt::Unchecked);
    ui->checkBox_volume_night->setCheckState(Qt::Unchecked);
    ui->checkBox_night_enable->setCheckState(Qt::Unchecked);
    ui->checkBox_night_start->setCheckState(Qt::Unchecked);
    ui->checkBox_night_end->setCheckState(Qt::Unchecked);
    ui->checkBox_serial->setCheckState(Qt::Unchecked);
    ui->checkBox_serial->setCheckState(Qt::Unchecked);
    ui->checkBox_serial->setCheckState(Qt::Unchecked);
    ui->checkBox_serial->setCheckState(Qt::Unchecked);
    ui->checkBox_serial->setCheckState(Qt::Unchecked);
    ui->checkBox_serial->setCheckState(Qt::Unchecked);
    ui->checkBox_area_code->setCheckState(Qt::Unchecked);
}

void tcp_ssl::on_pushButton_ssl_clicked()
{
    if (tcp_wait) return;
    QNetworkRequest request;
    m_manager = new QNetworkAccessManager(this);

    QSslConfiguration sslconfig;
    sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslconfig.setProtocol(QSsl::TlsV1_2);

    request.setRawHeader("host", "www.taobao.com");
    request.setUrl(QUrl("https://www.taobao.com"));
    request.setSslConfiguration(sslconfig);

    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(reply_ssl(QNetworkReply *)));
    connect(&tm, SIGNAL(timeout()), this, SLOT(handleTimeOut()));

    m_reply = m_manager->get(request);
    tcp_wait = true;
    if (tm.isActive()) tm.stop();
    tm.start(2000);
    ui->statusBar->showMessage("getting https://www.taobao.com", 2000);
}
