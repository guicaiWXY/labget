//
// Created by wxy on 12/29/16.
//

#ifndef LABGET_GZIP_H
#define LABGET_GZIP_H

#include "include/zlib/zlib.h"
#include "include/zlib/zconf.h"
#include <cstring>

// function declaration
int gzcompress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata);
int gzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata);
#endif //LABGET_GZIP_H
