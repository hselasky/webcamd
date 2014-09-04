/*-
 * Copyright (c) 2009-2014 Hans Petter Selasky. All rights reserved.
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

#ifndef _LINUX_SECTION_H_
#define	_LINUX_SECTION_H_

#include <sys/linker_set.h>

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

#define	module_parm_init(func)	\
  TEXT_SET(linux_parm_mod, func);

#define	module_init(func)	\
  TEXT_SET(linux_init_mod, func);

#define	module_exit(func)	\
  TEXT_SET(linux_exit_mod, func);

#define	late_initcall(func)	\
  TEXT_SET(linux_late_mod, func);

#define	postcore_initcall(func)	\
  TEXT_SET(linux_late_mod, func);

#define	subsys_initcall(f) module_init(f)

#define	module_usb_driver(x) \
static int x##_init(void) { return (usb_register(&(x)) ? -ENOMEM : 0); } \
module_init(x##_init); \
static void x##_exit(void) { usb_deregister(&(x)); } \
module_exit(x##_exit)

#define	module_driver(x,r,u) \
static int x##_init(void) { return (r(&(x)) ? -ENOMEM : 0); } \
module_init(x##_init); \
static void x##_exit(void) { u(&(x)); } \
module_exit(x##_exit)

#define	module_i2c_driver(x) \
static int x##_init(void) { return (i2c_add_driver(&(x)) ? -ENOMEM : 0); } \
module_init(x##_init); \
static void x##_exit(void) { i2c_del_driver(&(x)); } \
module_exit(x##_exit)

void	linux_parm(void);
void	linux_init(void);
void	linux_late(void);
void	linux_exit(void);

int	linux_module_init_end(void);
void	linux_module_exit_end(void);

#endif					/* _LINUX_SECTION_H_ */
