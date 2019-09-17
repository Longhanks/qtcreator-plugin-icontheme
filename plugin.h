#pragma once

#include <extensionsystem/iplugin.h>

#include <QStringList>

namespace IconTheme::Internal {

class IconThemePlugin final : public ExtensionSystem::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE
                          "icontheme.json")

public:
    explicit IconThemePlugin() noexcept;
    bool initialize(const QStringList &arguments,
                    QString *errorString) override;
    void extensionsInitialized() override {}
    ShutdownFlag aboutToShutdown() override;
};

} // namespace IconTheme::Internal
