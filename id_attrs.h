/* $Id: id_attrs.h,v 1.4 2023/02/19 03:07:27 leavens Exp $ */
#ifndef _ID_ATTRS_H
#define _ID_ATTRS_H
#include "token.h"
#include "file_location.h"

// kinds of entries in the symbol table
typedef enum {constant, variable} id_kind;

// attributes of identifiers in the symbol table
typedef struct {
    // file_loc is the source file location of the identifier's declaration
    file_location file_loc;
    id_kind kind;  // kind of identifier
    unsigned int offset; // offset from beginning of scope
} id_attrs;

// Return a freshly allocated id_attrs struct
// with token t, kind k, and offset ofst.
// If there is no space, bail with an error message,
// so this should never return NULL.
extern id_attrs *create_id_attrs(file_location floc, id_kind k,
				 unsigned int ofst);

// Return a lowercase version of the kind's name as a string
// (i.e. if k == variable, return "variable", else return "constant")
extern const char *kind2str(id_kind k);
#endif
