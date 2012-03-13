/*
    KDE Telepathy AdiumxtraProtocolHandler - Install Adiumxtra packages through adiumxtra://-pseudo protocol
    Copyright (C) 2010 Dominik Schmidt <domme@rautelinux.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "adiumxtra-protocol-handler.h"

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>

int main(int argc, char *argv[])
{
    KAboutData aboutData("ktp-adiumxtra-protocol-handler",
                         0,
                         ki18n("AdiumXtra Protocol Handler"),
                         "0.3");
    aboutData.addAuthor(ki18n("Dominik Schmidt"), ki18n("Developer"), "kde@dominik-schmidt.de");
    aboutData.setProductName("telepathy/text-ui"); //set the correct name for bug reporting
    aboutData.setLicense(KAboutData::License_GPL_V2);
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("!+install-chatstyles", ki18n("Install Adium packages"));
    KCmdLineArgs::addCmdLineOptions(options);


    AdiumxtraProtocolHandler app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for(int i = 0; i < args->count(); i++) {
        kDebug() << "install: " << args->arg(i);
        app.install(args->arg(i));
    }
    args->clear();

    return app.exec();
}
