
/**
 * @file
 * @brief General 
 */

#ifndef HANDLEBARS_H
#define HANDLEBARS_H

#ifdef	__cplusplus
extern "C" {
#endif

// Declarations
struct handlebars_context;
struct handlebars_token_list;

/**
 * @brief Enumeration of error types
 */
enum handlebars_error_type
{
    /**
     * @brief Indicates that no error has occurred
     */
    HANDLEBARS_SUCCESS = 0,
    
    /**
     * @brief Indicates that a generic error has occurred 
     */
    HANDLEBARS_ERROR = 1,
    
    /**
     * @brief Indicates that failure was due to an allocation failure
     */
    HANDLEBARS_NOMEM = 2,
    
    /**
     * @brief Indicates that failure was due to a null pointer argument
     */
    HANDLEBARS_NULLARG = 3,
    
    /**
     * @brief Indicates that failure was due to a parse error
     */
    HANDLEBARS_PARSEERR = 4
};

/**
 * @brief Get the library version as an integer
 * @return The version of handlebars as an integer
 */
int handlebars_version(void);

/**
 * @brief Get the library version as a string
 * @return The version of handlebars as a string
 */
const char * handlebars_version_string(void);

/**
 * @brief Convenience function for lexing to a token list
 * 
 * @param[in] ctx
 * @return the token list
 */
struct handlebars_token_list * handlebars_lex(struct handlebars_context * ctx);

// Flex/Bison prototypes
int handlebars_yy_get_column(void * yyscanner);
void handlebars_yy_set_column(int column_no, void * yyscanner);
int handlebars_yy_parse(struct handlebars_context * context);

#ifdef	__cplusplus
}
#endif

#endif
