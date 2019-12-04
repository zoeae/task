/*
 * test.c
 *
 */

#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "card_type.h"
#include "transaction_type.h"
#include "terminal.h"




/* card_type tests
 */
void test_card_type_is_valid(void) {
  CU_ASSERT(true == card_type_is_valid("Visa"));
  CU_ASSERT(true == card_type_is_valid("JBC"));
  CU_ASSERT(false == card_type_is_valid("abc"));
}

void test_card_type_find_by_name(void) {
  CU_ASSERT(NULL != card_type_find_by_name("Visa"));
  CU_ASSERT(NULL != card_type_find_by_name("JBC"));
  CU_ASSERT(NULL == card_type_find_by_name("abc"));
}

void test_card_type_find_by_id(void) {
  CU_ASSERT(NULL != card_type_find_by_id(1));
  CU_ASSERT(NULL != card_type_find_by_id(5));
  CU_ASSERT(NULL == card_type_find_by_id(8192));
}

/* transaction_type tests
 */
void test_transaction_type_is_valid(void) {
  CU_ASSERT(true == transaction_type_is_valid("Cheque"));
  CU_ASSERT(true == transaction_type_is_valid("Credit"));
  CU_ASSERT(false == transaction_type_is_valid("abc"));
}

void test_transaction_type_find_by_name(void) {
  CU_ASSERT(NULL != transaction_type_find_by_name("Cheque"));
  CU_ASSERT(NULL != transaction_type_find_by_name("Savings"));
  CU_ASSERT(NULL == transaction_type_find_by_name("abc"));
}

void test_transaction_type_find_by_id(void) {
  CU_ASSERT(NULL != transaction_type_find_by_id(91));
  CU_ASSERT(NULL != transaction_type_find_by_id(92));
  CU_ASSERT(NULL == transaction_type_find_by_id(8192));
}

/* terminal tests
 */
void test_terminal_init_data(void) {
  Terminal_Data t;
  t.id = 1; t.cards[0] = 2; t.trxs[0] = 91;
  terminal_init_data(&t);
  CU_ASSERT(0 == t.id);
  CU_ASSERT(0 == t.cards[0]);
  CU_ASSERT(0 == t.trxs[0]);
}

void test_terminal_find_by_id(void) {
  CU_ASSERT(NULL != terminal_find_by_id(1));
  CU_ASSERT(NULL == terminal_find_by_id(9876));
}

void test_terminal_is_valid(void) {
  Terminal_Data t;
  terminal_init_data(&t);
  t.id = 1; t.cards[0] = 1; t.cards[1] = 2; t.trxs[0] = 91;
  CU_ASSERT(true == terminal_is_valid(&t));
  terminal_init_data(&t);
  t.id = 0; t.cards[0] = 1; t.cards[1] = 2; t.trxs[0] = 91;
  CU_ASSERT(true == terminal_is_valid(&t));
  terminal_init_data(&t);
  t.id = 1; t.cards[0] = 8192; t.cards[1] = 2; t.trxs[0] = 91;
  CU_ASSERT(false == terminal_is_valid(&t));
  terminal_init_data(&t);
  t.id = 1; t.cards[0] = 1; t.cards[1] = 2; t.trxs[0] = 8192;
  CU_ASSERT(false == terminal_is_valid(&t));
}

void test_terminal_add(void) {
  Terminal_Data t;
  terminal_init_data(&t);
  t.id = 0; t.cards[0] = 1; t.cards[1] = 2; t.trxs[0] = 91;
  CU_ASSERT(true == terminal_add(&t));
  CU_ASSERT(1 == t.id);
  CU_ASSERT(NULL != terminal_find_by_id(t.id));
}

void test_terminal_to_json(void) {
  Terminal_Data t;
  char *actual;
  char *expected = "{\n\
 \"id\": 1,\n\
 \"CardType\": [\n\
  \"Visa\",\n\
  \"MasterCard\"\n\
 ],\n\
 \"TransactionType\": [\n\
  \"Credit\",\n\
  \"Savings\"\n\
 ]\n\
}";
  terminal_init_data(&t);
  t.id = 1;
  terminal_add_card_type(&t, "Visa");
  terminal_add_card_type(&t, "MasterCard");
  terminal_add_transaction_type(&t, "Credit");
  terminal_add_transaction_type(&t, "Savings");
  actual = terminal_to_json(&t);
  CU_ASSERT(NULL != actual);
  CU_ASSERT_STRING_EQUAL(actual, expected);
  free(actual);
}

void test_terminal_load_json(void) {
  Terminal_Data t;
  char *actual;
  char input[BUFSIZ];

  sprintf(input, "{\n\
 \"id\": 1,\n\
 \"CardType\": [\n\
  \"Visa\",\n\
  \"MasterCard\"\n\
 ],\n\
 \"TransactionType\": [\n\
  \"Credit\",\n\
  \"Savings\"\n\
 ]\n\
}");
  terminal_init_data(&t);
  CU_ASSERT(true == terminal_load_json(&t, input));

  sprintf(input, "{\n\
 \"id\": 1,\n\
 \"xxxCardType\": [\n\
  \"Visa\",\n\
  \"MasterCard\"\n\
 ],\n\
 \"TransactionType\": [\n\
  \"Credit\",\n\
  \"Savings\"\n\
 ]\n\
}");
  terminal_init_data(&t);
  CU_ASSERT(false == terminal_load_json(&t, input));

  sprintf(input, "{\n\
 \"id\": 1,\n\
 \"CardType\": [\n\
  \"Visa\",\n\
  \"MasterCard\"\n\
 ],\n\
 \"xxxTransactionType\": [\n\
  \"Credit\",\n\
  \"Savings\"\n\
 ]\n\
}");
  terminal_init_data(&t);
  CU_ASSERT(false == terminal_load_json(&t, input));

  sprintf(input, "{\n\
 \"id\": 1,\n\
 \"CardType\": [\n\
  \"xxxVisa\",\n\
  \"MasterCard\"\n\
 ],\n\
 \"TransactionType\": [\n\
  \"Credit\",\n\
  \"Savings\"\n\
 ]\n\
}");
  terminal_init_data(&t);
  CU_ASSERT(false == terminal_load_json(&t, input));

  sprintf(input, "{\n\
 \"id\": 1,\n\
 \"CardType\": [\n\
  \"Visa\",\n\
  \"MasterCard\"\n\
 ],\n\
 \"TransactionType\": [\n\
  \"xxxCredit\",\n\
  \"Savings\"\n\
 ]\n\
}");
  terminal_init_data(&t);
  CU_ASSERT(false == terminal_load_json(&t, input));

  sprintf(input, "{xxx]");
  terminal_init_data(&t);
  CU_ASSERT(false == terminal_load_json(&t, input));
}


/* tests */
int main() {
  CU_initialize_registry();
  CU_pSuite suite = CU_add_suite("rest server", 0, 0);

  /* card_type tests */
  CU_add_test(suite, "card_type_is_valid", test_card_type_is_valid);
  CU_add_test(suite, "card_type_find_by_name", test_card_type_find_by_name);
  CU_add_test(suite, "card_type_find_by_id", test_card_type_find_by_id);

  /* transaction_type tests */
  CU_add_test(suite, "transaction_type_is_valid", test_transaction_type_is_valid);
  CU_add_test(suite, "transaction_type_find_by_name", test_transaction_type_find_by_name);
  CU_add_test(suite, "transaction_type_find_by_id", test_transaction_type_find_by_id);

  /* terminal tests */
  CU_add_test(suite, "terminal_add", test_terminal_add);
  CU_add_test(suite, "terminal_init_data", test_terminal_init_data);
  CU_add_test(suite, "terminal_find_by_id", test_terminal_find_by_id);
  CU_add_test(suite, "terminal_is_valid", test_terminal_is_valid);
  CU_add_test(suite, "terminal_to_json", test_terminal_to_json);
  CU_add_test(suite, "terminal_load_json", test_terminal_load_json);

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return 0;
}
/* vim: set et sm ai ts=2: */
