/*=============================================================================
| Assignment: HW 3 - Parser and Declaration Checker for PL/0 
| Authors: Isabelle Montgomery and Christopher Colon Marquez
| Language: C
| Class: COP3402 (0001) - Systems Software - Spring 2023
| Instructor: Gary T. Leavens
| Due Date: 03/21/2023
|
| This program implements a parser and declaration checker for PL/0 in C. 
+=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "lexer_output.h"
#include "parser.h"
#include "ast.h"
#include "utilities.h"
#include "scope_check.h"
#include "symbol_table.h"
#include "unparser.h"

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        // Initializing the parser to work on given input file.
        parser_open(argv[1]);
        
        // Program is parsed by the function 
        // and it returns a pointer to an AST.
        AST *progast = parseProgram();
        
        parser_close();
        
        // Unparse to check on the AST
        unparseProgram(stdout, progast);

        // Build symbol table and check declarations
        scope_initialize();
        scope_check_program(progast);

        return EXIT_SUCCESS;
    }
    else
    {
        printf("Passed incorrect number of command-line arguments");
        return EXIT_FAILURE;
    }
}