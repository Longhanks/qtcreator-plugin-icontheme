#import <objc/runtime.h>

#import <Cocoa/Cocoa.h>

#include <QFile>

static IMP originalIconForFile;
static QByteArray folderIconData;

static id customThemeIconForFile(id objc_self, SEL objc_cmd, id fullPath) {
    BOOL isDir = NO;
    [[NSFileManager defaultManager]
        fileExistsAtPath:reinterpret_cast<NSString *>(fullPath)
             isDirectory:&isDir];
    if (isDir) {
        return [[NSImage alloc] initWithData:folderIconData.toRawNSData()];
    }
    return reinterpret_cast<NSImage *>(
        originalIconForFile(objc_self, objc_cmd, fullPath));
}

static void setupObjC() {
    Class classNSWorkspace = objc_getClass("NSWorkspace");
    Method methodIconForFile = class_getInstanceMethod(
        classNSWorkspace, sel_registerName("iconForFile:"));
    originalIconForFile = method_getImplementation(methodIconForFile);
    method_setImplementation(methodIconForFile,
                             reinterpret_cast<IMP>(customThemeIconForFile));
    auto folderIconFile =
        QFile(":/icons/qtcreator-nomo/scalable/places/folder.pdf");
    folderIconFile.open(QIODevice::ReadOnly);
    folderIconData = folderIconFile.readAll();
}

#define MUST_SETUP_OBJC
#include "plugin.cpp"
