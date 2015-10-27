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

#ifndef _JS_PARSER_DECL_H_
#define _JS_PARSER_DECL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FOR_EACH_REDUCE(r) \
	r(COPY_0)\
	r(COPY_1)\
	r(NULL)\
	r(EXPR_CONST)\
	r(EXPR_TRUE)\
	r(EXPR_FALSE)\
	r(EXPR_NULL)\
	r(EXPR_ID)\
	r(EXPR_THIS)\
	r(EXPR_REGEXP)\
	r(GET_PROP)\
	r(MEMBER)\
	r(NEW)\
	r(CALL)\
	r(PRE_INC)\
	r(PRE_DEC)\
	r(POST_INC)\
	r(POST_DEC)\
	r(TYPEOF)\
	r(DELETE)\
	r(VOID)\
	r(REV)\
	r(POSITIVE)\
	r(NEG)\
	r(NOT)\
	r(ADD)\
	r(SUB)\
	r(MUL)\
	r(DIV)\
	r(MOD)\
	r(SHL)\
	r(SHR)\
	r(USHR)\
	r(LT)\
	r(GT)\
	r(LE)\
	r(GE)\
	r(EQU)\
	r(NEQ)\
	r(SAME)\
	r(NSAME)\
	r(INSTANCEOF)\
	r(BAND)\
	r(BOR)\
	r(XOR)\
	r(IN)\
	r(AND)\
	r(OR)\
	r(COND)\
	r(ASSIGN)\
	r(ADD_ASSIGN)\
	r(SUB_ASSIGN)\
	r(MUL_ASSIGN)\
	r(DIV_ASSIGN)\
	r(MOD_ASSIGN)\
	r(SHL_ASSIGN)\
	r(SHR_ASSIGN)\
	r(USHR_ASSIGN)\
	r(BAND_ASSIGN)\
	r(BOR_ASSIGN)\
	r(XOR_ASSIGN)\
	r(ASSIGN_EXPR)\
	r(COMMA_EXPR)\
	r(NO_ARGUMENT)\
	r(FIRST_ARGUMENT)\
	r(APPEND_ARGUMENT)\
	r(ARRAY_SET_LENGTH)\
	r(ARRAY_RESIZE)\
	r(ZERO)\
	r(ONE)\
	r(ARRAY_INC_INDEX)\
	r(ARRAY_FIRST_INDEX)\
	r(ARRAY_SET_ELEMENT)\
	r(OBJECT_NEW)\
	r(PROP)\
	r(PROP_GET)\
	r(PROP_SET)\
	r(PROP_GET_BEGIN)\
	r(PROP_SET_BEGIN)\
	r(EMPTY_STATEMENT)\
	r(EMPTY_OPS)\
	r(MERGE_OPS)\
	r(MERGE_OPS_2)\
	r(CONTINUE)\
	r(CONTINUE_LABEL)\
	r(BREAK)\
	r(BREAK_LABEL)\
	r(RETURN)\
	r(RETURN_VALUE)\
	r(VAR)\
	r(VAR_INIT)\
	r(EXPR_STATEMENT)\
	r(IF_STATEMENT)\
	r(IF_ELSE_STATEMENT)\
	r(DO_STATEMENT)\
	r(WHILE_STATEMENT)\
	r(EMPTY_EXPR)\
	r(FOR_LOOP_2)\
	r(FOR_LOOP_3)\
	r(FOR_LOOP_VAR)\
	r(FOR_IN)\
	r(FOR_IN_VAR)\
	r(THROW)\
	r(CATCH)\
	r(TRY_CATCH)\
	r(TRY_FINALLY)\
	r(TRY_CATCH_FINALLY)\
	r(WITH)\
	r(SWITCH)\
	r(CASE_EMPTY)\
	r(CASE_DEFAULT)\
	r(CASE_DEFAULT_CASE)\
	r(CASE_CLAUSES)\
	r(CASE_CLAUSE)\
	r(CASE_CLAUSE_NOOP)\
	r(DEFAULT_CLAUSE)\
	r(DEFAULT_CLAUSE_NOOP)\
	r(LABEL)\
	r(PARAM)\
	r(PARAM_2)\
	r(FUNC_BEGIN)\
	r(FUNC_BODY)\
	r(FUNC_DECL)\
	r(FUNC_EXPR)\
	r(OBJECT_BEGIN)\
	r(VAR_STATEMENT)\
	r(DEBUGGER)\
	r(ACCEPT)

#ifdef __cplusplus
}
#endif

#endif

