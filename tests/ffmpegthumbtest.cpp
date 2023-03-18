/*
    SPDX-FileCopyrightText: 2010 Andreas Scherf <ascherfy@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ffmpegthumbnailer.h"
#include "ffmpegthumbnailersettings5.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStringList>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments();

    if (arguments.count() > 1) {
        QString inputFilename(arguments.last());
        FFMpegThumbnailer thumbnailer(&app, QVariantList());
        QMimeDatabase db;
        const QString mime = db.mimeTypeForFile(inputFilename).name();
        KIO::ThumbnailRequest request(QUrl::fromLocalFile(inputFilename), QSize(128, 128), mime, 0, 0);
        const auto res = thumbnailer.create(request);
        QFileInfo fileInfo(inputFilename);
        res.image().save(fileInfo.baseName() + QStringLiteral(".png"));
    } else {
        cout << "Usage:" << arguments.at(0).toLocal8Bit().data() << " filename" << endl;
    }
    return 0;
}
