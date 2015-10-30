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

#ifndef _SOPHON_INS_H_
#define _SOPHON_INS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

/**
 * \brief Get an instruction's tag
 * \param ins An instruction
 * \return The instruction's tag
 * \retval "ILLEGAL" When ins is not a valid instruction
 */
extern const char* sophon_ins_get_tag (Sophon_U8 ins);

/**
 * \brief Dump an instruction's information to the output
 * \param[in] vm The current virtual machine
 * \param[in] func The function contains this instruction
 * \param[in] pi The instruction buffer
 * \param len The instruction buffer's length
 * \return The instruction's length
 */
extern Sophon_Int  sophon_ins_dump (Sophon_VM *vm,
					Sophon_Function *func,
					Sophon_U8 *pi, Sophon_U32 len);

/**\brief Dump instruction pointer*/
#define SOPHON_INS_FL_IP   1
/**\brief Dump line number*/
#define SOPHON_INS_FL_LINE 2

/**
 * \brief Dump all the instruction information to the output
 * \param[in] vm The current virtual machine
 * \param[in] func The function contains this instruction
 * \param[in] pi The instruction buffer
 * \param len The instruction buffer's length
 * \param flags Dump flags
 */
extern void        sophon_ins_dump_buffer (Sophon_VM *vm,
					Sophon_Function *func,
					Sophon_U8 *pi, Sophon_U32 len,
					Sophon_U32 flags);

/**
 * \brief Run the instructions
 * \param[in] vm The current virtual machine
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_ins_run (Sophon_VM *vm);

#ifdef SOPHON_LINE_INFO

/**
 * \brief Get the line number of the instruction
 * \param[in] vm The current virtual machine
 * \param[in] func The function contains this instruction
 * \return The line number of the instruction
 */
extern Sophon_U16  sophon_ins_get_line (Sophon_VM *vm,
					Sophon_Function *func,
					Sophon_U8 *pi);
#endif /*SOPHON_LINE_INFO*/

#ifdef __cplusplus
}
#endif

#endif

