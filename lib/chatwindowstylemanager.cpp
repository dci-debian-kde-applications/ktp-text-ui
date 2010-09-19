/*
   kopetechatwindowstylemanager.cpp - Manager all chat window styles

   Copyright (c) 2005      by Michaël Larouche     <larouche@kde.org>

   Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This program is free software; you can redistribute it and/or modify  *
   * it under the terms of the GNU General Public License as published by  *
   * the Free Software Foundation; either version 2 of the License, or     *
   * (at your option) any later version.                                   *
   *                                                                       *
   *************************************************************************
*/

#include "chatwindowstylemanager.h"
#include "chatstyleplistfilereader.h"

// Qt includes
#include <QStack>
#include <QFileInfo>

// KDE includes
#include <kstandarddirs.h>
#include <kdirlister.h>
#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <karchive.h>
#include <kzip.h>
#include <ktar.h>
#include <kmimetype.h>
#include <kio/netaccess.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "chatwindowstyle.h"

class ChatWindowStyleManager::Private
{
public:
    Private()
            : styleDirLister(0) {}

    ~Private() {
        if (styleDirLister) {
            styleDirLister->deleteLater();
        }

        qDeleteAll(stylePool);
    }

    KDirLister *styleDirLister;
    QMap <QString, QString > availableStyles;

    // key = style id, value = ChatWindowStyle instance
    QHash<QString, ChatWindowStyle*> stylePool;

    QStack<KUrl> styleDirs;
};

ChatWindowStyleManager *ChatWindowStyleManager::self()
{
    static ChatWindowStyleManager s;
    return &s;
}

ChatWindowStyleManager::ChatWindowStyleManager(QObject *parent)
        : QObject(parent), d(new Private())
{
    kDebug(14000) ;
    loadStyles();
}

ChatWindowStyleManager::~ChatWindowStyleManager()
{
    kDebug(14000) ;
    delete d;
}

void ChatWindowStyleManager::loadStyles()
{
    // Make sure there exists a directory where chat styles can be installed to and it will be watched for changes
    KStandardDirs::locateLocal("data", QLatin1String("ktelepathy/styles/"));

    QStringList chatStyles = KGlobal::dirs()->findDirs("data", QLatin1String("ktelepathy/styles"));

    foreach(const QString &styleDir, chatStyles) {
        kDebug(14000) << styleDir;
        d->styleDirs.push(KUrl(styleDir));
    }

    d->styleDirLister = new KDirLister(this);
    d->styleDirLister->setDirOnlyMode(true);

    connect(d->styleDirLister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(slotNewStyles(const KFileItemList &)));
    connect(d->styleDirLister, SIGNAL(completed()), this, SLOT(slotDirectoryFinished()));

    if (!d->styleDirs.isEmpty())
        d->styleDirLister->openUrl(d->styleDirs.pop(), KDirLister::Keep);
}

QMap<QString, QString> ChatWindowStyleManager::getAvailableStyles() const
{
    return d->availableStyles;
}

int ChatWindowStyleManager::installStyle(const QString &styleBundlePath)
{
//    QString localStyleDir;
//    QStringList chatStyles = KGlobal::dirs()->findDirs("appdata", QLatin1String("styles"));
//    // findDirs returns preferred paths first, let's check if one of them is writable
//    foreach(const QString& styleDir, chatStyles) {
//        if (QFileInfo(styleDir).isWritable()) {
//            localStyleDir = styleDir;
//            break;
//        }
//    }
//    if (localStyleDir.isEmpty()) { // none of dirs is writable
//        return StyleNoDirectoryValid;
//    }

//    KArchiveEntry *currentEntry = 0L;
//    KArchiveDirectory* currentDir = 0L;
//    KArchive *archive = 0L;

//    // Find mimetype for current bundle. ZIP and KTar need separate constructor
//    QString currentBundleMimeType = KMimeType::findByPath(styleBundlePath, 0, false)->name();
//    if (currentBundleMimeType == "application/zip") {
//        archive = new KZip(styleBundlePath);
//    } else if (currentBundleMimeType == "application/x-compressed-tar" || currentBundleMimeType == "application/x-bzip-compressed-tar" || currentBundleMimeType == "application/x-gzip" || currentBundleMimeType == "application/x-bzip") {
//        archive = new KTar(styleBundlePath);
//    } else {
//        return StyleCannotOpen;
//    }

//    if (!archive->open(QIODevice::ReadOnly)) {
//        delete archive;

//        return StyleCannotOpen;
//    }

//    const KArchiveDirectory* rootDir = archive->directory();

//    // Ok where we go to check if the archive is valid.
//    // Each time we found a correspondance to a theme bundle, we add a point to validResult.
//    // A valid style bundle must have:
//    // -a Contents, Contents/Resources, Co/Res/Incoming, Co/Res/Outgoing dirs
//    // main.css, Footer.html, Header.html, Status.html files in Contents/Resources.
//    // So for a style bundle to be valid, it must have a result greather than 8, because we test for 8 required entry.
//    int validResult = 0;
//    const QStringList entries = rootDir->entries();
//    // Will be reused later.
//    QStringList::ConstIterator entriesIt, entriesItEnd = entries.end();
//    for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
//        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
////      kDebug() << "Current entry name: " << currentEntry->name();
//        if (currentEntry->isDirectory()) {
//            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
//            if (currentDir) {
//                if (currentDir->entry(QString::fromUtf8("Contents"))) {
////                  kDebug() << "Contents found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources"))) {
////                  kDebug() << "Contents/Resources found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/Incoming"))) {
////                  kDebug() << "Contents/Resources/Incoming found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/Outgoing"))) {
////                  kDebug() << "Contents/Resources/Outgoing found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/main.css"))) {
////                  kDebug() << "Contents/Resources/main.css found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/Footer.html"))) {
////                  kDebug() << "Contents/Resources/Footer.html found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/Status.html"))) {
////                  kDebug() << "Contents/Resources/Status.html found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/Header.html"))) {
////                  kDebug() << "Contents/Resources/Header.html found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/Incoming/Content.html"))) {
////                  kDebug() << "Contents/Resources/Incoming/Content.html found";
//                    validResult += 1;
//                }
//                if (currentDir->entry(QString::fromUtf8("Contents/Resources/Outgoing/Content.html"))) {
////                  kDebug() << "Contents/Resources/Outgoing/Content.html found";
//                    validResult += 1;
//                }
//            }
//        }
//    }
////  kDebug() << "Valid result: " << QString::number(validResult);
//    // The archive is a valid style bundle.
//    if (validResult >= 8) {
//        bool installOk = false;
//        for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
//            currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
//            if (currentEntry && currentEntry->isDirectory()) {
//                // Ignore this MacOS X "garbage" directory in zip.
//                if (currentEntry->name() == QString::fromUtf8("__MACOSX")) {
//                    continue;
//                } else {
//                    currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
//                    if (currentDir) {
//                        currentDir->copyTo(localStyleDir + currentDir->name());
//                        installOk = true;
//                    }
//                }
//            }
//        }

//        archive->close();
//        delete archive;

//        if (installOk)
//            return StyleInstallOk;
//        else
//            return StyleUnknow;
//    } else {
//        archive->close();
//        delete archive;

//        return StyleNotValid;
//    }

//    if (archive) {
//        archive->close();
//        delete archive;
//    }

//    return StyleUnknow;
}

bool ChatWindowStyleManager::removeStyle(const QString &styleId)
{
//    kDebug(14000) << styleId;
//    // Find for the current style in avaiableStyles map.
//    int foundStyleIdx = d->availableStyles.indexOf(styleId);

//    if (foundStyleIdx != -1) {
//        d->availableStyles.removeAt(foundStyleIdx);

//        // Remove and delete style from pool if needed.
//        if (d->stylePool.contains(styleId)) {
//            ChatWindowStyle *deletedStyle = d->stylePool[styleId];
//            d->stylePool.remove(styleId);
//            delete deletedStyle;
//        }

//        QStringList styleDirs = KGlobal::dirs()->findDirs("appdata", QString("styles/%1").arg(styleId));
//        if (styleDirs.isEmpty()) {
//            kDebug(14000) << "Failed to find style" << styleId;
//            return false;
//        }

//        // attempt to delete all dirs with this style
//        int numDeleted = 0;
//        foreach(const QString& stylePath, styleDirs) {
//            KUrl urlStyle(stylePath);
//            // Do the actual deletion of the directory style.
//            if (KIO::NetAccess::del(urlStyle, 0))
//                numDeleted++;
//        }
//        return numDeleted == styleDirs.count();
//    } else {
//        return false;
//    }
}

ChatWindowStyle *ChatWindowStyleManager::getValidStyleFromPool(const QString &styleId)
{
    ChatWindowStyle *style = 0;
    style = getStyleFromPool(styleId);
    if (style)
        return style;

    kDebug(14000) << "Trying default style";
    // Try default style
    style = getStyleFromPool("Kopete");
    if (style)
        return style;

    kDebug(14000) << "Trying first valid style";
    // Try first valid style
    foreach(const QString& name, d->availableStyles) {
        style = getStyleFromPool(name);
        if (style)
            return style;
    }

    kDebug(14000) << "Valid style not found!";
    return 0;
}

ChatWindowStyle *ChatWindowStyleManager::getStyleFromPool(const QString &styleId)
{
    if (d->stylePool.contains(styleId)) {
        kDebug(14000) << styleId << " was on the pool";

        // NOTE: This is a hidden config switch for style developers
        // Check in the config if the cache is disabled.
        // if the cache is disabled, reload the style every time it's getted.
        KConfigGroup config(KGlobal::config(), "KopeteStyleDebug");
        bool disableCache = config.readEntry("disableStyleCache", false);
        if (disableCache) {
            d->stylePool[styleId]->reload();
        }

        return d->stylePool[styleId];
    }

    // Build a chat window style and list its variants, then add it to the pool.
    ChatWindowStyle *style = new ChatWindowStyle(styleId, ChatWindowStyle::StyleBuildNormal);
    if (!style->isValid()) {
        kDebug(14000) << styleId << " is invalid style!";
        delete style;
        return 0;
    }

    d->stylePool.insert(styleId, style);
    kDebug(14000) << styleId << " is just created";

    return style;
}

void ChatWindowStyleManager::slotNewStyles(const KFileItemList &dirList)
{
    foreach(const KFileItem &item, dirList) {
        // Ignore data dir(from deprecated XSLT themes)
        if (!item.url().fileName().contains(QString::fromUtf8("data"))) {
            kDebug(14000) << "Listing: " << item.url().fileName();
            // If the style path is already in the pool, that's mean the style was updated on disk
            // Reload the style
            QString styleId = item.url().fileName();
            if (d->stylePool.contains(styleId)) {
                kDebug(14000) << "Updating style: " << styleId;

                d->stylePool[styleId]->reload();

                // Add to available if required.
                if (! d->availableStyles.contains(styleId)) {

                    //FIXME this code is in two places.. this sucks!!!
                    ChatStylePlistFileReader plistReader(item.url().path().append("/Contents/Info.plist"));
                    QString styleName = plistReader.CFBundleName();
                    if (plistReader.CFBundleName().isEmpty()) {
                        styleName = styleId;
                    }
                    d->availableStyles.insert(styleId, styleName);
                }
            } else {
                ChatStylePlistFileReader plistReader(item.url().path().append("/Contents/Info.plist"));
                QString styleName = plistReader.CFBundleName();
                if (plistReader.CFBundleName().isEmpty()) {
                    styleName = styleId;
                }
                d->availableStyles.insert(styleId, styleName);
            }
        }
    }
}

void ChatWindowStyleManager::slotDirectoryFinished()
{
    // Start another scanning if the directories stack is not empty
    if (!d->styleDirs.isEmpty()) {
        kDebug(14000) << "Starting another directory.";
        d->styleDirLister->openUrl(d->styleDirs.pop(), KDirLister::Keep);
    } else {
        emit loadStylesFinished();
    }
}

#include "chatwindowstylemanager.moc"
