#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>
#include "lexer.h"
#include "id_attrs.h"
#include "utilities.h"
#include "ast.h"

#define NUM_RELATIONALS 6

extern void parser_open(const char *filename); 
extern void parser_close(); 
extern AST *parseProgram(); 
extern AST *parse_skip_stmt(); 
extern AST *parse_ident_expr(); 
extern AST *parse_num_expr();  
extern AST *parse_paren_expr(); 
extern AST *parse_factor(); 
extern AST *parse_add_sub_term(); 
extern AST *parse_mult_div_factor(); 
extern AST *parse_term(); 
extern AST *parse_L_factor(); 
extern AST *parse_expression(); 
extern AST *parse_becomes_stmt(); 
extern AST *parse_stmt(); 
extern AST *parse_begin_stmt(); 
extern AST *parse_write_stmt(); 
extern AST *parse_read_stmt(); 
extern AST *parse_if_stmt(); 
extern AST *parse_condition(); 
extern AST *parse_while_stmt(); 

#endif
