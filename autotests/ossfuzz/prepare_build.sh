#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
# SPDX-License-Identifier: LGPL-2.0-or-later

apt-get install -y nasm

# For FFMpegThumbnailer
git clone --depth 1 https://git.ffmpeg.org/ffmpeg.git
