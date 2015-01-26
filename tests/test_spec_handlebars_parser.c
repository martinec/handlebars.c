
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <stdio.h>
#include <talloc.h>

#if defined(HAVE_JSON_C_JSON_H)
#include "json-c/json.h"
#include "json-c/json_object.h"
#include "json-c/json_tokener.h"
#elif defined(HAVE_JSON_JSON_H)
#include "json/json.h"
#include "json/json_object.h"
#include "json/json_tokener.h"
#endif

#include "handlebars.h"
#include "handlebars_ast_printer.h"
#include "handlebars_context.h"
#include "handlebars_memory.h"
#include "handlebars_token.h"
#include "handlebars_token_list.h"
#include "handlebars_token_printer.h"
#include "handlebars_utils.h"
#include "handlebars.tab.h"
#include "handlebars.lex.h"
#include "utils.h"

struct parser_test {
    char * description;
    char * it;
    char * tmpl;
    char * expected;
    int exception;
    char * message;
    TALLOC_CTX * mem_ctx;
};

static struct parser_test * tests;
static size_t tests_len = 0;
static size_t tests_size = 0;

static void loadSpecTest(json_object * object)
{
    json_object * cur = NULL;
    int nreq = 0;
    
    // Get test
    struct parser_test * test = &(tests[tests_len++]);
    
    // Get description
    cur = json_object_object_get(object, "description");
    if( cur && json_object_get_type(cur) == json_type_string ) {
        test->description = strdup(json_object_get_string(cur));
    }
    
    // Get it
    cur = json_object_object_get(object, "it");
    if( cur && json_object_get_type(cur) == json_type_string ) {
        test->it = strdup(json_object_get_string(cur));
    }
    
    // Get template
    cur = json_object_object_get(object, "template");
    if( cur && json_object_get_type(cur) == json_type_string ) {
        test->tmpl = strdup(json_object_get_string(cur));
    }
    
    // Get expected
    cur = json_object_object_get(object, "expected");
    if( cur && json_object_get_type(cur) == json_type_string ) {
        test->expected = strdup(json_object_get_string(cur));
        nreq++;
    }
    
    // Get exception
    cur = json_object_object_get(object, "exception");
    if( cur && json_object_get_type(cur) == json_type_boolean ) {
        test->expected = (int) json_object_get_boolean(cur);
        nreq++;
    }
    
    // Get message
    cur = json_object_object_get(object, "message");
    if( cur && json_object_get_type(cur) == json_type_string ) {
        test->message = strdup(json_object_get_string(cur));
        nreq++;
    }
    
    // Check
    if( nreq <= 0 ) {
        fprintf(stderr, "Warning: expected or exception/message must be specified\n");
    }
}

static void loadSpec(char * filename) {
    int error = 0;
    char * data = NULL;
    size_t data_len = 0;
    struct json_object * result = NULL;
    struct json_object * array_item = NULL;
    int array_len = 0;
    
    // Read JSON file
    error = file_get_contents((const char *) filename, &data, &data_len);
    if( error != 0 ) {
        fprintf(stderr, "Failed to read spec file: %s, code: %d\n", filename, error);
        exit(1);
    }
    
    // Parse JSON
    result = json_tokener_parse(data);
    // @todo: parsing errors seem to cause segfaults....
    if( result == NULL ) {
        fprintf(stderr, "Failed so parse JSON\n");
        exit(1);
    }
    
    // Root object should be array
    if( json_object_get_type(result) != json_type_array ) {
        fprintf(stderr, "Root JSON value was not array\n");
        exit(1);
    }
    
    // Get number of test cases
    array_len = json_object_array_length(result);
    
    // Allocate tests array
    tests_size = array_len + 1;
    tests = calloc(tests_size, sizeof(struct parser_test));
    
    // Iterate over array
    for( int i = 0; i < array_len; i++ ) {
        array_item = json_object_array_get_idx(result, i);
        if( json_object_get_type(array_item) != json_type_object ) {
            fprintf(stderr, "Warning: test case was not an object\n");
            continue;
        }
        loadSpecTest(array_item);
    }
}

START_TEST(handlebars_spec_parser)
{
    struct parser_test * test = &tests[_i];
    struct handlebars_context * ctx = handlebars_context_ctor();
    int retval;
    char * errmsg;
    char errlinestr[32];
    
    ctx->tmpl = test->tmpl;
    
    retval = handlebars_yy_parse(ctx);
    
    if( ctx->error != NULL ) {
        snprintf(errlinestr, sizeof(errlinestr), " on line %d, column %d", 
                ctx->errloc->last_line, 
                ctx->errloc->last_column);
        
        errmsg = handlebars_talloc_strdup(ctx, ctx->error);
        errmsg = handlebars_talloc_strdup_append(errmsg, errlinestr);
        
        ck_assert_msg(0, errmsg);
    } else {
        char * output = handlebars_ast_print(ctx->program, 0);
        
        ck_assert_str_eq(test->expected, output);
    }
  
    handlebars_context_dtor(ctx);
}
END_TEST

Suite * parser_suite(void)
{
    const char * title = "Handlebars Parser Spec";
    Suite * s = suite_create(title);
    
    TCase * tc_handlebars_spec_parser = tcase_create(title);
    // tcase_add_checked_fixture(tc_ ## name, setup, teardown);
    tcase_add_loop_test(tc_handlebars_spec_parser, handlebars_spec_parser, 0, tests_len - 1);
    suite_add_tcase(s, tc_handlebars_spec_parser);
    
    return s;
}

int main(void)
{
    int number_failed;
    Suite * s;
    SRunner * sr;
    char * spec_filename;
    
    // Load the spec
    spec_filename = getenv("handlebars_parser_spec");
    loadSpec(spec_filename);
    fprintf(stderr, "Loaded %lu test cases\n", tests_len);
    
    s = parser_suite();
    sr = srunner_create(s);
#if defined(_WIN64) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN32__)
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    //runner_set_log(sr, "test_spec_handlebars_parser.log");
    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}