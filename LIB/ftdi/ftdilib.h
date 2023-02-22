#ifndef FTDILIB_H
#define FTDILIB_H

#include "ftd2xx.h"

class ftdilib
{
public:
    ftdilib();

    static int device_index(const char *serial = 0, FT_STATUS* ftStatus = 0);
    static FT_HANDLE open(const char *serial = 0, FT_STATUS* ftStatus = 0);

};

#endif // FTDILIB_H
