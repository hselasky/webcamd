/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <linux/idr.h>

SET_DECLARE(linux_parm_mod, module_init_t);
SET_DECLARE(linux_init_mod, module_init_t);
SET_DECLARE(linux_late_mod, module_init_t);
SET_DECLARE(linux_exit_mod, module_exit_t);

void
linux_parm(void)
{
	module_init_t **t = SET_BEGIN(linux_parm_mod);
	module_init_t **t_end = SET_LIMIT(linux_parm_mod);

	for ( ; t != t_end; t++)
		(**t)();
}

void
linux_init(void)
{
	module_init_t **t = SET_BEGIN(linux_init_mod);
	module_init_t **t_end = SET_LIMIT(linux_init_mod);

	for ( ; t != t_end; t++)
		(**t)();
}

void
linux_exit(void)
{
	module_exit_t **t = SET_BEGIN(linux_exit_mod);
	module_exit_t **t_end = SET_LIMIT(linux_exit_mod);

	for ( ; t != t_end; t++)
		(**t)();
}

void
linux_late(void)
{
	module_init_t **t = SET_BEGIN(linux_late_mod);
	module_init_t **t_end = SET_LIMIT(linux_late_mod);

	for ( ; t != t_end; t++)
		(**t)();
}
