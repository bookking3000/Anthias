#pragma once

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWidget>
#include <QEventLoop>
#include <QMap>

class View : public QWebEngineView
{
    Q_OBJECT

public:
    explicit View(QWidget* parent);

    void loadPage(const QString &uri);
    void loadImage(const QString &uri);

    void cachePage(const QString &uri, const QString &htmlContent);
    void clearCache();

private slots:
    void handleAuthRequest(QNetworkReply*, QAuthenticator*);

private:
    QWebEnginePage* pre_loader;
    QEventLoop pre_loader_loop;

    static QMap<QString, QString> pageCache;
};
