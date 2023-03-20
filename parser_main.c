#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "lexer_output.h"
#include "parser.h"
#include "ast.h"
#include "utilities.h"
#include "unparser.h"

int main(int argc, char *argv[]){
    const char *cmdname = argv[0];
    int fileargindex = 1;
    bool produce_lexer_output = false;
    
    if (argc >= 2 && strcmp(argv[1], "-l") == 0) {
	    produce_lexer_output = true;
	    fileargindex = 2;
    } 
    else if (argc == 2) {
	    fileargindex = 1; 
        if (argv[1][0] == '-') {
            //usage(cmdname);
        }
    }
    else {
	    //usage(cmdname);
    }

    // parsing
    if (produce_lexer_output){
        lexer_open(argv[fileargindex]);
        lexer_output();
        lexer_close();
    }
    else {
        parser_open(argv[fileargindex]);
        AST * progast = parseProgram();
        parser_close();
        // unparse to check on the AST
        unparseProgram(stdout, progast);
        
        /*
        // build symbol table and check declarations
        scope_initialize();
        scope_check_program(progast); */
    }
    return EXIT_SUCCESS;
}