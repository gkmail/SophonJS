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

#ifndef _SOPHON_CONV_H_
#define _SOPHON_CONV_H_

#ifdef __cplusplus
extern "C" {
#endif

/**\brief Character encoding converter*/
typedef Sophon_Ptr Sophon_Conv;

/**
 * \brief Create a new character encoding converter
 * \param[out] The new converter
 * \param from Convert from this encoding
 * \param to Convert to this encoding
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_conv_create (Sophon_Conv *conv,
						Sophon_Encoding from,
						Sophon_Encoding to);

/**
 * \brief Release a characer encoding converter
 * \param[in] conv The converter to be released
 */
extern void          sophon_conv_destroy (Sophon_Conv conv);

/**
 * \brief Convert characters' encoding
 * \param[in] The converter
 * \param[in] in_buf Character input buffer
 * \param[in,out] in_size The character data size in bytes.
 * Return the real converted characters size in bytes.
 * \param[out] out_buf The output data buffer
 * \param[in,out] out_size The output buffer's size in bytes.
 * Return the real output data size in bytes.
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_conv (Sophon_Conv conv,
						const Sophon_U8 *in_buf,
						Sophon_U32 *in_size,
						Sophon_U8 *out_buf,
						Sophon_U32 *out_size);

#ifdef __cplusplus
}
#endif

#endif

