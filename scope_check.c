/* $Id: scope_check.c,v 1.2 2023/02/22 03:33:43 leavens Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scope_check.h"
#include "id_attrs.h"
#include "file_location.h"
#include "ast.h"
#include "utilities.h"
#include "symbol_table.h"
#include "scope_check.h"

// Build the symbol table for the given program AST
// and Check the given program AST for duplicate declarations
// or uses of identifiers that were not declared
void scope_check_program(AST *prog){
    scope_check_constDecls(prog->data.program.cds); 
    scope_check_varDecls(prog->data.program.vds);
    scope_check_stmt(prog->data.program.stmt);
}

// Put the given name, which is to be declared with var_type vt,
// and has its declaration at the given file location (floc),
// into the current scope's symbol table at the offset scope_size().
static void add_ident_to_scope(const char *name, id_kind vt, file_location floc){
    id_attrs *attrs = scope_lookup(name);
    if (attrs != NULL) {
	    general_error(floc, "%s \"%s\" is already declared as a %s", kind2str(vt), name, kind2str(attrs->kind));
    }
    else {
	    scope_insert(name, create_id_attrs(floc, vt, scope_size()));
    }
}

// build the symbol table and check the declarations in vds
void scope_check_varDecls(AST_list vds){
    while (!ast_list_is_empty(vds)) {
	    scope_check_varDecl(ast_list_first(vds));
	    vds = ast_list_rest(vds);
    }
}

// build the symbol table and check the declarations in cds
void scope_check_constDecls(AST_list cds){
    while (!ast_list_is_empty(cds)) {
	    scope_check_constDecl(ast_list_first(cds));
	    cds = ast_list_rest(cds);
    }
}

// check the var declaration vd
// and add it to the current scope's symbol table
// or produce an error if the name has already been declared
void scope_check_varDecl(AST *vd){
    add_ident_to_scope(vd->data.var_decl.name, variable, vd->file_loc);
}

// check the var declaration vd
// and add it to the current scope's symbol table
// or produce an error if the name has already been declared
void scope_check_constDecl(AST *cd){
    add_ident_to_scope(cd->data.const_decl.name, constant, cd->file_loc);
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_stmt(AST *stmt)
{
    switch (stmt->type_tag) {
    case assign_ast:
	    scope_check_assignStmt(stmt);
	    break;
    case skip_ast:
        // Nothing to check here 
        break; 
    case begin_ast:
	    scope_check_beginStmt(stmt);
	    break;
    case while_ast: 
        scope_check_whileStmt(stmt); 
        break; 
    case if_ast:
	    scope_check_ifStmt(stmt);
	    break;
    case read_ast:
	    scope_check_readStmt(stmt);
	    break;
    case write_ast:
	    scope_check_writeStmt(stmt);
	    break;
    default:
        printf("type tag: %d ", stmt->type_tag); 
	    bail_with_error("Call to scope_check_stmt with an AST that is not a statement!");
	    break;
    }
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_assignStmt(AST *stmt)
{
    scope_check_ident(stmt->file_loc, stmt->data.assign_stmt.name);
    scope_check_expr(stmt->data.assign_stmt.exp);
}

void scope_check_whileStmt(AST *stmt){
    scope_check_expr(stmt->data.while_stmt.cond); 
    scope_check_stmt(stmt->data.while_stmt.stmt); 
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_beginStmt(AST *stmt)
{
    AST_list stmts = stmt->data.begin_stmt.stmts;
    while (!ast_list_is_empty(stmts)) {
	    scope_check_stmt(ast_list_first(stmts));
	    stmts = ast_list_rest(stmts);
    }
}

void scope_check_bin_cond(AST *exp){
    scope_check_expr(exp->data.bin_cond.leftexp); 
    scope_check_expr(exp->data.bin_cond.rightexp); 
}

void scope_check_odd_cond(AST *exp){
    scope_check_expr(exp->data.odd_cond.exp); 
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_ifStmt(AST *stmt){
    scope_check_expr(stmt->data.if_stmt.cond);
    scope_check_stmt(stmt->data.if_stmt.thenstmt);
    scope_check_stmt(stmt->data.if_stmt.elsestmt);
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_readStmt(AST *stmt)
{
    scope_check_ident(stmt->file_loc, stmt->data.read_stmt.name);
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_writeStmt(AST *stmt)
{
    scope_check_expr(stmt->data.write_stmt.exp);
}

// check the expresion to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_expr(AST *exp)
{
    switch (exp->type_tag) {
    case ident_ast:
        scope_check_ident(exp->file_loc, exp->data.ident.name);
        break;
    case bin_cond_ast: 
        scope_check_bin_cond(exp); 
        break; 
    case odd_cond_ast: 
        scope_check_odd_cond(exp); 
        break; 
    case bin_expr_ast:
	    scope_check_bin_expr(exp);
	    break;
    case number_ast:
	    // no identifiers are possible in this case, so just return
	    break;
    default:
	    bail_with_error("Unexpected type_tag (%d) in scope_check_expr (for line %d, column %d)!", exp->type_tag, exp->file_loc.line, exp->file_loc.column);
	    break;
    }
}

// check that the given name has been declared,
// if not, then produce an error using the file_location (floc) given.
void scope_check_ident(file_location floc, const char *name)
{
    if (!scope_defined(name)) {
	    general_error(floc, "identifer \"%s\" is not declared!", name);
    }
}

// check the expression (exp) to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_bin_expr(AST *exp)
{
    scope_check_expr(exp->data.bin_expr.leftexp);
    scope_check_expr(exp->data.bin_expr.rightexp);
}