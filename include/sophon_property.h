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

#ifndef _SOPHON_PROPERTY_H_
#define _SOPHON_PROPERTY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_value.h"

/**\brief The property is writable*/
#define SOPHON_PROP_ATTR_WRITABLE     1
/**\brief The property is enumerable*/
#define SOPHON_PROP_ATTR_ENUMERABLE   2
/**\brief The property is configurable*/
#define SOPHON_PROP_ATTR_CONFIGURABLE 4
/**\brief The data property's value */
#define SOPHON_PROP_ATTR_ACCESSOR     8

/**\brief Default property attributes*/
#define SOPHON_DATA_PROP_ATTR\
		(SOPHON_PROP_ATTR_WRITABLE|\
				SOPHON_PROP_ATTR_ENUMERABLE|\
				SOPHON_PROP_ATTR_CONFIGURABLE)
/**\brief Default accessor property attributes*/
#define SOPHON_ACCESSOR_PROP_ATTR\
		(SOPHON_PROP_ATTR_WRITABLE|\
				SOPHON_PROP_ATTR_ENUMERABLE|\
				SOPHON_PROP_ATTR_CONFIGURABLE|\
				SOPHON_PROP_ATTR_ACCESSOR)

/**\brief The 'writable' attribute is set*/
#define SOPHON_FL_HAVE_WRITABLE     1
/**\brief The 'enumerable' attribute is set*/
#define SOPHON_FL_HAVE_ENUMERABLE   2
/**\brief The 'configurable' attribute is set*/
#define SOPHON_FL_HAVE_CONFIGURABLE 4
/**\brief The 'value' attribute is set*/
#define SOPHON_FL_HAVE_VALUE        8
/**\brief The 'set' attribute is set*/
#define SOPHON_FL_HAVE_GET          16
/**\brief The 'get' attribute is set*/
#define SOPHON_FL_HAVE_SET          32
/**\brief Throw exception on error*/
#define SOPHON_FL_THROW             64
/**\brief Return SOPHON_NONE if the property do not exist*/
#define SOPHON_FL_NONE              128
/**\brief Just push stack and do not run instruction*/
#define SOPHON_FL_LAZY              256
/**\brief Just get own property*/
#define SOPHON_FL_OWN               512
/**\brief Force the operation*/
#define SOPHON_FL_FORCE             1024

/**\brief Default property attributes flags*/
#define SOPHON_FL_DATA_PROP\
		(SOPHON_FL_HAVE_WRITABLE|\
				SOPHON_FL_HAVE_ENUMERABLE|\
				SOPHON_FL_HAVE_CONFIGURABLE|\
				SOPHON_FL_HAVE_VALUE)
/**\brief Default accessor property attributes flags*/
#define SOPHON_FL_ACCESSOR_PROP\
		(SOPHON_FL_HAVE_WRITABLE|\
				SOPHON_FL_HAVE_ENUMERABLE|\
				SOPHON_FL_HAVE_CONFIGURABLE|\
				SOPHON_FL_HAVE_GET|\
				SOPHON_FL_HAVE_SET)

/**\brief Property iterator*/
struct Sophon_PropIter_s {
	Sophon_PropIter *bottom; /**< The bottom iterator in the VM stack*/
	Sophon_Property *prop;   /**< The property*/
	Sophon_U16       brk_ip; /**< Break instruction pointer*/
	Sophon_U16       cont_ip;/**< Continue instruction pointer*/
	Sophon_U16       sp;     /**< Stack pointer*/
};

/**\brief Property*/
struct Sophon_Property_s {
	Sophon_Property *next;  /**< The next property in the list*/
	Sophon_String   *name;  /**< The property's name*/
	Sophon_U32       attrs; /**< The property's attributes*/
	Sophon_Value     value; /**< The data property's value*/
};

/**\brief Accessor Property*/
struct Sophon_AccessorProperty_s {
	Sophon_Property *next;  /**< The next property in the list*/
	Sophon_String   *name;  /**< The property's name*/
	Sophon_U32       attrs; /**< The property's attributes*/
	Sophon_Value     getv;  /**< The get function value*/
	Sophon_Value     setv;  /**< The set function value*/
};

#define sophon_prop_iter_stack_init(vm) ((vm)->pi_stack = NULL)

#ifdef __cplusplus
}
#endif

#endif

