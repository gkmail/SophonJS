/******************************************************************************
 *                SophonJS: An embedded javascript engine                     *
 *                                                                            *
 * Copyright (c) 2015 Gong Ke                                                 *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 * 1.Redistributions of source code must retain the above copyright notice,   *
 * this list of conditions and the following disclaimer.                      *
 * 2.Redistributions in binary form must reproduce the above copyright        *
 * notice, this list of conditions and the following disclaimer in the        *
 * documentation and/or other materials provided with the distribution.       *
 * 3.Neither the name of the Gong Ke; nor the names of its contributors may   *
 * be used to endorse or promote products derived from this software without  *
 * specific prior written permission.                                         *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR           *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *****************************************************************************/

add_rule(N_JS, N_PROGRAM, R_ACCEPT);

add_rule(N_PROGRAM, R_EMPTY_OPS);
add_rule(N_PROGRAM, N_STATEMENT, R_COPY_0);
add_rule(N_PROGRAM, N_PROGRAM, N_STATEMENT, R_MERGE_OPS);
add_rule(N_PROGRAM, T_PARSER_NONTERM_ERR);
add_rule(N_PROGRAM, N_PROGRAM, T_PARSER_NONTERM_ERR);

add_rule(N_STATEMENT_LIST, N_STATEMENT, R_COPY_0);
add_rule(N_STATEMENT_LIST, N_STATEMENT_LIST, N_STATEMENT, R_MERGE_OPS);

add_rule(N_BLOCK, '{', '}', R_EMPTY_OPS);
add_rule(N_BLOCK, '{', N_STATEMENT_LIST, '}', R_COPY_1);

add_rule(N_STATEMENT, ';', R_EMPTY_STATEMENT);
add_rule(N_STATEMENT, N_BLOCK, R_COPY_0);
add_rule(N_STATEMENT, T_continue, ';', R_CONTINUE);
add_rule(N_STATEMENT, T_continue, T_IDENTIFIER, ';', R_CONTINUE_LABEL);
add_rule(N_STATEMENT, T_break, ';', R_BREAK);
add_rule(N_STATEMENT, T_break, T_IDENTIFIER, ';', R_BREAK_LABEL);
add_rule(N_STATEMENT, T_return, ';', R_RETURN);
add_rule(N_STATEMENT, T_return, N_EXPR, ';', R_RETURN_VALUE);
add_rule(N_STATEMENT, T_var, N_VAR_DECL_LIST, ';', R_VAR_STATEMENT);
add_rule(N_STATEMENT, N_EXPR_STATEMENT, R_COPY_0);
add_rule(N_STATEMENT, N_IF_STATEMENT, R_COPY_0);
add_rule(N_STATEMENT, T_do, N_STATEMENT, T_while, '(', N_EXPR, ')', ';', R_DO_STATEMENT);
add_rule(N_STATEMENT, T_while, '(', N_EXPR, ')', N_STATEMENT, R_WHILE_STATEMENT);
add_rule(N_STATEMENT, N_FOR_STATEMENT, R_COPY_0);
add_rule(N_STATEMENT, T_with, '(', N_EXPR, ')', N_STATEMENT, R_WITH);
add_rule(N_STATEMENT, T_switch, '(', N_EXPR, ')', N_CASE_BLOCK, R_SWITCH);
add_rule(N_STATEMENT, T_IDENTIFIER, ':', N_STATEMENT, R_LABEL);
add_rule(N_STATEMENT, T_throw, N_EXPR, ';', R_THROW);
add_rule(N_STATEMENT, N_TRY_STATEMENT, R_COPY_0);
add_rule(N_STATEMENT, T_debugger, R_DEBUGGER);
add_rule(N_STATEMENT, N_FUNC_DECL, R_COPY_0);

add_rule(N_EXPR_STATEMENT, N_EXPR, ';', R_EXPR_STATEMENT);

add_rule(N_VAR_DECL_LIST, N_VAR_DECL, R_COPY_0);
add_rule(N_VAR_DECL_LIST, N_VAR_DECL_LIST, ',', N_VAR_DECL, R_MERGE_OPS_2);

add_rule(N_VAR_DECL, T_IDENTIFIER, R_VAR);
add_rule(N_VAR_DECL, T_IDENTIFIER, '=', N_ASSIGN_EXPR, R_VAR_INIT);

add_rule(N_VAR_DECL_LIST_NO_IN, N_VAR_DECL_NO_IN, R_COPY_0);
add_rule(N_VAR_DECL_LIST_NO_IN, N_VAR_DECL_LIST_NO_IN, ',', N_VAR_DECL_NO_IN, R_MERGE_OPS_2);

add_rule(N_VAR_DECL_NO_IN, T_IDENTIFIER, R_VAR);
add_rule(N_VAR_DECL_NO_IN, T_IDENTIFIER, '=', N_ASSIGN_EXPR_NO_IN, R_VAR_INIT);

add_rule(N_IF_STATEMENT, T_if, '(', N_EXPR, ')', N_STATEMENT, R_IF_STATEMENT);
add_rule(N_IF_STATEMENT, T_if, '(', N_EXPR, ')', N_STATEMENT, T_else, N_STATEMENT, R_IF_ELSE_STATEMENT);

add_rule(N_FOR_STATEMENT, T_for, '(', ';', N_EXPR_OR_EMPTY, ';', N_EXPR_OR_EMPTY, ')', N_STATEMENT, R_FOR_LOOP_2);
add_rule(N_FOR_STATEMENT, T_for, '(', N_EXPR, ';', N_EXPR_OR_EMPTY, ';', N_EXPR_OR_EMPTY, ')', N_STATEMENT, R_FOR_LOOP_3);
add_rule(N_FOR_STATEMENT, T_for, '(', T_var, N_VAR_DECL_LIST_NO_IN, ';', N_EXPR_OR_EMPTY, ';', N_EXPR_OR_EMPTY, ')', N_STATEMENT, R_FOR_LOOP_VAR);
add_rule(N_FOR_STATEMENT, T_for, '(', T_var, N_VAR_DECL_NO_IN, T_in, N_EXPR, ')', N_STATEMENT, R_FOR_IN_VAR);
add_rule(N_FOR_STATEMENT, T_for, '(', N_LEFT_EXPR, T_in, N_EXPR, ')', N_STATEMENT, R_FOR_IN);

add_rule(N_CASE_BLOCK, '{', '}', R_CASE_EMPTY);
add_rule(N_CASE_BLOCK, '{', N_CASE_CLAUSES, '}', R_COPY_1);
add_rule(N_CASE_BLOCK, '{', N_CASE_CLAUSES, N_DEFAULT_CLAUSE, '}', R_CASE_DEFAULT);
add_rule(N_CASE_BLOCK, '{', N_DEFAULT_CLAUSE, N_CASE_CLAUSES, '}', R_CASE_DEFAULT);
add_rule(N_CASE_BLOCK, '{', N_CASE_CLAUSES, N_DEFAULT_CLAUSE, N_CASE_CLAUSES, '}', R_CASE_DEFAULT_CASE);

add_rule(N_CASE_CLAUSES, N_CASE_CLAUSE, R_COPY_0);
add_rule(N_CASE_CLAUSES, N_CASE_CLAUSES, N_CASE_CLAUSE, R_CASE_CLAUSES);

add_rule(N_CASE_CLAUSE, T_case, N_EXPR, ':', R_CASE_CLAUSE_NOOP);
add_rule(N_CASE_CLAUSE, T_case, N_EXPR, ':', N_STATEMENT_LIST, R_CASE_CLAUSE);

add_rule(N_DEFAULT_CLAUSE, T_default, ':', R_DEFAULT_CLAUSE_NOOP);
add_rule(N_DEFAULT_CLAUSE, T_default, ':', N_STATEMENT_LIST, R_DEFAULT_CLAUSE);

add_rule(N_TRY_STATEMENT, T_try, N_BLOCK, N_CATCH, R_TRY_CATCH);
add_rule(N_TRY_STATEMENT, T_try, N_BLOCK, N_FINALLY, R_TRY_FINALLY);
add_rule(N_TRY_STATEMENT, T_try, N_BLOCK, N_CATCH, N_FINALLY, R_TRY_CATCH_FINALLY);

add_rule(N_CATCH, T_catch, '(', T_IDENTIFIER, ')', N_BLOCK, R_CATCH);

add_rule(N_FINALLY, T_finally, N_BLOCK, R_COPY_1);

add_rule(N_ID_OR_EMPTY, R_NULL);
add_rule(N_ID_OR_EMPTY, T_IDENTIFIER, R_COPY_0);

add_rule(N_FUNC_DECL, T_function, T_IDENTIFIER, '(', R_FUNC_BEGIN, N_FORMAL_PARAM_LIST_OR_EMPTY, ')', '{', R_FUNC_BODY, N_PROGRAM, '}', R_FUNC_DECL);

add_rule(N_FUNC_EXPR, T_function, N_ID_OR_EMPTY, '(', R_FUNC_BEGIN, N_FORMAL_PARAM_LIST_OR_EMPTY, ')', '{', R_FUNC_BODY, N_PROGRAM, '}', R_FUNC_EXPR);

add_rule(N_FORMAL_PARAM_LIST_OR_EMPTY);
add_rule(N_FORMAL_PARAM_LIST_OR_EMPTY, N_FORMAL_PARAM_LIST);

add_rule(N_FORMAL_PARAM_LIST, T_IDENTIFIER, R_PARAM);
add_rule(N_FORMAL_PARAM_LIST, N_FORMAL_PARAM_LIST, ',', T_IDENTIFIER, R_PARAM_2);

add_rule(N_PRIMARY_EXPR, T_NUMBER,     R_EXPR_CONST);
add_rule(N_PRIMARY_EXPR, T_IDENTIFIER, R_EXPR_ID);
add_rule(N_PRIMARY_EXPR, T_STRING,     R_EXPR_CONST);
add_rule(N_PRIMARY_EXPR, T_REGEXP,     R_EXPR_REGEXP);
add_rule(N_PRIMARY_EXPR, T_true,       R_EXPR_TRUE);
add_rule(N_PRIMARY_EXPR, T_false,      R_EXPR_FALSE);
add_rule(N_PRIMARY_EXPR, T_null,       R_EXPR_NULL);
add_rule(N_PRIMARY_EXPR, T_this,       R_EXPR_THIS);
add_rule(N_PRIMARY_EXPR, N_ARRAY_LITERAL,  R_COPY_0);
add_rule(N_PRIMARY_EXPR, N_OBJECT_LITERAL, R_COPY_0);
add_rule(N_PRIMARY_EXPR, '(', N_EXPR, ')', R_COPY_1);

add_rule(N_MEMBER_EXPR, N_PRIMARY_EXPR, R_COPY_0);
add_rule(N_MEMBER_EXPR, N_FUNC_EXPR,    R_COPY_0);
add_rule(N_MEMBER_EXPR, N_MEMBER_EXPR, '[', N_EXPR, ']', P_MEMBER,   R_GET_PROP);
add_rule(N_MEMBER_EXPR, N_MEMBER_EXPR, '.', T_IDENTIFIER, P_MEMBER,  R_MEMBER);
add_rule(N_MEMBER_EXPR, T_new, N_MEMBER_EXPR, N_ARGUMENTS_OR_EMPTY, P_NEW, R_NEW);

add_rule(N_LEFT_EXPR, N_MEMBER_EXPR, R_COPY_0);
add_rule(N_LEFT_EXPR, N_LEFT_EXPR, N_ARGUMENTS, P_MEMBER,   R_CALL);
add_rule(N_LEFT_EXPR, N_LEFT_EXPR, '[', N_EXPR, ']', P_MEMBER, R_GET_PROP);
add_rule(N_LEFT_EXPR, N_LEFT_EXPR, '.', T_IDENTIFIER, P_MEMBER, R_MEMBER);

add_rule(N_MATH_EXPR, N_LEFT_EXPR, R_COPY_0);
add_rule(N_MATH_EXPR, N_MATH_EXPR, T_INC, P_UNARY, R_POST_INC);
add_rule(N_MATH_EXPR, N_MATH_EXPR, T_DEC, P_UNARY, R_POST_DEC);
add_rule(N_MATH_EXPR, T_delete, N_MATH_EXPR, P_UNARY, R_DELETE);
add_rule(N_MATH_EXPR, T_void, N_MATH_EXPR, P_UNARY, R_VOID);
add_rule(N_MATH_EXPR, T_typeof, N_MATH_EXPR, P_UNARY, R_TYPEOF);
add_rule(N_MATH_EXPR, T_INC, N_MATH_EXPR, P_UNARY, R_PRE_INC);
add_rule(N_MATH_EXPR, T_DEC, N_MATH_EXPR, P_UNARY, R_PRE_DEC);
add_rule(N_MATH_EXPR, '+', N_MATH_EXPR, P_UNARY, R_POSITIVE);
add_rule(N_MATH_EXPR, '-', N_MATH_EXPR, P_UNARY, R_NEG);
add_rule(N_MATH_EXPR, '~', N_MATH_EXPR, P_UNARY, R_REV);
add_rule(N_MATH_EXPR, '!', N_MATH_EXPR, P_UNARY, R_NOT);
add_rule(N_MATH_EXPR, N_MATH_EXPR, '+', N_MATH_EXPR, P_ADD, R_ADD);
add_rule(N_MATH_EXPR, N_MATH_EXPR, '-', N_MATH_EXPR, P_ADD, R_SUB);
add_rule(N_MATH_EXPR, N_MATH_EXPR, '*', N_MATH_EXPR, P_MUL, R_MUL);
add_rule(N_MATH_EXPR, N_MATH_EXPR, '/', N_MATH_EXPR, P_MUL, R_DIV);
add_rule(N_MATH_EXPR, N_MATH_EXPR, '%', N_MATH_EXPR, P_MUL, R_MOD);
add_rule(N_MATH_EXPR, N_MATH_EXPR, T_SHL, N_MATH_EXPR, P_SHIFT, R_SHL);
add_rule(N_MATH_EXPR, N_MATH_EXPR, T_SHR, N_MATH_EXPR, P_SHIFT, R_SHR);
add_rule(N_MATH_EXPR, N_MATH_EXPR, T_USHR, N_MATH_EXPR, P_SHIFT, R_USHR);

add_rule(N_BASE_EXPR_NO_IN, N_MATH_EXPR, R_COPY_0);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, '<', N_BASE_EXPR_NO_IN, P_REL,  R_LT);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, '>', N_BASE_EXPR_NO_IN, P_REL,  R_GT);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_LE, N_BASE_EXPR_NO_IN, P_REL, R_LE);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_GE, N_BASE_EXPR_NO_IN, P_REL, R_GE);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_instanceof, N_BASE_EXPR_NO_IN, P_REL, R_INSTANCEOF);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_EQU, N_BASE_EXPR_NO_IN, P_EQU,   R_EQU);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_NEQ, N_BASE_EXPR_NO_IN, P_EQU,   R_NEQ);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_SAME, N_BASE_EXPR_NO_IN, P_EQU,  R_SAME);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_NSAME, N_BASE_EXPR_NO_IN, P_EQU, R_NSAME);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, '&', N_BASE_EXPR_NO_IN, P_BAND,  R_BAND);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, '|', N_BASE_EXPR_NO_IN, P_BOR,   R_BOR);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, '^', N_BASE_EXPR_NO_IN, P_XOR,   R_XOR);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_AND, N_BASE_EXPR_NO_IN, P_AND, R_AND);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, T_OR, N_BASE_EXPR_NO_IN, P_OR,   R_OR);
add_rule(N_BASE_EXPR_NO_IN, N_BASE_EXPR_NO_IN, '?', N_BASE_EXPR_NO_IN, ':', N_BASE_EXPR_NO_IN, P_COND, R_COND);

add_rule(N_BASE_EXPR, N_MATH_EXPR, R_COPY_0);
add_rule(N_BASE_EXPR, N_BASE_EXPR, '<', N_BASE_EXPR, P_REL,  R_LT);
add_rule(N_BASE_EXPR, N_BASE_EXPR, '>', N_BASE_EXPR, P_REL,  R_GT);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_LE, N_BASE_EXPR, P_REL, R_LE);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_GE, N_BASE_EXPR, P_REL, R_GE);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_instanceof, N_BASE_EXPR, P_REL, R_INSTANCEOF);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_in, N_BASE_EXPR, P_REL,  R_IN);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_EQU, N_BASE_EXPR, P_EQU, R_EQU);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_NEQ, N_BASE_EXPR, P_EQU, R_NEQ);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_SAME, N_BASE_EXPR, P_EQU,  R_SAME);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_NSAME, N_BASE_EXPR, P_EQU, R_NSAME);
add_rule(N_BASE_EXPR, N_BASE_EXPR, '&', N_BASE_EXPR, P_BAND,  R_BAND);
add_rule(N_BASE_EXPR, N_BASE_EXPR, '|', N_BASE_EXPR, P_BOR,   R_BOR);
add_rule(N_BASE_EXPR, N_BASE_EXPR, '^', N_BASE_EXPR, P_XOR,   R_XOR);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_AND, N_BASE_EXPR, P_AND, R_AND);
add_rule(N_BASE_EXPR, N_BASE_EXPR, T_OR, N_BASE_EXPR, P_OR,   R_OR);
add_rule(N_BASE_EXPR, N_BASE_EXPR, '?', N_BASE_EXPR, ':', N_BASE_EXPR, P_COND, R_COND);

add_rule(N_ASSIGN_EXPR, N_LEFT_EXPR, N_ASSIGN_OP, N_ASSIGN_EXPR, P_ASSIGN, R_ASSIGN_EXPR);
add_rule(N_ASSIGN_EXPR, N_BASE_EXPR, R_COPY_0);

add_rule(N_ASSIGN_EXPR_NO_IN, N_LEFT_EXPR, N_ASSIGN_OP, N_ASSIGN_EXPR_NO_IN, P_ASSIGN, R_ASSIGN_EXPR);
add_rule(N_ASSIGN_EXPR_NO_IN, N_BASE_EXPR_NO_IN, R_COPY_0);

add_rule(N_EXPR, N_ASSIGN_EXPR, R_COPY_0);
add_rule(N_EXPR, N_EXPR, ',', N_ASSIGN_EXPR, P_COMMA, R_COMMA_EXPR);

add_rule(N_ASSIGN_OP, '=', R_ASSIGN);
add_rule(N_ASSIGN_OP, T_ADD_ASSIGN,  R_ADD_ASSIGN);
add_rule(N_ASSIGN_OP, T_SUB_ASSIGN,  R_SUB_ASSIGN);
add_rule(N_ASSIGN_OP, T_MUL_ASSIGN,  R_MUL_ASSIGN);
add_rule(N_ASSIGN_OP, T_DIV_ASSIGN,  R_DIV_ASSIGN);
add_rule(N_ASSIGN_OP, T_MOD_ASSIGN,  R_MOD_ASSIGN);
add_rule(N_ASSIGN_OP, T_SHL_ASSIGN,  R_SHL_ASSIGN);
add_rule(N_ASSIGN_OP, T_SHR_ASSIGN,  R_SHR_ASSIGN);
add_rule(N_ASSIGN_OP, T_USHR_ASSIGN, R_USHR_ASSIGN);
add_rule(N_ASSIGN_OP, T_BAND_ASSIGN, R_BAND_ASSIGN);
add_rule(N_ASSIGN_OP, T_BOR_ASSIGN,  R_BOR_ASSIGN);
add_rule(N_ASSIGN_OP, T_XOR_ASSIGN,  R_XOR_ASSIGN);

add_rule(N_ARGUMENTS_OR_EMPTY, R_NO_ARGUMENT);
add_rule(N_ARGUMENTS_OR_EMPTY, N_ARGUMENTS, R_COPY_0);

add_rule(N_ARGUMENTS, '(', ')', R_NO_ARGUMENT);
add_rule(N_ARGUMENTS, '(', N_ARGUMENT_LIST, ')', R_COPY_1);

add_rule(N_ARGUMENT_LIST, N_ASSIGN_EXPR, R_FIRST_ARGUMENT);
add_rule(N_ARGUMENT_LIST, N_ARGUMENT_LIST, ',', N_ASSIGN_EXPR, R_APPEND_ARGUMENT);

add_rule(N_ARRAY_LITERAL, '[', N_ELISION_OR_EMPTY, ']', R_ARRAY_SET_LENGTH);
add_rule(N_ARRAY_LITERAL, '[', N_ELEMENT_LIST, ']', R_COPY_1);
add_rule(N_ARRAY_LITERAL, '[', N_ELEMENT_LIST, ',', N_ELISION_OR_EMPTY, ']', R_ARRAY_RESIZE);

add_rule(N_ELISION_OR_EMPTY, R_ZERO);
add_rule(N_ELISION_OR_EMPTY, N_ELISION, R_COPY_0);

add_rule(N_ELISION, ',', R_ONE);
add_rule(N_ELISION, N_ELISION, ',', R_ARRAY_INC_INDEX);

add_rule(N_ELEMENT_LIST, N_ELISION_OR_EMPTY, N_ASSIGN_EXPR, R_ARRAY_FIRST_INDEX);
add_rule(N_ELEMENT_LIST, N_ELEMENT_LIST, ',', N_ELISION_OR_EMPTY, N_ASSIGN_EXPR, R_ARRAY_SET_ELEMENT);

add_rule(N_OBJECT_LITERAL, '{', R_OBJECT_BEGIN, N_PROP_AND_VALUE_LIST_OR_EMPTY, '}', R_OBJECT_NEW);

add_rule(N_PROP_AND_VALUE_LIST_OR_EMPTY, R_EMPTY_EXPR);
add_rule(N_PROP_AND_VALUE_LIST_OR_EMPTY, N_PROP_AND_VALUE_LIST, R_COPY_0);

add_rule(N_PROP_AND_VALUE_LIST, N_PROP_ASSIGN, R_COPY_0);
add_rule(N_PROP_AND_VALUE_LIST, N_PROP_AND_VALUE_LIST, ',', N_PROP_ASSIGN, R_APPEND_ARGUMENT);

add_rule(N_PROP_ASSIGN, N_PROP_NAME, ':', N_ASSIGN_EXPR, R_PROP);
add_rule(N_PROP_ASSIGN, T_get, ':', N_ASSIGN_EXPR, R_PROP);
add_rule(N_PROP_ASSIGN, T_set, ':', N_ASSIGN_EXPR, R_PROP);
add_rule(N_PROP_ASSIGN, T_get, N_PROP_NAME, '(', ')', '{', R_PROP_GET_BEGIN, N_PROGRAM, '}', R_PROP_GET);
add_rule(N_PROP_ASSIGN, T_set, N_PROP_NAME, '(', T_IDENTIFIER, ')', '{', R_PROP_SET_BEGIN, N_PROGRAM, '}', R_PROP_SET);

add_rule(N_PROP_NAME, T_STRING, R_COPY_0);
add_rule(N_PROP_NAME, T_NUMBER, R_COPY_0);
add_rule(N_PROP_NAME, T_IDENTIFIER, R_COPY_0);

add_rule(N_EXPR_OR_EMPTY, R_EMPTY_EXPR);
add_rule(N_EXPR_OR_EMPTY, N_EXPR, R_COPY_0);


