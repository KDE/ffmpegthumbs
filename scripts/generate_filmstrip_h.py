#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Ryan <fauxpark@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import sys

from pathlib import Path
from PIL import Image

FILMSTRIP_SIZES = [4, 8, 16, 32, 64]
MAX_LINE_LENGTH = 8

img_src_dir = Path(__file__).parent.resolve()
dest_file = Path(sys.argv[1])


if __name__ == '__main__':
    with open(dest_file, 'w') as f:
        def generate_filmstrip_array(size):
            filename = img_src_dir / f'filmStrip{size}.png'

            img = Image.open(filename)
            img_data = img.getdata()

            if img.width != size or img.height != size:
                print(f'{filename}: Image dimensions must be {size}x{size}!')
                exit(1)

            line_length = min(img.width, MAX_LINE_LENGTH)

            f.write(f'static const quint8 filmStrip{size}[{size} * {size} * 3] = {{\n')

            for i, v in enumerate(img_data):
                if i % line_length == 0:
                    f.write('    ')

                f.write(f'0x{v[0]:02X}, 0x{v[1]:02X}, 0x{v[2]:02X}')

                if i < len(img_data) - 1:
                    f.write(',')

                    if (i + 1) % line_length > 0:
                        f.write(' ')

                if (i + 1) % line_length == 0:
                    f.write('\n')

            f.write('};\n')

        f.write('/****************************************************\n')
        f.write(' *              THIS FILE IS GENERATED!             *\n')
        f.write(' *                                                  *\n')
        f.write(' * To regenerate, run:                              *\n')
        f.write(' *   ../scripts/generate_filmstrip_h.py filmstrip.h *\n')
        f.write(' ****************************************************/\n')
        f.write('\n')
        f.write('#ifndef FILMSTRIP_H\n')
        f.write('#define FILMSTRIP_H\n')
        f.write('\n')
        f.write('#include <QtGlobal>\n')
        f.write('\n')
        f.write(f'static const quint32 SMALLEST_FILM_STRIP_WIDTH = {min(FILMSTRIP_SIZES)};\n')
        f.write('\n')

        for s in FILMSTRIP_SIZES:
            generate_filmstrip_array(s)
            f.write('\n')

        f.write('#endif\n')
