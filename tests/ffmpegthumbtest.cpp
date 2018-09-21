//    Copyright (C) 2010 Andreas Scherf <ascherfy@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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

