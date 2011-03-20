/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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

static TAILQ_HEAD(, mod_param) mod_param_head = TAILQ_HEAD_INITIALIZER(mod_param_head);

int
mod_set_param(const char *name, const char *value)
{
	struct mod_param *p;

	TAILQ_FOREACH(p, &mod_param_head, entry) {
		if (strcmp(p->name, name) == 0)
			break;
	}
	if (p == NULL)
		return (-1);

	if (strcmp(p->type, "string") == 0) {
		strlcpy(p->ptr, value, p->size);
	} else if (strcmp(p->type, "short") == 0) {
		*((short *)p->ptr) = strtoul(value, NULL, 0);
	} else if (strcmp(p->type, "ushort") == 0) {
		*((unsigned short *)p->ptr) = strtoul(value, NULL, 0);
	} else if (strcmp(p->type, "int") == 0) {
		*((int *)p->ptr) = strtoul(value, NULL, 0);
	} else if (strcmp(p->type, "bool") == 0) {
		*((bool *) p->ptr) = strtoul(value, NULL, 0) ? 1 : 0;
	} else if (strcmp(p->type, "uint") == 0) {
		*((unsigned int *)p->ptr) = strtoul(value, NULL, 0);
	} else if (strcmp(p->type, "charp") == 0) {
		free(*((void **)p->ptr));
		*((void **)p->ptr) = strdup(value);
	} else {
		return (-1);
	}
	return (0);
}

void
mod_show_params(void)
{
	struct mod_param *p;

	TAILQ_FOREACH(p, &mod_param_head, entry) {
		if (strcmp(p->type, "string") == 0) {
			printf("\t%s=\"%s\" <string>\n", (const char *)p->name, (const char *)p->ptr);
		} else if (strcmp(p->type, "bool") == 0) {
			printf("\t%s=%d <bool>\n", (const char *)p->name, *((bool *) p->ptr));
		} else if (strcmp(p->type, "int") == 0) {
			printf("\t%s=%d <int>\n", (const char *)p->name, *((int *)p->ptr));
		} else if (strcmp(p->type, "uint") == 0) {
			printf("\t%s=%u <uint>\n", (const char *)p->name, *((unsigned int *)p->ptr));
		} else if (strcmp(p->type, "short") == 0) {
			printf("\t%s=%d <short>\n", (const char *)p->name, *((short *)p->ptr));
		} else if (strcmp(p->type, "ushort") == 0) {
			printf("\t%s=%u <ushort>\n", (const char *)p->name, *((unsigned short *)p->ptr));
		} else if (strcmp(p->type, "charp") == 0) {
			printf("\t%s=\"%s\" <string>\n", (const char *)p->name, *((const char **)p->ptr) ?
			    *((const char **)p->ptr) : "");
		} else {
			printf("\t%s=<unknown %s>\n", (const char *)p->name, (const char *)p->type);
		}
	}
}

void
mod_param_register(struct mod_param *p)
{
	TAILQ_INSERT_TAIL(&mod_param_head, p, entry);
}
