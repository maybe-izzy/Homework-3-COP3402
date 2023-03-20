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

int main(int argc, char *argv[]){
    if (argc == 2) {
        parser_open(argv[1]);
        AST * progast = parseProgram();
        parser_close();
        // unparse to check on the AST
        unparseProgram(stdout, progast);
        
        // build symbol table and check declarations
        scope_initialize();
        scope_check_program(progast); 

        return EXIT_SUCCESS;
    }
    else {
        printf("Passed incorrect number of command-line arguments"); 
        return EXIT_FAILURE; 
    }
}