#ifndef REL_EQ_ASSOC_H
#define REL_EQ_ASSOC_H

#include "../auxi/auxiliary.h"
#include "../auxi/test.h"

/**
 * @brief Applies associativity transformation on relational algebra expressions.
 *        Transforms ((A⋈B)⋈C) into (A⋈(B⋈C)) if pattern is matched.
 * @param list_rel_eq Pointer to pointer to list of relational algebra expressions.
 * @return Pointer to a new list of transformed expressions.
 */
struct list_node* AK_rel_eq_assoc(struct list_node** list_rel_eq);

/**
 * @brief Unit test for associative transformation.
 *        Verifies transformation of ((A⋈B)⋈C) into (A⋈(B⋈C)).
 * @return TestResult indicating success or failure.
 */
TestResult AK_rel_eq_assoc_test(void);

/**
 * @brief Unit test for cases where no associative transformation is expected.
 * @return TestResult indicating success or failure.
 */
TestResult AK_rel_eq_assoc_nochange_test();

/**
 * @brief Pokreće sve testove asocijativnosti unutar jedne funkcije s ispisom rezultata.
 * @return TestResult s brojem uspješnih i neuspješnih testova.
 */
TestResult AK_rel_eq_assoc_all_tests(void);

#endif // REL_EQ_ASSOC_H
