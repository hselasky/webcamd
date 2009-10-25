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

#ifndef _LINUX_END_SECTION_H_
#define	_LINUX_END_SECTION_H_

#define	MODULE_INIT_MAGIC ((uint8_t *)0 + 0x123468AC)
#define	MODULE_EXIT_MAGIC ((uint8_t *)0 + 0x1246789F)

typedef int (module_init_t)(void);
typedef void (module_exit_t)(void);

struct module_init_struct {
	module_init_t *func;
	uint8_t *magic;
};

struct module_exit_struct {
	module_exit_t *func;
	uint8_t *magic;
};

#define	module_init(func) static struct module_init_struct \
  __attribute__((__section__("linux_init_mod"),__used__,__aligned__(1))) func##_p = { func, MODULE_INIT_MAGIC };

#define	module_exit(func) static struct module_exit_struct \
  __attribute__((__section__("linux_exit_mod"),__used__,__aligned__(1))) func##_p = { func, MODULE_EXIT_MAGIC };

void	linux_init(void);
void	linux_exit(void);

int	linux_module_init_end(void);
void	linux_module_exit_end(void);

#endif					/* _LINUX_END_SECTION_H_ */
