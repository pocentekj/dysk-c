#include "src/deferred.h"
#include "src/dysk_c.h"
#include "src/strutil.h"

#include <check.h>
#include <stdlib.h>

// clang-format off

START_TEST(test_starts_with)
{
    ck_assert(starts_with("/dev/mapper/root", "/dev"));
    ck_assert(!starts_with("/dev/mapper/root", "dev"));
}
END_TEST

START_TEST(test_parse_line)
{
    deferred(free_string) string test_line = strdup("/dev/mapper/root / ext4 rw,relatime 0 0");
    deferred(string_array_destroy) string_array *tokens = parse_line(test_line);
    ck_assert(tokens);
    ck_assert(string_array_len(tokens) == 6);
}
END_TEST

START_TEST(test_parse_line_invalid)
{
    deferred(free_string) string test_line = strdup("This line is invalid");
    deferred(string_array_destroy) string_array *tokens = parse_line(test_line);
    ck_assert(tokens == NULL);
}
END_TEST

START_TEST(test_entry_from_line)
{
    deferred(free_string) string line = strdup("/dev/mapper/root / ext4 rw,relatime 0 0");
    parse_result r = Entry_from_line(line);
    ck_assert(r.ok);
    ck_assert(r.status == PARSE_SUCCESS);
    ck_assert_str_eq(r.entry->dev, "/dev/mapper/root");
    Entry_destroy(&(r.entry));

    deferred(free_string) string invalid_line = strdup("This is invalid");
    r = Entry_from_line(invalid_line);
    ck_assert(!r.ok);
    ck_assert(r.status == PARSE_ERROR);
    ck_assert(r.entry == NULL);
}
END_TEST

// clang-format on

Suite *dysk_c_suite(void) {
    Suite *s = suite_create("Dysk C");
    TCase *tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_starts_with);
    tcase_add_test(tc_core, test_parse_line);
    tcase_add_test(tc_core, test_parse_line_invalid);
    tcase_add_test(tc_core, test_entry_from_line);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    Suite *s = dysk_c_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
