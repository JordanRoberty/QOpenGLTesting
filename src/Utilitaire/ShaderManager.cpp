// ShaderManager.cpp
#include "ShaderManager.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QFileInfo>

ShaderManager::ShaderManager(const QString& baseDir, const QString& version)
    : m_baseDir(baseDir)
    , m_shaderVersion(version) 
{
}

bool ShaderManager::loadModule(const QString& filePath) 
{
    QString fullPath = m_baseDir + "/" + filePath;
    QFile file(fullPath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Impossible d'ouvrir le fichier shader:" << fullPath;
        return false;
    }

    QString content = file.readAll();
    
    // Analyse des dépendances (#include)
    QStringList dependencies;
    QRegularExpression includeRegex("#include\\s+\"([^\"]+)\"");
    auto matches = includeRegex.globalMatch(content);
    
    while (matches.hasNext()) {
        auto match = matches.next();
        QString dependency = match.captured(1);
        dependencies << dependency;
        
        // Charger récursivement les dépendances
        loadModule(dependency);
    }

    // Stocker le module
    QFileInfo fileInfo(filePath);
    m_modules[fileInfo.fileName()] = {content, dependencies, filePath};
    return true;
}

QString ShaderManager::buildShader(const QString& mainFile) 
{
    m_processingStack.clear();
    QString version = QString("#version %1\n\n").arg(m_shaderVersion);
    
    if (!m_modules.contains(mainFile)) {
        qWarning() << "Fichier principal non trouvé:" << mainFile;
        return QString();
    }

    return version + processIncludes(m_modules[mainFile].content, mainFile);
}

QString ShaderManager::processIncludes(const QString& content, const QString& currentFile) 
{
    if (m_processingStack.contains(currentFile)) {
        qWarning() << "Inclusion cyclique détectée pour le fichier:" << currentFile;
        return QString();
    }

    m_processingStack.insert(currentFile);
    
    QString processedContent = content;
    QRegularExpression includeRegex("#include\\s+\"([^\"]+)\"");
    auto matches = includeRegex.globalMatch(content);
    
    while (matches.hasNext()) {
        auto match = matches.next();
        QString includePath = match.captured(1);
        QString includeContent;
        
        if (m_modules.contains(includePath)) {
            includeContent = processIncludes(m_modules[includePath].content, includePath);
        } else {
            qWarning() << "Fichier include non trouvé:" << includePath;
            continue;
        }
        
        // Remplacer la ligne #include par le contenu
        processedContent.replace(match.captured(0), includeContent);
    }

    m_processingStack.remove(currentFile);
    return processedContent + "\n";
}

QString ShaderManager::getFileContent(const QString& filePath)
{
    QFile file(m_baseDir + "/" + filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return file.readAll();
}