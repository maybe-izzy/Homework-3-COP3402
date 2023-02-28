/* $Id: id_attrs.c,v 1.6 2023/02/19 19:07:57 leavens Exp $ */
// Attributes of identifiers in the symbol table
#include <stdlib.h>
#include <stddef.h>
#include "utilities.h"
#include "id_attrs.h"

// Return a freshly allocated id_attrs struct
// with its field tok set to t, kind set to k, 
// and its offset to ofst.
// If there is no space, bail with an error message,
// so this should never return NULL.
extern id_attrs *create_id_attrs(file_location floc, id_kind k,
				 unsigned int ofst)
{
    id_attrs *ret = (id_attrs *)malloc(sizeof(id_attrs));
    if (ret == NULL) {
	bail_with_error("No space to allocate id_attrs!");
    }
    ret->file_loc = floc;
    ret->kind = k;
    ret->offset = ofst;
    return ret;
}

// Return a English version of the kind's name as a string
// (i.e. if k == variable, return "variable", else return "constant")
const char *kind2str(id_kind k)
{
    static const char *kind_names[2] = {"constant", "variable"};
    return kind_names[k];
}
