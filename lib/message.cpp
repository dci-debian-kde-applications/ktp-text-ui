/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "message.h"
#include <KDebug>

using namespace KTp;

Message::Message(Tp::Message &original)
    : originalMessage(original)
{
    QString htmlMessage= Qt::escape(originalMessage.text());
    htmlMessage.replace(QLatin1String("\n "), QLatin1String("<br/>&nbsp;")); //keep leading whitespaces
    htmlMessage.replace(QLatin1Char('\n'), QLatin1String("<br/>"));
    htmlMessage.replace(QLatin1Char('\t'), QLatin1String("&nbsp; &nbsp; ")); // replace tabs by 4 spaces
    htmlMessage.replace(QLatin1String("  "), QLatin1String(" &nbsp;")); // keep multiple whitespaces
    htmlMessage.replace(QLatin1Char('\\'), QLatin1String("\\\\")); //replace a single backslash with two backslashes.

    setMainMessagePart(htmlMessage);
}

QString Message::mainMessagePart() const
{
    return content[Message::MainMessage];
}

void Message::setMainMessagePart(const QString& message)
{
    content[Message::MainMessage] = message;
}

void Message::appendMessagePart(const QString& part)
{
    content << part;
}

QString Message::finalizedMessage() const
{
    QString msg = content.join(QLatin1String("\n"));
    kDebug() << msg;
    return msg;
}

QVariantMap& Message::miscData()
{
    return m_miscData;
}
