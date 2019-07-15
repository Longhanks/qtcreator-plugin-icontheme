#include "plugin.h"

#ifdef __APPLE__
#include <objc/message.h>
#include <objc/runtime.h>
#endif

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QSettings>

#if defined(GTK3_FOUND) && !defined(__APPLE__) && !defined(_WIN32)
#ifdef signals
#undef signals
#endif
#include <gtk/gtk.h>
#include <qobjectdefs.h>
#endif

#ifdef __APPLE__
static IMP originalIconForFile;
static QByteArray *folderIconData;

static id customThemeIconForFile(id objc_self, SEL objc_cmd, id fullPath) {
    BOOL isDir = NO;
    Class NSFileManager = objc_getClass("NSFileManager");
    id defaultManager = objc_msgSend(reinterpret_cast<id>(NSFileManager),
                                     sel_registerName("defaultManager"));
    objc_msgSend(defaultManager,
                 sel_registerName("fileExistsAtPath:isDirectory:"),
                 fullPath,
                 reinterpret_cast<id>(&isDir));
    if (isDir) {
        Class NSImage = objc_getClass("NSImage");
        id imageMemory = objc_msgSend(reinterpret_cast<id>(NSImage),
                                      sel_registerName("alloc"));
        return objc_msgSend(imageMemory,
                            sel_registerName("initWithData:"),
                            folderIconData->toRawNSData());
    }
    return originalIconForFile(objc_self, objc_cmd, fullPath);
}
#endif

inline void init_resource() {
    Q_INIT_RESOURCE(icontheme);
}

namespace IconTheme::Internal {

static inline QByteArray getDesktopEnvironment() {
    const auto xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (!xdgCurrentDesktop.isEmpty()) {
        return xdgCurrentDesktop.toUpper();
    }
    if (!qEnvironmentVariableIsEmpty("KDE_FULL_SESSION")) {
        return QByteArrayLiteral("KDE");
    }
    if (!qEnvironmentVariableIsEmpty("GNOME_DESKTOP_SESSION_ID")) {
        return QByteArrayLiteral("GNOME");
    }
    auto desktopSession = qgetenv("DESKTOP_SESSION");
    int slash = desktopSession.lastIndexOf('/');
    if (slash != -1) {
        QSettings desktopFile(QFile::decodeName(desktopSession + ".desktop"),
                              QSettings::IniFormat);
        desktopFile.beginGroup(QStringLiteral("Desktop Entry"));
        auto desktopName =
            desktopFile.value(QStringLiteral("DesktopNames")).toByteArray();
        if (!desktopName.isEmpty()) {
            return desktopName;
        }
        desktopSession = desktopSession.mid(slash + 1);
    }
    if (desktopSession == "gnome") {
        return QByteArrayLiteral("GNOME");
    }
    if (desktopSession == "xfce") {
        return QByteArrayLiteral("XFCE");
    }
    if (desktopSession == "kde") {
        return QByteArrayLiteral("KDE");
    }
    return QByteArrayLiteral("UNKNOWN");
}

IconThemePlugin::IconThemePlugin() {
    init_resource();

    const QString fallbackThemeName = []() -> QString {
        if (getDesktopEnvironment() == QByteArrayLiteral("KDE")) {
            auto configDir = qEnvironmentVariable("XDG_CONFIG_DIRS");
            if (configDir.isEmpty()) {
                configDir = QDir::home().absolutePath() + "/.config";
            }

            auto settings =
                QSettings(configDir + "/kdeglobals", QSettings::IniFormat);
            auto val = settings.value("Icons/Theme");

            if (val.canConvert<QString>()) {
                return val.toString();
            }

            return QString();
        }
#if defined(GTK3_FOUND) && !defined(__APPLE__) && !defined(_WIN32)
        else {
            if (!gtk_init_check(nullptr, nullptr)) {
                return QIcon::themeName();
            }
            GtkSettings *settings = gtk_settings_get_default();
            gchararray iconThemeName;
            g_object_get(settings, "gtk-icon-theme-name", &iconThemeName, nullptr);
            auto qIconThemeName = QString::fromUtf8(iconThemeName);
            if (!qIconThemeName.isEmpty()) {
                return qIconThemeName;
            }
        }
#endif
        return QIcon::themeName();
    }();

    QIcon::setThemeName("qtcreator-nomo");
    if (!fallbackThemeName.isEmpty()) {
        QIcon::setFallbackThemeName(fallbackThemeName);
    }

#ifdef __APPLE__
    Class NSWorkspace = objc_getClass("NSWorkspace");
    Method methodIconForFile =
        class_getInstanceMethod(NSWorkspace, sel_registerName("iconForFile:"));
    originalIconForFile = method_getImplementation(methodIconForFile);
    method_setImplementation(methodIconForFile,
                             reinterpret_cast<IMP>(customThemeIconForFile));
    auto folderIconFile =
        QFile(":/icons/qtcreator-nomo/scalable/places/folder.pdf");
    folderIconFile.open(QIODevice::ReadOnly);
    folderIconData = new QByteArray(folderIconFile.readAll());
#endif
}

bool IconThemePlugin::initialize([[maybe_unused]] const QStringList &arguments,
                                 [[maybe_unused]] QString *errorString) {
    return true;
}

ExtensionSystem::IPlugin::ShutdownFlag IconThemePlugin::aboutToShutdown() {
#ifdef __APPLE__
    delete folderIconData;
#endif
    return SynchronousShutdown;
}

} // namespace IconTheme::Internal
