#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QStandardPaths>
#include <QEventLoop>
#include <QTimer>
#include <QMap>

#include "view.h"


View::View(QWidget* parent) : QWebEngineView(parent)
{
    connect(
        QWebEngineView::page(),
        SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
        this,
        SLOT(handleAuthRequest(QNetworkReply*,QAuthenticator*))
    );
    pre_loader = new QWebEnginePage;
}

void View::loadPage(const QString &uri)
{
    qDebug() << "Type: Webpage";

    // Check if the page is in the cache
    if (pageCache.contains(uri)) {
        // Page is in the cache, load it from there
        setHtml(pageCache.value(uri));
    } else {
        // Page is not in the cache, load it from the internet
        stop();
        load(QUrl(uri));

        // Cache the page after it's loaded
        connect(this, &QWebEngineView::loadFinished, this, [this, uri](bool success) {
            if (success) {
                // Cache the HTML content of the loaded page
                cachePage(uri, page()->toHtml());
            }
        });
    }
}

void View::cachePage(const QString &uri, const QString &htmlContent)
{
    // Cache the HTML content for the given URI
    pageCache.insert(uri, htmlContent);
}

// Declare and initialize the pageCache
QMap<QString, QString> View::pageCache;

// You can also add a method to clear the cache if needed
void View::clearCache()
{
    pageCache.clear();
}

void View::loadImage(const QString &preUri)
{
    qDebug() << "Type: Image";
    QFileInfo fileInfo = QFileInfo(preUri);
    QString src;

    if (fileInfo.isFile())
    {
        qDebug() << "Location: Local File";

        QUrl url;
        url.setScheme("http");
        // url.setHost(qgetenv("LISTEN"));
        url.setHost("anthias-nginx");
        url.setPath("/screenly_assets/" + fileInfo.fileName());

        src = url.toString();
    }
    else if (preUri == "null")
    {
        qDebug() << "Black page";
    }
    else
    {
        qDebug() << "Location: Remote URL";
        src = preUri;
    }

    qDebug() << "Current src: " + src;

    QString script = "window.setimg=function(n){var o=new Image;o.onload=function()"
                     "{document.body.style.backgroundSize=o.width>window.innerWidth||o.height>window.innerHeight?\"contain\":\"auto\",document.body.style.backgroundImage=\"url('\"+n+\"')\"},o.src=n};";
    QString styles = "background: #000 center no-repeat";

    stop();
    pre_loader -> setHtml("<html><head><script>" + script + "</script></head><body style='" + styles + "'><script>window.setimg(\"" + src + "\");</script></body></html>");
    connect(pre_loader,SIGNAL(loadFinished(bool)),&pre_loader_loop,SLOT(quit()));
    QTimer::singleShot(5000, &pre_loader_loop, SLOT(quit()));
    pre_loader_loop.exec();
    pre_loader -> toHtml([&](const QString &result){ setHtml(result); });
}

void View::handleAuthRequest(QNetworkReply* reply, QAuthenticator* auth)
{
    Q_UNUSED(reply)
    Q_UNUSED(auth)
    load(QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::DataLocation, "res/access_denied.html")));
}
