#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "id_attrs.h"
#include "token.h"
#include "id_attrs.h"
#include "utilities.h"
#include "ast.h"
#include "parser.h"

#define CAN_BEGIN_STMT 7

static token tok;
static char relationals[][3] = {"=", "<>", "<", "<=", ">", ">=" }; 
static token_type begin_stmt_tokens[] = {identsym, beginsym, ifsym, whilesym, readsym, writesym, skipsym}; 

static rel_op get_rel_op(char *op){
    for (int i = 0; i < NUM_RELATIONALS; i++){
        if (strcmp(op, relationals[i]) == 0){
          return i; 
        }
    }
    return -1; 
}

void parser_open(const char *filename){
    lexer_open(filename);
    tok = lexer_next();
}

void parser_close(){
    lexer_close();
}

static void advance(){
    if (!lexer_done()) {
	    tok = lexer_next();
    }
}

static void eat(token_type tt) {
    if (tok.typ == tt) {
        advance();
    }
    else {
        token_type expected[1] = {tt};
        parse_error_unexpected(expected, 1, tok);
    }
}

AST *parse_skip_stmt(){
    token skip_tok = tok; 
    eat(skipsym); 
    return ast_skip_stmt(skip_tok); 
}

AST *parse_ident_expr(){
    token idt = tok;
    eat(identsym);
    return ast_ident(idt, idt.text);
}

AST *parse_num_expr(){
    token t = tok; 
    token num_tok; 
    short mult = 1; 
    token_type expected[] = {plussym, minussym, numbersym};

    switch (tok.typ){
        case (plussym): 
            t = tok; 
            eat(plussym); 
        case (minussym): 
            t = tok; 
            mult = -1; 
            eat(minussym); 
        case (numbersym): 
            num_tok = tok; 
            eat(numbersym); 
            break;
        default: 
            parse_error_unexpected(expected, 3, tok);
    }   
    return ast_number(t, num_tok.value * mult);
}

AST *parse_paren_expr(){
    token left_par = tok;
    eat(lparensym);
    AST *ret = parse_expression();
    eat(rparensym);
    ret->file_loc = token2file_loc(left_par);
    return ret;
}

AST *parse_factor(){
    AST *exp = NULL; 

    switch(tok.typ){
        case (identsym):
            exp = parse_ident_expr();
            break;
        case (plussym): 
            exp = parse_num_expr(); 
            break;
        case (minussym): 
            exp = parse_num_expr(); 
            break; 
        case (numbersym): 
            exp = parse_num_expr();
            break; 
        case (lparensym): 
            exp = parse_paren_expr(); 
            break; 
        default: 
            break; 
    } 
    return exp; 
}

AST *parse_add_sub_term(){
    token opt = tok;
    token_type expected[] = {plussym, minussym};

    switch (tok.typ) {
        case plussym:
	        eat(plussym);
            AST *exp = parse_term();
            return ast_op_expr(opt, addop, exp);
            break;
        case minussym:
            eat(minussym);
            AST *e = parse_term();
            return ast_op_expr(opt, subop, e);
            break;
        default:
            parse_error_unexpected(expected, 2, tok);
            break;
    }
    // The following should never execute
    return (AST *) NULL;
}


AST *parse_mult_div_factor(){
    token_type expected[] = {multsym, divsym};
    token opt = tok;

    switch (tok.typ) {
        case multsym:
        eat(multsym);
	    AST *exp = parse_factor();
	    return ast_op_expr(opt, multop, exp);
	    break;
    case divsym:
	    eat(divsym);
	    AST *e = parse_factor();
	    return ast_op_expr(opt, divop, e);
	    break;
    default:
	    parse_error_unexpected(expected, 2, tok);
	    break;
    }
    // The following should never execute
    return (AST *) NULL;
}

AST *parse_term(){
    token first = tok; 
    AST *factor = parse_factor(); 
    while (tok.typ == multsym || tok.typ == divsym) {
	    AST *rght = parse_mult_div_factor();
	    factor = ast_bin_expr(first, factor, rght->data.op_expr.arith_op, rght->data.op_expr.exp);
    }

    return factor; 
}

AST *parse_expression(){
    token fst = tok;
    AST *exp = parse_term();
    while (tok.typ == plussym || tok.typ == minussym) {
	    AST *rght = parse_add_sub_term();
	    exp = ast_bin_expr(fst, exp, rght->data.op_expr.arith_op, rght->data.op_expr.exp);
    }
    return exp;
}

AST *parse_becomes_stmt(){
    token ident_tok = tok; 
    eat(identsym); 
    eat(becomessym); 
    AST *exp = parse_expression(); 
    return ast_assign_stmt(ident_tok, ident_tok.text, exp); 
}

AST *parse_begin_stmt(){
    token begin_tok = tok;
    eat(beginsym);
    AST_list stmts = ast_list_singleton(parse_stmt());
    AST_list last = stmts;
    while (tok.typ == semisym) {
        eat(semisym); 
	    AST *stmt = parse_stmt();
	    ast_list_splice(last, stmt);
        last = stmt; 
    }
    eat(endsym);
    AST *ret = ast_begin_stmt(begin_tok, stmts);
    return ret;
}

AST *parse_read_stmt(){
    token rt = tok;
    eat(readsym);
    const char *name = tok.text;
    eat(identsym);
    return ast_read_stmt(rt, name);
}

AST *parse_write_stmt(){
    token wt = tok;
    eat(writesym);
    AST *exp = parse_expression();
    return ast_write_stmt(wt, exp);
}

AST *parse_if_stmt(){
    token if_tok = tok;
    eat(ifsym);
    AST *if_cond = parse_condition();
    eat(thensym); 
    AST *then_stmt = parse_stmt();
    eat(elsesym); 
    AST *else_stmt = parse_stmt(); 
    return ast_if_stmt(if_tok, if_cond, then_stmt, else_stmt);
}

static bool can_start_exp(token_type tt){
    switch (tt){
        case (plussym):
            return true; 
            break; 
        case (minussym): 
            return true; 
            break; 
        case (numbersym): 
            return true;
            break; 
        case (identsym): 
            return true; 
            break; 
        case (lparensym):
            return true; 
            break; 
        default: 
            return false; 
    }
}

AST *parse_while_stmt(){
  token while_tok = tok;
  eat(whilesym); 
  AST *cond = parse_condition(); 
  eat(dosym); 
  AST *stmt = parse_stmt(); 
  return ast_while_stmt(while_tok, cond, stmt); 
}

AST *parse_condition(){
  if (tok.typ == oddsym){
      token odd_tok = tok; 
      eat(oddsym); 
      AST *exp = parse_expression(); 
      return ast_odd_cond(odd_tok, exp);
  }
  else if (can_start_exp(tok.typ)){
      token start_tok = tok; 
      AST *exp1 = parse_expression(); 
      token op_tok = tok;
      rel_op op = get_rel_op(op_tok.text); 

      // If not a relational operator 
      if (op == -1){
        token_type expected[6] = {eqsym, neqsym, lessym, leqsym, gtrsym, geqsym}; 
        parse_error_unexpected(expected, 6, tok);
      }
      eat(op_tok.typ); 
      AST *exp2 = parse_expression(); 
      return ast_bin_cond(start_tok, exp1, op, exp2); 
  }
  else {
      token_type expected[] = {oddsym, plussym, minussym, numbersym, identsym, lparensym};
      parse_error_unexpected(expected, 6, tok);
  }
  // Should never execute
  return (AST *) NULL; 
}

AST *parse_stmt(){
    AST* ret = NULL;
    switch (tok.typ) {
        case identsym: 
            ret = parse_becomes_stmt(); 
            break; 
        case beginsym: 
            ret = parse_begin_stmt(); 
            break;
        case ifsym: 
            ret = parse_if_stmt(); 
            break; 
        case whilesym: 
            ret = parse_while_stmt(); 
            break;
        case readsym: 
            ret = parse_read_stmt(); 
            break; 
        case writesym: 
            ret = parse_write_stmt(); 
            break;  
        case skipsym:
            ret = parse_skip_stmt();
            break; 
        default:
            parse_error_unexpected(begin_stmt_tokens, CAN_BEGIN_STMT, tok);
    } 
    return ret;
}


static AST_list parseConstDecl(){
    token ident_tok = tok; 
    eat(identsym); 
    eat(eqsym); 
    token num_tok = tok; 
    eat(numbersym); 
    return ast_list_singleton(ast_const_def(ident_tok, ident_tok.text, num_tok.value));  
}

static AST_list parseConstDecls(){
    AST_list head = ast_list_empty_list();  

    if (tok.typ == constsym){
        eat(constsym);
        head = parseConstDecl(); 
        AST_list last = head;
        while (tok.typ == commasym) {
            eat(commasym);
            AST_list const_list = parseConstDecl();
            ast_list_splice(last, const_list);
            last = const_list; 
        }
        if (tok.typ == semisym){
            eat(semisym); 
        }
        ast_list_splice(last, parseConstDecls());          
    } 
    return head; 
}

static AST_list parseVarDecl(){
    token idtok = tok; 
    eat(identsym); 
    return ast_list_singleton(ast_var_decl(idtok, idtok.text));  
}

static AST_list parseVarDecls(){
    AST_list head = ast_list_empty_list();  

    if (tok.typ == varsym){
        eat(varsym); 
        head = parseVarDecl(); 
        AST_list last = head;
        while (tok.typ == commasym) {
            eat(commasym);
            AST_list var_list = parseVarDecl();
            ast_list_splice(last, var_list);
            last = var_list; 
        }
        if (tok.typ == semisym){
            eat(semisym); 
        }
        ast_list_splice(last, parseVarDecls());          
    }
    return head; 
}

AST *parseProgram(){
    AST_list cds = parseConstDecls();
    AST_list vds = parseVarDecls();
    AST* stmts = parse_stmt();
    eat(periodsym); 
    eat(eofsym);

    file_location floc; 
    if (!ast_list_is_empty(vds)) {
        if (ast_list_first(vds)->type_tag == var_decl_ast) {
            floc = ast_list_first(vds)->file_loc;
        }
        else {
            bail_with_error("Bad AST for var declarations");
        }
    }
    else {
	    floc = stmts->file_loc;
    } 
    return ast_program(floc.filename, floc.line, floc.column, cds, vds, stmts);
}