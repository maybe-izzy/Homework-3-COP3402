#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "lexer_output.h"
#include "parser.h"
#include "ast.h"
//#include "scope_symtab.h"
//#include "scope_check.h"
#include "utilities.h"
#include "unparser.h"


int main(int argc, char *argv[])
{
    --argc;
    
    // Calling parser. 
    if (argc == 1) 
	{  
	    parser_open(argv[1]);
		
		AST * progast = parseProgram();
		
		parser_close();
		
		// unparse to check on the AST
		unparseProgram(stdout, progast);

		// build symbol table and check declarations
		scope_initialize();
		scope_check_program(progast); 
    }
    else 
	{
		// Throw error if received invalid command line args.
	    bail_with_error("Invalid arguments in lexer invocation"); 
    }
	return EXIT_SUCCESS;
}