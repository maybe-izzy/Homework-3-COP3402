#ifndef _SCOPE_CHECK_H
#define _SCOPE_CHECK_H
#include "ast.h"

// Builds the symbol table and checks the program's AST for
// duplicate declarations or uses of identifiers not declared.
extern void scope_check_program(AST *prog);

// Builds the symbol table and checks the declarations in vds.
extern void scope_check_varDecls(AST *vds);

// Checks the var declaration and adds it to the current 
// scope's symbol table. Error produced if the name has already been declared.
extern void scope_check_varDecl(AST *vd);

// The following functions are tasked with making sure that
// all identifiers referenced in them have been declared.
// If not, then an error is produced.
extern void scope_check_stmt(AST *stmt);
extern void scope_check_assignStmt(AST *stmt);
extern void scope_check_whileStmt(AST *stmt); 
extern void scope_check_bin_cond(AST *exp); 
extern void scope_check_odd_cond(AST *exp); 
extern void scope_check_constDecl(AST *cd);
extern void scope_check_constDecls(AST_list cds); 
extern void scope_check_beginStmt(AST *stmt);
extern void scope_check_ifStmt(AST *stmt);
extern void scope_check_readStmt(AST *stmt);
extern void scope_check_writeStmt(AST *stmt);
extern void scope_check_expr(AST *exp);
extern void scope_check_ident(file_location floc, const char *name);
extern void scope_check_bin_expr(AST *exp);

#endif