/*
    Copyright (C) 2010  David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt <dev@dominik-schmidt.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "chat-window.h"

#include "chattab.h"

#include <KStandardAction>
#include <KIcon>
#include <KLocale>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KIcon>
#include <KColorScheme>
#include <KTabBar>
#include <KSettings/Dialog>
#include <KNotifyConfigWidget>

#include <TelepathyQt4/TextChannel>

ChatWindow::ChatWindow()
{
    //setup actions
    KStandardAction::quit(KApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());
    KStandardAction::configureNotifications(this, SLOT(showNotificationsDialog()), actionCollection());

    // set up m_tabWidget
    m_tabWidget = new KTabWidget(this);
    m_tabWidget->setTabReorderingEnabled(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setCloseButtonEnabled(true);
    m_tabWidget->setHoverCloseButtonDelayed(true);
    m_tabWidget->setTabBarHidden(true);
    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), this, SLOT(removeTab(QWidget*)));
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(qobject_cast<KTabBar*>(m_tabWidget->tabBar()), SIGNAL(mouseMiddleClick(int)),
                m_tabWidget, SLOT(removeTab(int)));

    setCentralWidget(m_tabWidget);

    setupGUI(static_cast<StandardWindowOptions>(Default^StatusBar), "chatwindow.rc");
}

ChatWindow::~ChatWindow()
{
}

void ChatWindow::startChat(Tp::TextChannelPtr incomingTextChannel)
{
    bool duplicateTab = false;

    // check that the tab requested isn't already open
    for(int index = 0; index < m_tabWidget->count() && !duplicateTab; index++) {

        // get chatWidget object
        ChatTab *auxChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));

        // this should never happen
        if(!auxChatWidget)
            return;

        if(auxChatWidget->textChannel() == incomingTextChannel) {   // got duplicate tab
            duplicateTab = true;
            m_tabWidget->setCurrentIndex(index);                    // set focus on selected tab
        }
    }

    // got new chat, create it
    if(!duplicateTab) {
        ChatTab *chatTab = new ChatWidget(incomingTextChannel, m_tabWidget);
        connect(chatTab, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
        connect(chatTab, SIGNAL(iconChanged(KIcon)), this, SLOT(updateTabIcon(KIcon)));
        connect(chatTab, SIGNAL(userTypingChanged(bool)), this, SLOT(onUserTypingChanged(bool)));

        m_tabWidget->addTab(chatTab, chatTab->icon(), chatTab->title());
        m_tabWidget->setCurrentWidget(chatTab);

        if(m_tabWidget->isTabBarHidden()) {
            if(m_tabWidget->count() > 1) {
                m_tabWidget->setTabBarHidden(false);
            }
        }
    }
}

void ChatWindow::removeTab(QWidget* chatWidget)
{
    m_tabWidget->removePage(chatWidget);
    delete chatWidget;
}

void ChatWindow::setTabText(int index, const QString &newTitle)
{
    m_tabWidget->setTabText(index, newTitle);

    // this updates the window title and icon if the updated tab is the current one
    if (index == m_tabWidget->currentIndex()) {
        onCurrentIndexChanged(index);
    }
}

void ChatWindow::setTabIcon(int index, const KIcon & newIcon)
{
    m_tabWidget->setTabIcon(index, newIcon);

    // this updates the window title and icon if the updated tab is the current one
    if (index == m_tabWidget->currentIndex()) {
        onCurrentIndexChanged(index);
    }
}

void ChatWindow::setTabTextColor(int index, const QColor& color)
{
    m_tabWidget->setTabTextColor(index, color);
}

void ChatWindow::onCurrentIndexChanged(int index)
{
    kDebug() << index;

    if(index == -1) {
        close();
        return;
    }

    ChatTab* currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    setWindowTitle(currentChatTab->title());
    setWindowIcon(currentChatTab->icon());

}

void ChatWindow::onUserTypingChanged(bool isTyping)
{
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        int tabIndex = m_tabWidget->indexOf(sender);
        if (isTyping) {
            setTabTextColor(tabIndex, ChatTab::colorForRole(ChatTab::CurrentlyTyping));
        } else {
            setTabTextColor(tabIndex, ChatTab::colorForRole(ChatTab::Default));
        }
    }
}

void ChatWindow::onContactPresenceChanged(const Tp::Presence& presence)
{
    kDebug();

    ChatTab* sender = qobject_cast<ChatTab*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = m_tabWidget->indexOf(sender);
        setTabIcon(tabIndexToChange, sender->icon());
        setTabText(tabIndexToChange, sender->title());
        setTabTextColor(tabIndexToChange, sender->titleColor());
    }
}

void ChatWindow::onUnreadMessagesChanged()
{
    kDebug();

    ChatTab* sender = qobject_cast<ChatTab*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = m_tabWidget->indexOf(sender);
        if(sender->unreadMessages() > 0) {
            kDebug() << "New unread messages";
            // only change tab color if the widget is hidden
            // the slot is also triggered if the window is not active
            if(!sender->isVisible()) {
                setTabTextColor(tabIndexToChange, ChatTab::colorForRole(ChatTab::UnreadMessages));
            }
        } else {
            kDebug() << "No unread messages anymore";
            setTabTextColor(tabIndexToChange, sender->titleColor());
        }

    }
}

void ChatWindow::showSettingsDialog()
{
    kDebug();

    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    dialog->addModule("kcm_telepathy_chat");
    dialog->addModule("kcm_telepathy_accounts");

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ChatWindow::showNotificationsDialog()
{
    KNotifyConfigWidget::configure(this, "ktelepathy");
}


#include "chat-window.moc"
