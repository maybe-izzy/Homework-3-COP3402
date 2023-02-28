/* $Id: file_location.c,v 1.1 2023/02/19 03:07:27 leavens Exp $ */
#include "file_location.h"

// Return the file location information from a token
file_location token2file_loc(token t)
{
    file_location ret;
    ret.filename = t.filename;
    ret.line = t.line;
    ret.column = t.column;
    return ret;
}
