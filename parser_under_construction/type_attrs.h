/* $Id: type_attrs.h,v 1.1 2023/02/22 03:33:43 leavens Exp $ */
#ifndef _TYPE_ATTRS_H
#define _TYPE_ATTRS_H

typedef enum {var_t} var_type;

// Return a string representing the var_type given
extern const char *vt2str(var_type vt);

#endif