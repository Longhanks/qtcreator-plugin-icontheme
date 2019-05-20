#pragma once

#include <extensionsystem/iplugin.h>

#include <QStringList>

namespace IconTheme::Internal {

class IconThemePlugin : public ExtensionSystem::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE
                          "icontheme.json")

public:
    IconThemePlugin();
    bool initialize(const QStringList &arguments,
                    QString *errorString) override;
    void extensionsInitialized() override {}
};

} // namespace IconTheme::Internal
