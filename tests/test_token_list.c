
#include <check.h>
#include <talloc.h>

#include "handlebars.h"
#include "handlebars_memory.h"
#include "handlebars_token.h"
#include "handlebars_token_list.h"
#include "handlebars.tab.h"
#include "utils.h"

static TALLOC_CTX * ctx;

static void setup(void)
{
    handlebars_memory_fail_disable();
    ctx = talloc_new(NULL);
}

static void teardown(void)
{
    handlebars_memory_fail_disable();
    talloc_free(ctx);
    ctx = NULL;
}

START_TEST(test_token_list_append)
{
	struct handlebars_token_list * list = handlebars_token_list_ctor(ctx);
	struct handlebars_token * node1 = handlebars_talloc(list, struct handlebars_token);
	struct handlebars_token * node2 = handlebars_talloc(list, struct handlebars_token);
	handlebars_token_list_append(list, node1);
	handlebars_token_list_append(list, node2);
	
	ck_assert_ptr_eq(list->first->data, node1);
	ck_assert_ptr_eq(list->first->next->data, node2);
	ck_assert_ptr_eq(list->last->data, node2);
	
	handlebars_token_list_dtor(list);
}
END_TEST

START_TEST(test_token_list_append_failed_alloc)
{
	struct handlebars_token_list * list = handlebars_token_list_ctor(ctx);
	struct handlebars_token * node1 = handlebars_talloc(list, struct handlebars_token);
	int retval;
	
	handlebars_memory_fail_enable();
	retval = handlebars_token_list_append(list, node1);
	handlebars_memory_fail_disable();
	
	ck_assert_int_eq(retval, HANDLEBARS_NOMEM);
	ck_assert_ptr_eq(list->first, NULL);
	ck_assert_ptr_eq(list->last, NULL);
	
	handlebars_token_list_dtor(list);
}
END_TEST

START_TEST(test_token_list_append_null)
{
	handlebars_token_list_append(NULL, NULL);
}
END_TEST

START_TEST(test_token_list_ctor)
{
	struct handlebars_token_list * list = handlebars_token_list_ctor(ctx);
	
	ck_assert_ptr_ne(list, NULL);
	
	handlebars_token_list_dtor(list);
}
END_TEST

START_TEST(test_token_list_ctor_failed_alloc)
{
	struct handlebars_token_list * list;
	
	handlebars_memory_fail_enable();
	list = handlebars_token_list_ctor(ctx);
	handlebars_memory_fail_disable();
	
	ck_assert_ptr_eq(list, NULL);
}
END_TEST

START_TEST(test_token_list_prepend)
{
	struct handlebars_token_list * list = handlebars_token_list_ctor(ctx);
	struct handlebars_token * node1 = handlebars_talloc(list, struct handlebars_token);
	struct handlebars_token * node2 = handlebars_talloc(list, struct handlebars_token);
	handlebars_token_list_prepend(list, node1);
	handlebars_token_list_prepend(list, node2);
	
	ck_assert_ptr_eq(list->first->data, node2);
	ck_assert_ptr_eq(list->first->next->data, node1);
	ck_assert_ptr_eq(list->last->data, node1);
	
	handlebars_token_list_dtor(list);
}
END_TEST

START_TEST(test_token_list_prepend_failed_alloc)
{
	struct handlebars_token_list * list = handlebars_token_list_ctor(ctx);
	struct handlebars_token * node1 = handlebars_talloc(list, struct handlebars_token);
	int retval;
	
	handlebars_memory_fail_enable();
	retval = handlebars_token_list_prepend(list, node1);
	handlebars_memory_fail_disable();
	
	ck_assert_int_eq(retval, HANDLEBARS_NOMEM);
	ck_assert_ptr_eq(list->first, NULL);
	ck_assert_ptr_eq(list->last, NULL);
	
	handlebars_token_list_dtor(list);
}
END_TEST

START_TEST(test_token_list_prepend_null)
{
	handlebars_token_list_prepend(NULL, NULL);
}
END_TEST

Suite * parser_suite(void)
{
	Suite * s = suite_create("Token List");
	
	REGISTER_TEST_FIXTURE(s, test_token_list_append, "Append");
	REGISTER_TEST_FIXTURE(s, test_token_list_append_failed_alloc, "Append with failed alloc");
	REGISTER_TEST_FIXTURE(s, test_token_list_append_null, "Append with null argument");
	REGISTER_TEST_FIXTURE(s, test_token_list_ctor, "Constructor");
	REGISTER_TEST_FIXTURE(s, test_token_list_ctor_failed_alloc, "Constructor with failed alloc");
	REGISTER_TEST_FIXTURE(s, test_token_list_prepend, "Prepend");
	REGISTER_TEST_FIXTURE(s, test_token_list_prepend_failed_alloc, "Prepend with failed alloc");
	REGISTER_TEST_FIXTURE(s, test_token_list_prepend_null, "Prepend with null argument");
	
	return s;
}

int main(void)
{
    int number_failed;
    int memdebug;
    int error;
    
    // Check if memdebug enabled
    memdebug = getenv("MEMDEBUG") ? atoi(getenv("MEMDEBUG")) : 0;
    if( memdebug ) {
        talloc_enable_leak_report_full();
    }
    
    // Set up test suite
    Suite * s = parser_suite();
    SRunner * sr = srunner_create(s);
    if( IS_WIN || memdebug ) {
        srunner_set_fork_status(sr, CK_NOFORK);
    }
    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    error = (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    
    // Generate report for memdebug
    if( memdebug ) {
        talloc_report_full(NULL, stderr);
    }
    
    // Return
    return error;
}
