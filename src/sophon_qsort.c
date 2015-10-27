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

#include <sophon_types.h>
#include <sophon_mm.h>
#include <sophon_util.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

#define swap(a, b)\
	SOPHON_MACRO_BEGIN\
        Sophon_Value t;\
        t = *(a);\
        *(a) = *(b);\
        *(b) = t;\
    SOPHON_MACRO_END

#define vecswap(a, b, n)\
	    if ((n) > 0) swapfunc(a, b, n)

static void
swapfunc (Sophon_Value *a, Sophon_Value *b, Sophon_Int n)
{
	Sophon_Value t;
	while (n--) {
		t = *a;
		*a++ = *b;
		*b++ = t;
	}
}

static Sophon_Result
med3(Sophon_VM *vm, Sophon_Value *a, Sophon_Value *b, Sophon_Value *c,
		Sophon_CmpFunc cmp, Sophon_Ptr arg, Sophon_Value **pr)
{
	Sophon_Int r;

	r = cmp(vm, *a, *b, arg);
	if (sophon_has_exception(vm))
		return SOPHON_ERR_THROW;

	if (r < 0) {
		r = cmp(vm, *b, *c, arg);
		if (sophon_has_exception(vm))
			return SOPHON_ERR_THROW;

		if (r < 0) {
			*pr = b;
		} else {
			r = cmp(vm, *a, *c, arg);
			if (sophon_has_exception(vm))
				return SOPHON_ERR_THROW;

			*pr = (r < 0) ? c : a;
		}
	} else {
		r = cmp(vm, *b, *c, arg);
		if (sophon_has_exception(vm))
			return SOPHON_ERR_THROW;

		if (r > 0) {
			*pr = b;
		} else {
			r = cmp(vm, *a, *c, arg);
			if (sophon_has_exception(vm))
				return SOPHON_ERR_THROW;

			*pr = (r < 0) ? a : c;
		}
	}

	return SOPHON_OK;
}

Sophon_Result
sophon_qsort (Sophon_VM *vm, Sophon_Value *aa, Sophon_Int n,
		Sophon_CmpFunc cmp, Sophon_Ptr arg)
{
	Sophon_Value *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	Sophon_Int d, r, swap_cnt;
	Sophon_Value *a = aa;
	Sophon_Result ret;

loop:
	swap_cnt = 0;

	if (n < 7) {
		for (pm = a + 1; pm < a + n; pm++) {
			for (pl = pm; pl > a; pl --) {
				r = cmp(vm, pl[-1], pl[0], arg);
				if (sophon_has_exception(vm))
					return SOPHON_ERR_THROW;
				if (r <= 0)
					break;
				swap(pl, pl - 1);
			}
		}
		return SOPHON_OK;
	}

	pm = a + (n / 2);
	if (n > 7) {
		pl = a;
		pn = a + (n - 1);
		if (n > 40) {
			d = (n / 8);
			ret = med3(vm, pl, pl + d, pl + 2 * d, cmp, arg, &pl);
			if (ret != SOPHON_OK) return ret;
			ret = med3(vm, pm - d, pm, pm + d, cmp, arg, &pm);
			if (ret != SOPHON_OK) return ret;
			ret = med3(vm, pn - 2 * d, pn - d, pn, cmp, arg, &pn);
			if (ret != SOPHON_OK) return ret;
		}
		ret = med3(vm, pl, pm, pn, cmp, arg, &pm);
		if (ret != SOPHON_OK) return ret;
	}
	swap(a, pm);
	pa = pb = a + 1;

	pc = pd = a + (n - 1);
	for (;;) {
		while (pb <= pc) {
			r = cmp(vm, *pb, *a, arg);
			if (sophon_has_exception(vm))
				return SOPHON_ERR_THROW;
			if (r > 0)
				break;
			if (r == 0) {
				swap_cnt = 1;
				swap(pa, pb);
				pa ++;
			}
			pb ++;
		}
		while (pb <= pc) {
			r = cmp(vm, *pc, *a, arg);
			if (sophon_has_exception(vm))
				return SOPHON_ERR_THROW;
			if (r < 0)
				break;
			if (r == 0) {
				swap_cnt = 1;
				swap(pc, pd);
				pd --;
			}
			pc --;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		swap_cnt = 1;
		pb ++;
		pc --;
	}

	if (swap_cnt == 0) {
		for (pm = a + 1; pm < a + n; pm ++) {
			for (pl = pm; pl > a; pl --) {
				r = cmp(vm, pl[-1], pl[0], arg);
				if (sophon_has_exception(vm))
					return SOPHON_ERR_THROW;
				if (r <= 0)
					break;
				swap(pl, pl - 1);
			}
		}
		return SOPHON_OK;
	}
	
	pn = a + n;
	r = SOPHON_MIN(pa - a, pb - pa);
	vecswap(a, pb - r, r);
	r = SOPHON_MIN(pd - pc, pn - pd - 1);
	vecswap(pb, pn - r, r);
	if ((r = pb - pa) > 1)
		if ((ret = sophon_qsort(vm, a, r, cmp, arg)) != SOPHON_OK)
			return ret;
	if ((r = pd - pc) > 1) {
		a = pn - r;
		n = r;
		goto loop;
	}

	return SOPHON_OK;
}
