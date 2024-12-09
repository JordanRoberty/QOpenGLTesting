#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <QString>
#include <QMap>
#include <QSet>
#include <QStringList>

class ShaderManager {
public:
    explicit ShaderManager(const QString& baseDir, const QString& version = "130");
    bool loadModule(const QString& filePath);
    QString buildShader(const QString& mainFile);

private:
    struct ShaderModule {
        QString content;
        QStringList dependencies;
        QString filePath;
    };

    QString processIncludes(const QString& content, const QString& currentFile);
    QString getFileContent(const QString& filePath);
    
    QMap<QString, ShaderModule> m_modules;
    QString m_shaderVersion;
    QString m_baseDir;
    QSet<QString> m_processingStack;
};

#endif // SHADERMANAGER_H