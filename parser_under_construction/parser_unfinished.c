
#include <stdio.h>
#include "lexer.h"
#include "id_attrs.h"
#include "scope_symtab.h"
#include "utilities.h"
#include "parser.h"

// Token obtained from lexer.
static token tok;

static unsigned int scope_offset;

// initialize the parser to work on the given file
void parser_open(const char *filename)
{
    lexer_open(filename);
    tok = lexer_next();
    scope_offset = 0;
}

// finish using the parser
void parser_close()
{
    lexer_close();
}

// Advance to the next token
// (if the lexer is not done,
//  otherwise do nothing)
static void advance()
{
    if (!lexer_done()) 
    {
	    tok = lexer_next();
    }
}

// Check that the current token is of type tt
// and advance to the next token if so;
// otherwise, produce a syntax error.
static void eat(token_type tt) 
{
    if (tok.typ == tt) 
    {
	    // debug_print("Eating a %s token\n", ttyp2str(tt));
        advance();
    } 
    else 
    {
	    token_type expected[1] = {tt};
	    parse_error_unexpected(expected, 1, tok);
    }
}

// ⟨program⟩ ::= ⟨block⟩ .
// ⟨block⟩ ::= ⟨const-decls⟩ ⟨var-decls⟩ ⟨stmt⟩
AST *parseProgram()
{
    AST_list cds = parseConstDecls();
    AST_list vds = parseVarDecls();
    AST *stmt = parseStmt();
    
    eat(periodsym);
    eat(eofsym);
    //.....................................> THIS BLOCK NEEDS TO BE FIXED <<<<<<<<<<<<<<<<<
    file_location floc;

    if (!ast_list_is_empty(vds)) 
    {
        if (ast_list_first(vds)->type_tag == var_decl_ast) 
        {
            floc = ast_list_first(vds)->file_loc;
        } 
        else 
        {
            bail_with_error("Bad AST for var declarations");
        }
    } 
    else 
    {
        floc = stmt->file_loc;
    }
    //..........................................................................^^^^^^^^^^^^^^^^^
    return ast_program(floc.filename, floc.line, floc.column, cds,vds, stmt);
}

// Requires: !ast_list_is_empty(*head) ==> !ast_list_is_empty(*last).
// Requires: when called head points to the first element of an AST_list
// and last points to the last element in that list.
// Modifies *head, *last;
// Splice the list starting at lst into the AST list starting at *head,
// and make *last point to the last element in the resulting list.
static void add_AST_to_end(AST_list *head, AST_list *last, AST_list lst)
{
    if (ast_list_is_empty(*head)) 
    {
	    *head = lst;
	    *last = ast_list_last_elem(lst);
    } 
    else 
    {
	    // assert(*last != NULL);
	    ast_list_splice(*last, lst);
	    *last = ast_list_last_elem(lst);
    }
}

// ⟨const-decls⟩ ::= {⟨const-decl⟩}
// ⟨const-decl⟩ ::= const ⟨const-def⟩ {⟨comma-const-def⟩} ;
// ⟨const-def⟩ ::= ⟨ident⟩ = ⟨number⟩
// ⟨comma-const-def⟩ ::= , ⟨const-def⟩


// ⟨var-decls⟩ ::= {⟨var-decl⟩}
// ⟨var-decl⟩ ::= var ⟨idents⟩ ;
static AST_list parseVarDecls()
{
    AST_list ret = ast_list_empty_list();
    AST_list last = ast_list_empty_list();
    
    while (tok.typ == varsym) 
    {
	    AST_list vdasts;
	    
        var_type vt = var_t;
        eat(varsym);

        vdasts = parseIdents(vt);
        
        eat(semisym);
        
        add_AST_to_end(&ret, &last, vdasts);
    }
    return ret;
}

// <idents> ::= <ident> { <comma-ident> }
static AST_list parseIdents(var_type vt)
{
    token ident_token = tok;
    eat(identsym);
    
    AST_list ret = ast_list_singleton(ast_var_decl(ident_token, vt, ident_token.text));
    AST_list last = ret;
    
    while (tok.typ == commasym) 
    {
	    eat(commasym);
	
        token ident_tok = tok;
        eat(identsym);
        
        AST *vd = ast_var_decl(ident_tok, vt, ident_tok.text);
        
        add_AST_to_end(&ret, &last, ast_list_singleton(vd));
    }
    return ret;
}

// The tokens that start a statement for PL/0
#define STMTBEGINTOKS 7
static token_type can_begin_stmt[STMTBEGINTOKS] = 
        {identsym, beginsym, ifsym, whilesym, readsym, writesym, skipsym};


// Checking is token start a statement
static bool is_stmt_beginning_token(token t)
{
    for (int i = 0; i < STMTBEGINTOKS; i++) 
    {
	    if (t.typ == can_begin_stmt[i]) 
        {
	        return true;
	    }
    }
    return false;
}

// <stmt> ::= <ident> = <expr>  | ...
AST *parseStmt()
{
    AST *ret = NULL;
    switch (tok.typ) 
    {
        case identsym:
            ret = parseAssignStmt();
            break;
        case beginsym:
            ret = parseBeginStmt();
            break;
        case ifsym:
            ret = parseIfStmt(); // Working on this function
            break;
        case whilesym:
            ret = parseWhileStmt(); 
            break;
        case readsym:
            ret = parseReadStmt();
            break;
        case writesym:
            ret = parseWriteStmt();
            break;
        case skipsym:
            break;
        default:
            parse_error_unexpected(can_begin_stmt, STMTBEGINTOKS, tok);
    }
    return ret;
}

// ⟨stmt⟩ ::= ⟨ident⟩ := ⟨expr⟩
static AST *parseAssignStmt()
{
    token ident_token = tok;

    eat(identsym);
    eat(becomessym);
    
    AST *exp = parseExpr();

    return ast_assign_stmt(ident_token, ident_token.text, exp);
}

// begin ⟨stmt⟩ {⟨semi-stmt⟩} end
static AST_list parseBeginStmt()
{
    token begin_token = tok;
    eat(beginsym);
    
    AST_list stmts = ast_list_singleton(parseStmt());
    AST_list last = stmts;
    
    while (tok.typ == commasym) 
    {
        eat(commasym);
        
	    AST *stmt = parseStmt();
	    add_AST_to_end(&stmts, &last, ast_list_singleton(stmt));
    }
    eat(endsym);

    AST *ret = ast_begin_stmt(begin_token, stmts);
    
    return ret;
}

// if ⟨condition⟩ then ⟨stmt⟩ else ⟨stmt⟩
static AST *parseIfStmt()
{
    token if_token = tok;
    eat(ifsym);
    AST *cond = parseCond();
    
    token then_token = tok;
    eat(thensym);
    AST *s1 = parseStmt();
    
    token else_token = tok;
    eat(elsesym);
    AST *s2 = parseStmt();

    
    return ast_if_stmt(if_token, cond, s1, s2);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>START: WORKING ON<<<<<<<<<<<<<<<<<<<<<<<<<

static AST *parseWhileStmt(); 
            
static AST *parseReadStmt();

static AST *parseWriteStmt();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>^^END: WORKING ON^^<<<<<<<<<<<<<<<<<<<<<<<<<

// ⟨condition⟩ ::= odd ⟨expr⟩ | ⟨expr⟩ ⟨rel-op⟩ ⟨expr⟩
AST *parseCond()
{
    token condition_token = tok;
    AST *exp;
    // check if the token is the "odd" symbol
    if (tok.typ == oddsym)
    {
        eat(oddsym);
        AST * odd_exp = parseExpr();
        odd_exp = ast_odd_cond(condition_token, odd_exp);
    }
    else
    {

        //not odd need to be finished
        // ⟨expr⟩ ⟨rel-op⟩ ⟨expr⟩
    }
    return exp;
}

// ⟨expr⟩ ::= ⟨term⟩ { ⟨add-sub-term⟩ }
static AST *parseExpr()
{
    token fst = tok;
    AST *exp = parseTerm();

    while(tok.typ == plussym || tok.typ == minussym)
    {
        AST * add_sub_term = parseAddSubTerm();

        exp = ast_bin_expr(fst, exp, add_sub_term->data.op_expr.arith_op, add_sub_term->data.op_expr.exp);
    }
    return exp;
}

// ⟨add-sub-term⟩ ::= ⟨add-sub⟩ ⟨term⟩
static AST *parseAddSubTerm()
{
    token operator_token = tok;
    
    switch (tok.typ)
    {
        case plussym:
            
            eat(plussym);
            AST *exp = parseTerm();
            return ast_op_expr(operator_token, addop, exp);
            break;
        
        case minussym:

            eat(minussym);
            AST *expr = parseTerm();
            return ast_op_expr(operator_token, subop, expr);
            break;
        
        default:
            
            token_type expected[2] = {plussym, minussym};
            parse_error_unexpected(expected, 2, tok);
            break;
    }
    return (AST*) NULL;
}


// ⟨term⟩ ::= ⟨factor⟩ { ⟨mult-div-factor⟩ }
static AST *parseTerm()
{
    token fct = tok;
    
    AST * factor = parseFactor();
    AST *exp = factor;

    while(tok.typ == multsym || tok.typ == divsym)
    {
        AST *mult_div_factor = parseMultDivFactor(); 
        
        exp = ast_bin_expr(fct,  exp, mult_div_factor->data.op_expr.arith_op, mult_div_factor->data.op_expr.exp);
    }
    return exp;
}

// ⟨mult-div-factor⟩ ::= ⟨mult-div⟩ ⟨factor⟩
static AST *parseMultDivFactor()
{
    token operator_token = tok;
    switch (tok.typ)
    {
        case multsym:

            eat(multsym);
            AST *exp = parseFactor();
            return ast_op_expr(operator_token, multop, exp);
            break;
        
        case divsym:

            eat(divsym);
            AST *expr = parseFactor();
            return ast_op_expr(operator_token, divop, expr);

        default:

            token_type expected[2] = {multsym, divsym};
            parse_error_unexpected(expected, 2, tok);
            break;
    }
    return (AST*) NULL;
}

// ⟨factor⟩ ::= ⟨ident⟩ | ⟨sign⟩ ⟨number⟩ | ( ⟨expr⟩ )
static AST *parseFactor()
{
    switch (tok.typ)
    {
        case identsym:
            return parseIdentExpr();
            break;
        
        case numbersym:
            parseNumberExpr();
            break;
        
        case lparensym:
            parseParenExpr();
            break;
        default:
            token_type expected[3] = {identsym, numbersym, lparensym};
            parse_error_unexpected(expected, 3, tok);
            break;
    }
    return (AST*) NULL;
}

// ⟨ident⟩ ::= ⟨letter⟩ { ⟨letter-or-digit⟩ }
static AST *parseIdentExpr()
{   
    token ident = tok;
    eat(identsym);
    return ast_ident(ident, ident.text);   
}

// ⟨sign⟩ ::= ⟨plus⟩ | ⟨minus⟩ | ⟨empty⟩
// ⟨number⟩ ::= ⟨digit⟩ {⟨digit⟩}
static AST *parseNumberExpr()
{
    token numExp_token = tok;
    
    eat(numbersym);
    
    int val = numExp_token.value;
    
    return ast_number(numExp_token, val);
}

// ( <expr> )
static AST *parseParenExpr()
{
    token leftPar_tk = tok;
    
    eat(lparensym);
    
    AST *ret = parseExpr();
    
    eat(rparensym);
    
    ret->file_loc = token2file_loc(leftPar_tk);
    
    return ret;
}