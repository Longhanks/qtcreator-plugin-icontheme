#include "plugin.h"

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QSettings>

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
        // TODO: Implement GNOME/Gtk
        return QIcon::themeName();
    }();

    QIcon::setThemeName("qtcreator-nomo");
    if (!fallbackThemeName.isEmpty()) {
        QIcon::setFallbackThemeName(fallbackThemeName);
    }

#ifdef MUST_SETUP_OBJC
    setupObjC();
#endif
}

bool IconThemePlugin::initialize([[maybe_unused]] const QStringList &arguments,
                                 [[maybe_unused]] QString *errorString) {

    return true;
}

ExtensionSystem::IPlugin::ShutdownFlag IconThemePlugin::aboutToShutdown() {
#ifdef MUST_SETUP_OBJC
    releaseObjcC();
#endif
    return SynchronousShutdown;
}

} // namespace IconTheme::Internal
