/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <clocale>
#include "accessManager.h"
#include "dialogs.h"
#include "downloader.h"
#include "downloaderAbstractItem.h"
#include "mpvObject.h"
#include "playlistModel.h"
#include "platform/application.h"
#include "platform/detectOpengl.h"
#include "platform/paths.h"
#include "plugin.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    qputenv("PYTHONIOENCODING", QByteArrayLiteral("utf-8"));

    detectOpenGLEarly();
    
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    Application app(argc, argv);
    app.setOrganizationName(QStringLiteral("coslyk"));
    app.setOrganizationDomain(QStringLiteral("coslyk.github.io"));
    app.setApplicationName(QStringLiteral("MoonPlayer"));
    app.setApplicationVersion(QStringLiteral(MOONPLAYER_VERSION));
    
    detectOpenGLLate();
    
    if (!app.parseArgs())
        return 0;
    
    
    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");
    qputenv("LC_NUMERIC", QByteArrayLiteral("C"));
    
    // Translate
    QTranslator translator;
    if (translator.load(QLocale::system().name(), QStringLiteral(":/l10n")))
        app.installTranslator(&translator);

    qmlRegisterType<MpvObject>("MoonPlayer", 1, 0, "MpvObject");
    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/Color.qml")), "MoonPlayer", 1, 0, "Color");
    qmlRegisterUncreatableType<DownloaderAbstractItem>("MoonPlayer", 1, 0, "DownloaderItem", QStringLiteral("Access to enums & flags only"));
    qmlRegisterSingletonType<Dialogs>("MoonPlayer", 1, 0, "Dialogs", [](QQmlEngine*, QJSEngine*) -> QObject* { return new Dialogs(); });
    
    QQmlApplicationEngine engine;

    // Set UI style
    if (QSettings().value(QStringLiteral("player/use_system_frame")).toBool())
    {
        engine.addImportPath(QStringLiteral("qrc:/qml/classicUI"));
        qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("fusion"));
    }
    else
    {
        engine.addImportPath(QStringLiteral("qrc:/qml/modernUI"));
        qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", QByteArrayLiteral("Dense"));
        qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("material"));
    }

    QQmlContext* context = engine.rootContext();
    Downloader* downloader = Downloader::instance();

    Q_ASSERT(context != nullptr);
    Q_ASSERT(downloader != nullptr);
    
    context->setContextProperty(QStringLiteral("accessManager"), NetworkAccessManager::instance());
    context->setContextProperty(QStringLiteral("downloaderModel"), QVariant::fromValue(downloader->model()));
    context->setContextProperty(QStringLiteral("playlistModel"), PlaylistModel::instance());
    context->setContextProperty(QStringLiteral("plugins"), QVariant::fromValue(Plugin::loadPlugins()));
    context->setContextProperty(QStringLiteral("utils"), new Utils());
    
    // Update downloader model
    QObject::connect(downloader, &Downloader::modelUpdated, [=]() {
        context->setContextProperty(QStringLiteral("downloaderModel"), QVariant::fromValue(downloader->model()));
    });

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    
    // Check updates
    Utils::checkUpdate();
    
    // Create user resources dir
    if (!QDir(userResourcesPath()).exists())
        QDir().mkpath(userResourcesPath());
    
    return app.exec();
}
