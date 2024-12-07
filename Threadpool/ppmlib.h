/***************************************************************************//**
 * @file ppmlib.h
 * @author Dorian Weber
 * @brief Contains a small macro library used for preprocessor programming.
 * 
 * @details
 * Preprocessor metaprogramming is very valuable in larger C projects. It's
 * efficient and can manipulate code at a very basic level. This small library
 * is intended to facilitate macro programming by collecting commonly used
 * macro functions. It supports argument lists with a size of up to 32.
 ******************************************************************************/

#ifndef PPMLIB_H_INCLUDED
#define PPMLIB_H_INCLUDED

#include <stddef.h>

/**@brief Analyses the number of arguments passed to a variadic macro.
 * 
 * This includes the possibility that the first and only argument is empty,
 * which returns 0.
 * 
 * @param ...  variadic list of arguments
 * @sa NUM_PARGS
 */
#define NUM_ARGS(...)  \
	IF_EMPTY(__VA_ARGS__)(0)(NUM_PARGS(__VA_ARGS__))

/**@brief Analyses the number of arguments passed to a variadic macro.
 * 
 * The only difference between this macro and NUM_ARGS is the result if the
 * first and only argument is empty. This macro will return 1 in this case,
 * since the empty string is a valid preprocessor argument. If you can ensure
 * or don't care if the first argument is empty, this macro is faster to
 * compute than the NUM_ARGS equivalent.
 * 
 * @param ...  variadic list of arguments
 * @sa NUM_ARGS
 */
#define NUM_PARGS(...) \
	NUM_ARGS_(__VA_ARGS__,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17, \
	          16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1,~)

/**@brief Returns if the variadic list of arguments contains a comma. 
 * 
 * A variadic list contains a comma iff it contains at least two arguments.
 * 
 * @param ...  variadic list of arguments
 * @return 1 if the list contains a comma, \n
 *         0 otherwise
 */
#define HAS_COMMA(...) \
	NUM_ARGS_(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
	           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,~)

/**@brief Applies a macro transformation on each of the macro arguments.
 * @note Using this macro directly is kind of complicated. If you just want to
 * apply a simple macro to each of the arguments, using TRAFO is easier.
 * 
 * @param OUTER  unary macro function that is applied to each argument, except
 *               for the last one, and after the INNER macro has been applied
 * @param INNER  unary macro function that is applied to each argument,
 *               including the last one, and before the OUTER macro is applied
 * @param LIST   the macro arguments, wrapped into parenthesis
 * @sa MAP, FOREACHP
 */
#define FOREACH(OUTER, INNER, LIST) \
	FOREACH_(OUTER, NUM_ARGS LIST, INNER, LIST)

/**@brief Applies a macro transformation on each of the macro arguments and
 * takes an additional static argument.
 * 
 * It works exactly like the FOREACH-macro but allows passing an addition data
 * argument.
 * @sa FOLD, FOREACH
 */
#define FOREACHP(OUTER, INNER, PARAM, LIST) \
	FOREACHP_(OUTER, NUM_ARGS LIST, INNER, PARAM, LIST)

/**@brief Returns whether the variadic list of macro arguments only contains
 * the empty parameter or not.
 * 
 * @param ...  variadic argument list
 * @return 1 if there is only one argument and it is empty,\n
 *         0 otherwise
 */
#define IS_EMPTY(...) _IS_EMPTY(                                        \
	/* test if there is just one argument, possibly an empty one */     \
	HAS_COMMA(__VA_ARGS__),                                             \
	/* test if _IS_EMPTY_FN together with the argument adds a comma */  \
	HAS_COMMA(_IS_EMPTY_FN __VA_ARGS__),                                \
	/* test if the argument together with a parenthesis adds a comma */ \
	HAS_COMMA(__VA_ARGS__ (/*empty*/)),                                 \
	/* test if placing it between _IS_EMPTY_FN and the parenthesis      \
	 * adds a comma */                                                  \
	HAS_COMMA(_IS_EMPTY_FN __VA_ARGS__ (/*empty*/))                     \
)

/**@name Expansion specifier
 * Contains unary macros that perform common transformations on a macro
 * argument.
 * @{
 */

/**@brief Simply expands the macro arguments.
 * 
 * Use it to unpack argument lists like:
 * @code
 * 	#define ARGS (1, 2, 3, 4, 5)
 * 	CALL_VAR_FN(EXPAND ARGS); // expands to CALL_VAR_FN(1, 2, 3, 4, 5)
 * @endcode
 */
#define EXPAND(...) \
	__VA_ARGS__

/**@brief Prevents a function-like macro from expanding immediately.
 * 
 * Use it to prevent macros from expanding immediately:
 * @code
 * 	#define FOO() foo
 * 	DEFER(FOO)()         // DEFER(FOO)() -> FOO()
 * 	EXPAND(DEFER(FOO)()) // EXPAND(DEFER(FOO)()) -> EXPAND(FOO()) -> foo
 * @endcode
 */
#define DEFER(...) \
	__VA_ARGS__ FORGET()

/**@brief Expands the macro arguments and adds a comma to them.
 */
#define EXPAND_W_COMMA(...) \
	__VA_ARGS__,

/**@brief Expands two macro arguments as a pair.
 */
#define EXPAND_AS_PAIR(E1, E2) \
	(E1, E2)

/**@brief Replaces the macro arguments with the empty argument.
 */
#define FORGET(...)

/**@brief Stringifies the arguments.
 */
#define STR(...) \
	STR_(__VA_ARGS__)

/**@brief Generates a unique name using the current line in the code.
 * This is useful for variable names in macro-expansions.
 */
#define UID(VAR) \
	CONCAT(VAR, _, __LINE__)

/**@}*/

/**@brief Applies the unary macro to each element of a variadic argument list.
 * 
 * This is the less powerful version of the FOREACH macro. It simply applies a
 * unary macro to each of its arguments.
 * 
 * @param M    the macro function to apply
 * @param ...  variadic argument list 
 * @sa FOLD, FOREACH
 */
#define MAP(M, ...) \
	FOREACH(EXPAND, M, (__VA_ARGS__))

/**@brief Applies a binary macro for each element of a variadic argument list.
 * 
 * The macro receives both a static argument as well as the current element.
 * The static argument is given as the \p P parameter during invokation.
 * @sa MAP, FOREACHP
 */
#define FOLD(M,P, ...) \
	FOREACHP(EXPAND, M, P, (__VA_ARGS__))

/**@brief Splices together two variadic sequences into a sequence of pairs.
 * 
 * The number of elements in the combined sequence is always equal to the
 * number of elements in the first sequence. If the first sequence contains
 * more elements, the final sequence's tail has empty second arguments.
 * @sa ZIP_MAP
*/
#define ZIP(SEQ_A, SEQ_B) \
	ZIP_MAP(EXPAND_AS_PAIR, SEQ_A, SEQ_B)

/**@brief Splices together two variadic sequences using a binary macro.
 * 
 * Like the ZIP-macro, the number of combined elements is always equal to the
 * number of elements in the first sequence. This operator is slightly more
 * generic since it allows arbitrary operations on every pair of elements
 * through the binary macro.
 * @sa ZIP
 */
#define ZIP_MAP(M, SEQ_A, SEQ_B) \
	CONCAT(ZIP_, NUM_ARGS SEQ_A)(M, SEQ_A, SEQ_B)

/**@brief Selects an argument from a variadic list.
 * 
 * This macro function can be used to index into a variadic argument list. It
 * is possible to select a higher index than the list is long. In this case the
 * result is the empty argument.
 * 
 * @param N    index of the argument to select
 * @param ...  variadic argument list
 */
#define ARG(N, ...) \
	CONCAT_2(ARG_,N)(__VA_ARGS__,)

/**@brief Selects and returns the first argument from a variadic list.
 */
#define ARG_FIRST(...) \
	ARG_1_(__VA_ARGS__,)

/**@brief Selects and returns the last argument from a variadic list.
 */
#define ARG_LAST(...) \
	ARG(NUM_PARGS(__VA_ARGS__),__VA_ARGS__)

/**@brief Selects the remainder of an argument list after removing the first.
 */
#define ARG_REST(...) \
	IF_COMMA(__VA_ARGS__)(ARG_REST_(__VA_ARGS__))()

/**@brief Selects every argument except for the last of a variadic list.
 */
#define ARG_CHOP(...) \
	FOREACH_(EXPAND_W_COMMA, NUM_ARGS(ARG_REST(__VA_ARGS__)), EXPAND, (__VA_ARGS__))

/**@brief Concatenates a variable number of arguments.
 * 
 * Since it is not specified by the C standard: concatenation occurs from left
 * to right with left associativity. To give an example `CONCAT(a,b,c)` would
 * be treated as `(a ## b) ## c`.
 * 
 * @param ...  variadic list of valid preprocessor tokens
 */
#define CONCAT(...) \
	CONCAT_(NUM_PARGS(__VA_ARGS__), __VA_ARGS__)

/**@name Inline conditionals
 * Macros that support the conditional generation of text.
 * @{
 */

/**@brief Tests if two numbers are equal and generates code according to the
 * result.
 * 
 * The numbers are expanded first. The syntax for this tool is like
 * @code
 * IF_EQ(a,b)(<true>)(<false>)
 * @endcode
 * The code in the first branch is generated if `a` equals `b`, the code in the
 * second branch otherwise.
 * 
 * @param A  first number
 * @param B  second number
 */
#define IF_EQ(A, B) \
	_IF_CLAUSE(CONCAT_4(_IS_,A,_EQ_,B)())

/**@brief Tests if the number of arguments is equal to a number and generates
 * code according to the result.
 * 
 * The numbers are expanded first. The syntax for this tool is like
 * @code
 * IF_LEN(a,__VA_ARGS__)(<true>)(<false>)
 * @endcode
 * The code in the first branch is generated if `a` equals the length of
 * `__VA_ARGS__`, the code in the second branch otherwise.
 * 
 * @param A    first number
 * @param ...  argument list
 */
#define IF_LEN(A, ...) \
	_IF_CLAUSE(CONCAT_4(_IS_,A,_EQ_,NUM_ARGS(__VA_ARGS__)()))

/**@brief Tests if the argument list passed is empty and generates code
 * according to the result.
 * 
 * The argument list is empty iff it only contains one argument that is the
 * empty token. Use this function like this:
 * @code
 * IF_EMPTY(<arglist>)(<empty>)(<not empty>)
 * @endcode
 * The branch labeled with `<empty>` gets generated if the passed `<arglist>`
 * is indeed empty, the other branch if it isn't.
 * 
 * @param ...  variadic argument list
 * @sa IS_EMPTY
 */
#define IF_EMPTY(...) \
	IF_EQ(1, IS_EMPTY(__VA_ARGS__))

/**@brief Tests if the argument list contains a comma and generates code
 * according to the result.
 * 
 * If the argument list contains a comma, it must contain at least two
 * arguments. Use this tool like:
 * @code
 * IF_COMMA(<arglist>)(<comma>)(<no comma>)
 * @endcode
 * The result is the branch labeled with `<comma>` if the `<arglist>`
 * contains at least two arguments, otherwise the contents of the other branch
 * are produced.
 * 
 * @param ...  variadic argument list
 * @sa HAS_COMMA
 */
#define IF_COMMA(...) \
	IF_EQ(1, HAS_COMMA(__VA_ARGS__))

/**@}*/

/**@cond Internal */
#define NUM_ARGS_(_20,_1F,_1E,_1D,_1C,_1B,_1A,_19,_18,_17,_16,_15,_14,_13,_12,_11, \
                  _10, _F, _E, _D, _C, _B, _A, _9, _8, _7, _6, _5, _4, _3, _2, _1, N,...) N

#define FOREACH_(E,N,M, LIST)     FOREACH__(E,N,M,LIST)
#define FOREACH__(E,N,M, LIST)    FOREACH_N##N(E,M,LIST)

#define FOREACHP_(E,N,M,P, LIST)  FOREACHP__(E,N,M,P,LIST)
#define FOREACHP__(E,N,M,P, LIST) FOREACH_P##N(E,M,P,LIST)

#define FOREACH_P0(E,M,P, LIST)  /* empty */
#define FOREACH_P1(E,M,P, LIST)  M(P,ARG_1 LIST)
#define FOREACH_P2(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P1(E,M,P, (ARG_REST LIST))
#define FOREACH_P3(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P2(E,M,P, (ARG_REST LIST))
#define FOREACH_P4(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P3(E,M,P, (ARG_REST LIST))
#define FOREACH_P5(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P4(E,M,P, (ARG_REST LIST))
#define FOREACH_P6(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P5(E,M,P, (ARG_REST LIST))
#define FOREACH_P7(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P6(E,M,P, (ARG_REST LIST))
#define FOREACH_P8(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P7(E,M,P, (ARG_REST LIST))
#define FOREACH_P9(E,M,P, LIST)  E(M(P,ARG_1 LIST)) FOREACH_P8(E,M,P, (ARG_REST LIST))
#define FOREACH_P10(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P9(E,M,P, (ARG_REST LIST))
#define FOREACH_P11(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P10(E,M,P, (ARG_REST LIST))
#define FOREACH_P12(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P11(E,M,P, (ARG_REST LIST))
#define FOREACH_P13(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P12(E,M,P, (ARG_REST LIST))
#define FOREACH_P14(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P13(E,M,P, (ARG_REST LIST))
#define FOREACH_P15(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P14(E,M,P, (ARG_REST LIST))
#define FOREACH_P16(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P15(E,M,P, (ARG_REST LIST))
#define FOREACH_P17(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P16(E,M,P, (ARG_REST LIST))
#define FOREACH_P18(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P17(E,M,P, (ARG_REST LIST))
#define FOREACH_P19(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P18(E,M,P, (ARG_REST LIST))
#define FOREACH_P20(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P19(E,M,P, (ARG_REST LIST))
#define FOREACH_P21(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P20(E,M,P, (ARG_REST LIST))
#define FOREACH_P22(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P21(E,M,P, (ARG_REST LIST))
#define FOREACH_P23(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P22(E,M,P, (ARG_REST LIST))
#define FOREACH_P24(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P23(E,M,P, (ARG_REST LIST))
#define FOREACH_P25(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P24(E,M,P, (ARG_REST LIST))
#define FOREACH_P26(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P25(E,M,P, (ARG_REST LIST))
#define FOREACH_P27(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P26(E,M,P, (ARG_REST LIST))
#define FOREACH_P28(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P27(E,M,P, (ARG_REST LIST))
#define FOREACH_P29(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P28(E,M,P, (ARG_REST LIST))
#define FOREACH_P30(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P29(E,M,P, (ARG_REST LIST))
#define FOREACH_P31(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P30(E,M,P, (ARG_REST LIST))
#define FOREACH_P32(E,M,P, LIST) E(M(P,ARG_1 LIST)) FOREACH_P31(E,M,P, (ARG_REST LIST))

#define FOREACH_N0(E,M, LIST)  /* empty */
#define FOREACH_N1(E,M, LIST)  M(ARG_1 LIST)
#define FOREACH_N2(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N1(E,M, (ARG_REST LIST))
#define FOREACH_N3(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N2(E,M, (ARG_REST LIST))
#define FOREACH_N4(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N3(E,M, (ARG_REST LIST))
#define FOREACH_N5(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N4(E,M, (ARG_REST LIST))
#define FOREACH_N6(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N5(E,M, (ARG_REST LIST))
#define FOREACH_N7(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N6(E,M, (ARG_REST LIST))
#define FOREACH_N8(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N7(E,M, (ARG_REST LIST))
#define FOREACH_N9(E,M, LIST)  E(M(ARG_1 LIST)) FOREACH_N8(E,M, (ARG_REST LIST))
#define FOREACH_N10(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N9(E,M, (ARG_REST LIST))
#define FOREACH_N11(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N10(E,M, (ARG_REST LIST))
#define FOREACH_N12(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N11(E,M, (ARG_REST LIST))
#define FOREACH_N13(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N12(E,M, (ARG_REST LIST))
#define FOREACH_N14(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N13(E,M, (ARG_REST LIST))
#define FOREACH_N15(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N14(E,M, (ARG_REST LIST))
#define FOREACH_N16(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N15(E,M, (ARG_REST LIST))
#define FOREACH_N17(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N16(E,M, (ARG_REST LIST))
#define FOREACH_N18(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N17(E,M, (ARG_REST LIST))
#define FOREACH_N19(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N18(E,M, (ARG_REST LIST))
#define FOREACH_N20(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N19(E,M, (ARG_REST LIST))
#define FOREACH_N21(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N20(E,M, (ARG_REST LIST))
#define FOREACH_N22(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N21(E,M, (ARG_REST LIST))
#define FOREACH_N23(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N22(E,M, (ARG_REST LIST))
#define FOREACH_N24(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N23(E,M, (ARG_REST LIST))
#define FOREACH_N25(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N24(E,M, (ARG_REST LIST))
#define FOREACH_N26(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N25(E,M, (ARG_REST LIST))
#define FOREACH_N27(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N26(E,M, (ARG_REST LIST))
#define FOREACH_N28(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N27(E,M, (ARG_REST LIST))
#define FOREACH_N29(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N28(E,M, (ARG_REST LIST))
#define FOREACH_N30(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N29(E,M, (ARG_REST LIST))
#define FOREACH_N31(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N30(E,M, (ARG_REST LIST))
#define FOREACH_N32(E,M, LIST) E(M(ARG_1 LIST)) FOREACH_N31(E,M, (ARG_REST LIST))

#define ZIP_0(M, SEQ_A, SEQ_B) /* empty */
#define ZIP_1(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B)
#define ZIP_2(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_1(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_3(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_2(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_4(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_3(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_5(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_4(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_6(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_5(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_7(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_6(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_8(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_7(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_9(M, SEQ_A, SEQ_B)  M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_8(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_10(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_9(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_11(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_10(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_12(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_11(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_13(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_12(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_14(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_13(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_15(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_14(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_16(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_15(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_17(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_16(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_18(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_17(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_19(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_18(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_20(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_19(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_21(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_20(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_22(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_21(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_23(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_22(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_24(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_23(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_25(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_24(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_26(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_25(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_27(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_26(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_28(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_27(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_29(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_28(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_30(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_29(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_31(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_30(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))
#define ZIP_32(M, SEQ_A, SEQ_B) M(ARG_1 SEQ_A, ARG_1 SEQ_B), ZIP_31(M, (ARG_REST SEQ_A), (ARG_REST SEQ_B))

#define CONCAT_(C, ...)   CONCAT__(C,__VA_ARGS__)
#define CONCAT__(C, ...)  CONCAT_ ## C(__VA_ARGS__)

#define CONCAT_1(_1)           _1
#define CONCAT_2(_1,_2)        _1 ## _2
#define CONCAT_3(_1,_2,...)    CONCAT_2(_1 ## _2, __VA_ARGS__)
#define CONCAT_4(_1,_2,...)    CONCAT_3(_1 ## _2, __VA_ARGS__)
#define CONCAT_5(_1,_2,...)    CONCAT_4(_1 ## _2, __VA_ARGS__)
#define CONCAT_6(_1,_2,...)    CONCAT_5(_1 ## _2, __VA_ARGS__)
#define CONCAT_7(_1,_2,...)    CONCAT_6(_1 ## _2, __VA_ARGS__)
#define CONCAT_8(_1,_2,...)    CONCAT_7(_1 ## _2, __VA_ARGS__)
#define CONCAT_9(_1,_2,...)    CONCAT_8(_1 ## _2, __VA_ARGS__)
#define CONCAT_10(_1,_2,...)   CONCAT_9(_1 ## _2, __VA_ARGS__)
#define CONCAT_11(_1,_2,...)   CONCAT_10(_1 ## _2, __VA_ARGS__)
#define CONCAT_12(_1,_2,...)   CONCAT_11(_1 ## _2, __VA_ARGS__)
#define CONCAT_13(_1,_2,...)   CONCAT_12(_1 ## _2, __VA_ARGS__)
#define CONCAT_14(_1,_2,...)   CONCAT_13(_1 ## _2, __VA_ARGS__)
#define CONCAT_15(_1,_2,...)   CONCAT_14(_1 ## _2, __VA_ARGS__)
#define CONCAT_16(_1,_2,...)   CONCAT_15(_1 ## _2, __VA_ARGS__)
#define CONCAT_17(_1,_2,...)   CONCAT_16(_1 ## _2, __VA_ARGS__)
#define CONCAT_18(_1,_2,...)   CONCAT_17(_1 ## _2, __VA_ARGS__)
#define CONCAT_19(_1,_2,...)   CONCAT_18(_1 ## _2, __VA_ARGS__)
#define CONCAT_20(_1,_2,...)   CONCAT_19(_1 ## _2, __VA_ARGS__)
#define CONCAT_21(_1,_2,...)   CONCAT_20(_1 ## _2, __VA_ARGS__)
#define CONCAT_22(_1,_2,...)   CONCAT_21(_1 ## _2, __VA_ARGS__)
#define CONCAT_23(_1,_2,...)   CONCAT_22(_1 ## _2, __VA_ARGS__)
#define CONCAT_24(_1,_2,...)   CONCAT_23(_1 ## _2, __VA_ARGS__)
#define CONCAT_25(_1,_2,...)   CONCAT_24(_1 ## _2, __VA_ARGS__)
#define CONCAT_26(_1,_2,...)   CONCAT_25(_1 ## _2, __VA_ARGS__)
#define CONCAT_27(_1,_2,...)   CONCAT_26(_1 ## _2, __VA_ARGS__)
#define CONCAT_28(_1,_2,...)   CONCAT_27(_1 ## _2, __VA_ARGS__)
#define CONCAT_29(_1,_2,...)   CONCAT_28(_1 ## _2, __VA_ARGS__)
#define CONCAT_30(_1,_2,...)   CONCAT_29(_1 ## _2, __VA_ARGS__)
#define CONCAT_31(_1,_2,...)   CONCAT_30(_1 ## _2, __VA_ARGS__)
#define CONCAT_32(_1,_2,...)   CONCAT_31(_1 ## _2, __VA_ARGS__)

#define ARG_REST_(_1, ...) \
	__VA_ARGS__

#define ARG_(...)            /* empty */
#define ARG_0(...)           /* empty */
#define ARG_1_(_1, ...)      _1
#define ARG_1(...)           ARG_1_(__VA_ARGS__,)
#define ARG_2(...)           ARG_1(ARG_REST_(__VA_ARGS__,))
#define ARG_3(...)           ARG_2(ARG_REST_(__VA_ARGS__,))
#define ARG_4(...)           ARG_3(ARG_REST_(__VA_ARGS__,))
#define ARG_5(...)           ARG_4(ARG_REST_(__VA_ARGS__,))
#define ARG_6(...)           ARG_5(ARG_REST_(__VA_ARGS__,))
#define ARG_7(...)           ARG_6(ARG_REST_(__VA_ARGS__,))
#define ARG_8(...)           ARG_7(ARG_REST_(__VA_ARGS__,))
#define ARG_9(...)           ARG_8(ARG_REST_(__VA_ARGS__,))
#define ARG_10(...)          ARG_9(ARG_REST_(__VA_ARGS__,))
#define ARG_11(...)          ARG_10(ARG_REST_(__VA_ARGS__,))
#define ARG_12(...)          ARG_11(ARG_REST_(__VA_ARGS__,))
#define ARG_13(...)          ARG_12(ARG_REST_(__VA_ARGS__,))
#define ARG_14(...)          ARG_13(ARG_REST_(__VA_ARGS__,))
#define ARG_15(...)          ARG_14(ARG_REST_(__VA_ARGS__,))
#define ARG_16(...)          ARG_15(ARG_REST_(__VA_ARGS__,))
#define ARG_17(...)          ARG_16(ARG_REST_(__VA_ARGS__,))
#define ARG_18(...)          ARG_17(ARG_REST_(__VA_ARGS__,))
#define ARG_19(...)          ARG_18(ARG_REST_(__VA_ARGS__,))
#define ARG_20(...)          ARG_19(ARG_REST_(__VA_ARGS__,))
#define ARG_21(...)          ARG_20(ARG_REST_(__VA_ARGS__,))
#define ARG_22(...)          ARG_21(ARG_REST_(__VA_ARGS__,))
#define ARG_23(...)          ARG_22(ARG_REST_(__VA_ARGS__,))
#define ARG_24(...)          ARG_23(ARG_REST_(__VA_ARGS__,))
#define ARG_25(...)          ARG_24(ARG_REST_(__VA_ARGS__,))
#define ARG_26(...)          ARG_25(ARG_REST_(__VA_ARGS__,))
#define ARG_27(...)          ARG_26(ARG_REST_(__VA_ARGS__,))
#define ARG_28(...)          ARG_27(ARG_REST_(__VA_ARGS__,))
#define ARG_29(...)          ARG_28(ARG_REST_(__VA_ARGS__,))
#define ARG_30(...)          ARG_29(ARG_REST_(__VA_ARGS__,))
#define ARG_31(...)          ARG_30(ARG_REST_(__VA_ARGS__,))
#define ARG_32(...)          ARG_31(ARG_REST_(__VA_ARGS__,))

#define _CLAUSE1(...) __VA_ARGS__ FORGET
#define _CLAUSE2(...) EXPAND

#define __IF_CLAUSE(A,B,C,...) C
#define _IF_CLAUSE(EXP) __IF_CLAUSE(EXP, _CLAUSE1, _CLAUSE2,)

#define _IS_0_EQ_0    ,
#define _IS_1_EQ_1    ,
#define _IS_2_EQ_2    ,
#define _IS_3_EQ_3    ,
#define _IS_4_EQ_4    ,
#define _IS_5_EQ_5    ,
#define _IS_6_EQ_6    ,
#define _IS_7_EQ_7    ,
#define _IS_8_EQ_8    ,
#define _IS_9_EQ_9    ,
#define _IS_10_EQ_10  ,
#define _IS_11_EQ_11  ,
#define _IS_12_EQ_12  ,
#define _IS_13_EQ_13  ,
#define _IS_14_EQ_14  ,
#define _IS_15_EQ_15  ,
#define _IS_16_EQ_16  ,
#define _IS_17_EQ_17  ,
#define _IS_18_EQ_18  ,
#define _IS_19_EQ_19  ,
#define _IS_20_EQ_20  ,
#define _IS_21_EQ_21  ,
#define _IS_22_EQ_22  ,
#define _IS_23_EQ_23  ,
#define _IS_24_EQ_24  ,
#define _IS_25_EQ_25  ,
#define _IS_26_EQ_26  ,
#define _IS_27_EQ_27  ,
#define _IS_28_EQ_28  ,
#define _IS_29_EQ_29  ,
#define _IS_30_EQ_30  ,
#define _IS_31_EQ_31  ,
#define _IS_32_EQ_32  ,

#define STR_(...)   #__VA_ARGS__

/* detect empty arguments */
#define _IS_EMPTY(_0, _1, _2, _3) \
	HAS_COMMA(CONCAT_5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001 ,
#define _IS_EMPTY_FN(...) ,

/**@endcond*/

#endif	/* PPMLIB_H_INCLUDED */
