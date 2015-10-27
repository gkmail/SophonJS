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

#ifndef _JS_NONTERMS_H_
#define _JS_NONTERMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FOR_EACH_NONTERM(n)\
	n(JS,      ops)\
	n(PROGRAM, ops)\
	n(ID_OR_EMPTY, v)\
	n(FUNC_DECL,   ops)\
	n(STATEMENT,   ops)\
	n(STATEMENT_LIST, ops)\
	n(EXPR_STATEMENT, ops)\
	n(IF_STATEMENT,   ops)\
	n(FOR_STATEMENT,  ops)\
	n(TRY_STATEMENT,  ops)\
	n(BLOCK,          ops)\
	n(PRIMARY_EXPR,   ops)\
	n(EXPR,           expr)\
	n(EXPR_OR_EMPTY,  expr)\
	n(LEFT_EXPR,      expr)\
	n(ASSIGN_EXPR,    expr)\
	n(ASSIGN_EXPR_NO_IN, expr)\
	n(VAR_DECL_LIST,  ops)\
	n(VAR_DECL,       var)\
	n(VAR_DECL_LIST_NO_IN, ops)\
	n(VAR_DECL_NO_IN, var)\
	n(CASE_BLOCK,     cases)\
	n(CASE_CLAUSES,   cases)\
	n(CASE_CLAUSE,    cases)\
	n(DEFAULT_CLAUSE, cases)\
	n(CATCH,          ops)\
	n(FINALLY,        ops)\
	n(FORMAL_PARAM_LIST, i)\
	n(FORMAL_PARAM_LIST_OR_EMPTY, i)\
	n(ARRAY_LITERAL,  expr)\
	n(OBJECT_LITERAL, expr)\
	n(ELISION,        i)\
	n(ELISION_OR_EMPTY, i)\
	n(ELEMENT_LIST,   expr)\
	n(PROP_AND_VALUE_LIST, expr)\
	n(PROP_AND_VALUE_LIST_OR_EMPTY, expr)\
	n(PROP_ASSIGN,    expr)\
	n(PROP_NAME,      v)\
	n(FUNC_EXPR,      expr)\
	n(MEMBER_EXPR,    expr)\
	n(ARGUMENTS,      expr)\
	n(ARGUMENTS_OR_EMPTY, expr)\
	n(ARGUMENT_LIST,  expr)\
	n(ASSIGN_OP,      i)\
	n(MATH_EXPR,      expr)\
	n(BASE_EXPR,      expr)\
	n(BASE_EXPR_NO_IN,expr)

#ifdef __cplusplus
}
#endif

#endif

