/*=============================================================================
| Assignment: HW 2 - Lexer 
| Authors: Isabelle Montgomery and Christopher Colon Marquez
| Language: C
| Class: COP3402 (0001) - Systems Software - Spring 2023
| Instructor: Gary T. Leavens
| Due Date: 02/14/2023
|
| This program implements a lexical analyzer in C. 
+=============================================================================*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "token.h"
#include "lexer_output.h"
#include "utilities.h"

// Variables associated with lexer 
int done_flag; 
unsigned int line; 
unsigned int column; 
FILE *file_ptr = NULL; 
const char *file_name = NULL; 
char buffer[MAX_IDENT_LENGTH + 1]; 
char legal_symbols[] = {'>', '<', '(', ')', '*', '+', '-', '/', ':', ';', ',', '.', '='}; 

// Returns token type for a string input character 
int string_type(){
    if (strcmp(buffer, "var") == 0){
        return varsym; 
    }
    else if (strcmp(buffer, "do") == 0){
        return dosym; 
    }
    else if (strcmp(buffer, "skip") == 0){
        return skipsym; 
    }
    else if (strcmp(buffer, "const") == 0){
        return constsym; 
    }
    else if (strcmp(buffer, "begin") == 0){
        return beginsym; 
    }
    else if (strcmp(buffer, "call") == 0){
        return callsym; 
    }
    else if (strcmp(buffer, "procedure") == 0){
        return procsym; 
    }
    else if (strcmp(buffer, "end") == 0){
        return endsym; 
    }
    else if (strcmp(buffer, "write") == 0){
        return writesym; 
    }
    else if (strcmp(buffer, "read") == 0){
        return readsym; 
    }
    else if (strcmp(buffer, "if") == 0){
        return ifsym; 
    }
    else if (strcmp(buffer, "then") == 0){
        return thensym; 
    }
    else if (strcmp(buffer, "else") == 0){
        return elsesym; 
    }
    else if (strcmp(buffer, "while") == 0){
        return whilesym; 
    }
    else if (strcmp(buffer, "odd") == 0){
        return oddsym; 
    }
    else {
        return identsym; 
    }
}

// Checks if a character is legal in the lexer's accepted alphabet 
void is_legal(char c){
    if (isalpha(c) || isdigit(c) || isspace(c) || (c == EOF)){
        return; 
    }
    // If character is symbol, verify 
    else {
        for (int i = 0; i < 13; i++){
            if (c == legal_symbols[i]){
                return; 
            }
        }
        char error[50]; 
        sprintf(error, "Illegal character '%c' (%.3o)", c, c); 
        lexical_error(file_name, lexer_line(), lexer_column(), error);
    }
}

// "Resets" the buffer for character intake
void buffer_reset(){
    strcpy(buffer, ""); 
}

void lexer_open(const char *fname){
    FILE *fp = fopen(fname, "r"); 
    
    // Make sure that the program was passed a valid filename 
    if (fp == NULL){
        bail_with_error("Invalid file name");
    }

    // Initialize the lexer
    column = 0; 
    line = 1; 
    file_name = fname; 
    file_ptr = fp; 
    done_flag = 0; 
    buffer_reset(); 
}

void lexer_close(){
    fclose(file_ptr); 
}

bool lexer_done(){
    if (done_flag){
        return true; 
    }
    return false; 
}

const char *lexer_filename(){
    return file_name; 
}

unsigned int lexer_line(){
    return line; 
}

unsigned int lexer_column(){
    int len = strlen(buffer); 
    if (len > 0){
        return (column -  strlen(buffer) + 1);
    }
    else {
        return column; 
    }
}

// Push character to the buffer 
void buffer_cat(char c){
    strncat(buffer, &c, 1); 
}

// Function to assemble a token 
token assemble_token(token_type type){
    token new_token; 

    new_token.typ = type; 
    new_token.column = lexer_column(); 
    new_token.line = lexer_line(); 

    if (type == numbersym){
        new_token.value = atoi(buffer); // Already have error handling for vals > length of short
    }
    if (type == eofsym){
        new_token.text = NULL; 
    }
    else {
        new_token.text = (char*) malloc(sizeof(buffer + 1) * sizeof(char)); 
        strcpy(new_token.text, buffer); 
    }
    new_token.filename = file_name; 
    buffer_reset(); 
    return new_token; 
}

// Gets a character from input and appends it to the running buffer 
char get_character(){
    column++;
    char c = getc(file_ptr);  
    buffer_cat(c);

    return c;
}

// Pushes a character back to input 
void put_back(){
    ungetc(buffer[strlen(buffer) - 1], file_ptr); 
    buffer[strlen(buffer) - 1] = '\0'; 
    column--; 
}

// Eats whitespace/comments until meaningful input is encountered
void eat_characters(){ 
    int stop_eating = 0; 
    char current_char; 

    while (!stop_eating){
        current_char =  get_character(); 
        
        // Handle whitespace
        if (isspace(current_char)){
            if (current_char == '\n'){
                line++; 
                column = 0;  
            }
        }
        // Handle comments 
        else if (current_char == '#'){  
            while (current_char != '\n'){
                current_char = get_character(); 
                if (current_char == EOF){
                    lexical_error(file_name, lexer_line(), column, "File ended while reading comment!");
                }
                // Remove comment character from the buffer 
                buffer_reset(); 
            }
            ungetc(current_char, file_ptr); 
        }
        else {
            stop_eating = 1; 
        }
    }   
    // Put back the meaningful input which was encountered and reset the buffer
    put_back();
    buffer_reset();  
}

token lexer_next(){
    eat_characters(); 
    char error[50]; 

    char current_char = get_character(); 
    is_legal(current_char); 
    char next_char; 

    // Detect end of input 
    if (current_char == EOF){
        done_flag = 1; 
        return assemble_token(eofsym); 
    }
    // Detect keywords and identifiers  
    else if (isalpha(current_char)){ 
        next_char = get_character(); 

        while (isalpha(next_char) || isdigit(next_char)){
            next_char = get_character(); 
            
            // Ensure input is not running beyond the max acceptable length 
            if (strlen(buffer) >= MAX_IDENT_LENGTH){
                lexical_error(file_name, lexer_line(), lexer_column(), "Identifier starting \"%s\" is too long!", buffer);
            }
        }
        put_back(); 
        // Assemble a token with an appropriate type for the string input 
        return assemble_token(string_type()); 
    }
    // Detect numerical input  
    else if (isdigit(current_char)){
        next_char = get_character(); 
    
        while (isdigit(next_char)){
            // Ensure input is not running beyond the max acceptable length 
            if ((atoi(buffer) > SHRT_MAX) || (atoi(buffer) < SHRT_MIN)){
                sprintf(error, "The value of %d is too large for a short!", atoi(buffer)); 
                lexical_error(file_name, lexer_line(), lexer_column(), error);
            }
            next_char = get_character();
        }
        put_back(); 
        return assemble_token(numbersym); 
    }
    // Detect punctuation input 
    else {
        if (current_char == ':'){
            next_char = get_character(); 
            if (next_char == '='){
                return assemble_token(becomessym);  
            }
            else { 
                // Since error is specific to character at current column, use the non-adjusted column value
                lexical_error(file_name, lexer_line(), column, "Expecting '=' after a colon, not '%c'", next_char);
                return assemble_token(-1);  
            }
        }
        else if (current_char == ';'){
            return assemble_token(semisym); 
        }
        else if (current_char == '.'){
            return assemble_token(periodsym); 
        }
        else if (current_char == ','){
            return assemble_token(commasym); 
        }
        else if (current_char == '='){
            return assemble_token(eqsym); 
        }
        else if (current_char == '('){
            return assemble_token(lparensym); 
        }
        else if (current_char == ')'){
            return assemble_token(rparensym); 
        }
        else if (current_char == '<'){
            char next_char = get_character(); 
            if (next_char == '>'){
                return assemble_token(neqsym); 
            }
            else if (next_char == '='){
                return assemble_token(leqsym); 
            }
            else {
                put_back(); 
                return assemble_token(lessym); 
            }
        }
        else if (current_char == '>'){
            char next_char = get_character(); 
            if (next_char == '='){
                return assemble_token(geqsym); 
            }
            else {
                put_back(); 
                return assemble_token(gtrsym); 
            }
        }
        else if (current_char == '+'){
            return assemble_token(plussym); 
        }
        else if (current_char == '-'){
            return assemble_token(minussym); 
        }
        else if (current_char == '/'){
            return assemble_token(divsym); 
        }
        else if (current_char == '*'){
            return assemble_token(multsym); 
        }
        else {
            lexical_error(file_name, line, column, "Unidentified character"); 
            return assemble_token(-1); 
        }
    }
}