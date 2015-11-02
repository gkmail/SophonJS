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

add_rule(C_BEGIN, "[\\x09\\x0B\\x0C\\u0020\\u00a0\\u1680\\u180e\\u2000-\\u200a\\u202f\\u205f\\u3000\\uFEFF]+", A_NONE);
add_rule(C_BEGIN, "\\r\\n", A_LINE_TERM);
add_rule(C_BEGIN, "[\\r\\n\\u2028\\u2029]", A_LINE_TERM);
add_rule(C_BEGIN, "0[0-7]+", A_OCT_NUM_EXT);
add_rule(C_BEGIN, "0[xX][[:xdigit:]]+", A_HEX_NUM);
add_rule(C_BEGIN, "0[oO][[0-7]]+", A_OCT_NUM);
add_rule(C_BEGIN, "0[bB][[01]]+", A_BIN_NUM);
add_rule(C_BEGIN, "[[:digit:]]+", A_DEC_NUM);
add_rule(C_BEGIN, "[[:digit:]]+\".\"?[[:digit:]]*([eE][\\+\\-]?[[:digit:]]+)?", A_DOUBLE);
add_rule(C_BEGIN, "\".\"[[:digit:]]+([eE][\\+\\-]?[[:digit:]]+)?", A_DOUBLE);
add_rule(C_BEGIN, "\"/*\"", A_COMMENT_BLOCK_BEGIN);
add_rule(C_BEGIN, "\"//\"", A_COMMENT_LINE_BEGIN);
add_rule(C_BEGIN, "\\\"", A_STRING);
add_rule(C_BEGIN, "\\\'", A_STRING);
add_rule(C_BEGIN, "[_[:alpha:]][[:alnum:]_]*", A_CHAR);
add_rule(C_BEGIN, ".", A_CHAR);

add_rule(C_COMMENT_BLOCK, "\"*/\"", A_COMMENT_BLOCK_END);
add_rule(C_COMMENT_BLOCK, "\\r\\n", A_COMMENT_LINE_TERM);
add_rule(C_COMMENT_BLOCK, "[\\r\\n\\u2028\\u2029]", A_COMMENT_LINE_TERM);
add_rule(C_COMMENT_BLOCK, ".", A_NONE);

add_rule(C_COMMENT_LINE, "\\r\\n", A_COMMENT_LINE_END);
add_rule(C_COMMENT_LINE, "[\\r\\n\\u2028\\u2029]", A_COMMENT_LINE_END);
add_rule(C_COMMENT_LINE, ".", A_NONE);

