/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *    Copyright (C) 2013  Andrea Scarpino <andrea@archlinux.org>
 *
 *    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>
 *    Copyright (c) 2004-2005 by Olivier Goffart  <ogoffart@kde. org>
 *    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "latex-filter.h"
// KConfigSkeleton
#include "latexconfig.h"

#include <QImage>

#include <KPluginFactory>
#include <KDebug>
#include <KStandardDirs>
#include <KProcess>
#include <X11/Xproto.h>

LatexFilter::LatexFilter(QObject* parent, const QVariantList &)
    : AbstractMessageFilter(parent)
{
}

void LatexFilter::filterMessage(KTp::Message &message, const KTp::MessageContext &context)
{
    Q_UNUSED(context);

    const QString messageText = message.mainMessagePart();
    if (!messageText.contains(QLatin1String("$$"))) {
        return;
    }

    QRegExp rg(QLatin1String("\\$\\$.+\\$\\$"));
    rg.setMinimal(true);

    QMap<QString, QString> replaceMap;
    int pos = 0;
    while (pos >= 0 && pos < messageText.length()) {
        pos = rg.indexIn(messageText, pos);

        if (pos >= 0) {
            const QString match = rg.cap(0);
            pos += rg.matchedLength();

            QString formul = match;
            // first remove the $$ delimiters on start and end
            formul.remove(QLatin1String("$$"));
            // then trim the result, so we can skip totally empty/whitespace-only formulas
            formul = formul.trimmed();
            if (formul.isEmpty() || !securityCheck(formul)) {
                continue;
            }

            // get the image and encode it with base64
            replaceMap[match] = handleLatex(formul);
        }
    }

    if (replaceMap.isEmpty()) {
        // we haven't found any LaTeX strings
        return;
    }

    for (QMap<QString, QString>::ConstIterator it = replaceMap.constBegin(); it != replaceMap.constEnd(); ++it) {
        QImage theImage(*it);
        if (theImage.isNull()) {
            continue;
        }

        message.appendMessagePart(
                QLatin1Literal("<br/>") %
                QLatin1Literal("<img src=\"") %
                (*it) %
                QLatin1Literal("\" style=\"max-width:100%;margin-top:3px\" alt=\"") %
                QLatin1Literal("\" />")
        );
    }
}

QString LatexFilter::handleLatex(const QString &latexFormula)
{
    QString latexText;
    latexText.append(LatexConfig::latexHeader());
    latexText.append(QLatin1String("\\begin{document}\n"));
    latexText.append(QString(QLatin1String("$%1$\n"))
                    .arg(latexFormula));
    latexText.append(QLatin1String("\\end{document}"));

    KTemporaryFile *texFile(new KTemporaryFile);
    texFile->setPrefix(QLatin1String("ktplatex-"));
    texFile->setSuffix(QLatin1String(".tex"));
    if (!texFile->open()) {
      kError() << "Cannot create the TeX file";
      return QString();
    }
    texFile->write(latexText.toAscii());
    texFile->close();
    m_tempFiles << texFile;

    if (LatexConfig::latexCmd().isEmpty()) {
        kError() << "No TeX compiler set!";
        return QString();
    }
    const QStringList latexCmd = LatexConfig::latexCmd().split(QRegExp(QLatin1String("\\s+")));

    QStringList latexArgs;
    Q_FOREACH(const QString &cmd, latexCmd.mid(1, latexCmd.size())) {
        latexArgs << cmd;
    }
    const KStandardDirs outputDir;
    latexArgs << QString(QLatin1String("-output-directory=%1")).arg(outputDir.resourceDirs("tmp").first());
    latexArgs << texFile->fileName();

    if (KStandardDirs::findExe(latexCmd.first()).isEmpty()) {
        kError() << "Cannot find the TeX" << latexCmd.first() << " program.\n;"
                 << "Please get the software from http://tug.org/texlive/"
                 << "or from your distribution's package manager.";
        return QString();
    }

    kDebug() << "Running " << latexCmd.first() << latexArgs;

    KProcess p;
    p.execute(latexCmd.first(), latexArgs);
    if (p.exitCode()) {
        kError() << "Error compiling the TeX text";
        return QString();
    }

    if (KStandardDirs::findExe(QLatin1String("dvipng")).isEmpty()) {
        kError() << "Cannot find the TeX 'dvipng' program.\n;"
                 << "Please get the software from http://tug.org/texlive/"
                 << "or from your distribution's package manager.";
        return QString();
    }

    const QString dviFile = texFile->fileName().replace(QLatin1String(".tex"),QLatin1String(".dvi"));
    const QString imageFile = texFile->fileName().replace(QLatin1String(".tex"),QLatin1String(".png"));

    QStringList dvipngArgs;
    dvipngArgs << QLatin1String("-D300");
    dvipngArgs << QLatin1String("-bgTransparent");
    dvipngArgs << QString(QLatin1String("-o%1")).arg(imageFile);
    dvipngArgs << dviFile;

    kDebug() << "Rendering dvipng" << dvipngArgs;

    p.execute(QLatin1String("dvipng"), dvipngArgs);
    if (p.exitCode()){
        kError() << "Error rendering the image to PNG";
        return QString();
    }

    return imageFile;
}

bool LatexFilter::securityCheck(const QString &latexFormula)
{
    return !latexFormula.contains(QRegExp(QLatin1String(
        "\\\\(def|let|futurelet|newcommand|renewcomment|else|fi|write|input|include"
        "|chardef|catcode|makeatletter|noexpand|toksdef|every|errhelp|errorstopmode|scrollmode|nonstopmode|batchmode"
        "|read|csname|newhelp|relax|afterground|afterassignment|expandafter|noexpand|special|command|loop|repeat|toks"
        "|output|line|mathcode|name|item|section|mbox|DeclareRobustCommand)[^a-zA-Z]"
    )));
}

LatexFilter::~LatexFilter()
{
    qDeleteAll(m_tempFiles);
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<LatexFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_latex"))