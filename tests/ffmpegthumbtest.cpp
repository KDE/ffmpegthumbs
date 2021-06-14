/*
    SPDX-FileCopyrightText: 2010 Andreas Scherf <ascherfy@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ffmpegthumbnailer.h"
#include "ffmpegthumbnailersettings5.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments();

    if (arguments.count() > 1) {
        QString inputFilename(arguments.last());
        QImage image;
        FFMpegThumbnailer *thumbnailer = new FFMpegThumbnailer();
        thumbnailer->create(inputFilename, 128, 128, image);
        QFileInfo fileInfo(inputFilename);
        image.save(fileInfo.baseName() + ".png");
        delete thumbnailer;
    } else {
        cout << "Usage:" << arguments.at(0).toLocal8Bit().data() << " filename" << endl;
    }
    return 0;
}

