/*-
 * Copyright (c) 2011 Hans Petter Selasky <hselasky@freebsd.org>
 * All rights reserved.
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <err.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/queue.h>
#include <sys/stat.h>
#include <errno.h>

struct node;
typedef TAILQ_HEAD(, node) node_head_t;
typedef TAILQ_ENTRY(node) node_entry_t;

struct config;
typedef TAILQ_HEAD(, config) config_head_t;
typedef TAILQ_ENTRY(config) config_entry_t;

struct directory;
typedef TAILQ_HEAD(, directory) directory_head_t;
typedef TAILQ_ENTRY(directory) directory_entry_t;

struct makefile;
typedef TAILQ_HEAD(, makefile) makefile_head_t;
typedef TAILQ_ENTRY(makefile) makefile_entry_t;

#if 0
void
style_fix()
{
}

#endif

struct node {
	node_entry_t entry;
	node_head_t children;
	char   *name;			/* obj-y/obj-m/obj-n/xxx.o */
};

struct directory {
	directory_entry_t entry;
	char   *name;
};

struct makefile {
	makefile_entry_t entry;
	char   *name;
};

struct config {
	config_entry_t entry;
	char   *name;
	char	value;			/* m/y/n */
};

static node_head_t rootNode = TAILQ_HEAD_INITIALIZER(rootNode);
static config_head_t rootConfig = TAILQ_HEAD_INITIALIZER(rootConfig);
static directory_head_t rootDirectory = TAILQ_HEAD_INITIALIZER(rootDirectory);
static makefile_head_t rootMakefile = TAILQ_HEAD_INITIALIZER(rootMakefile);
static int opt_verbose;

static char *opt_config = "config";
static char *opt_input;
static char *opt_output;

static char *
strcatdup(const char *a, const char *b)
{
	int al = strlen(a);
	int bl = strlen(b);
	char *ptr;

	ptr = malloc(al + bl + 1);
	if (ptr == NULL)
		errx(EX_SOFTWARE, "Out of memory.");

	memcpy(ptr, a, al);
	memcpy(ptr + al, b, bl + 1);
	return (ptr);
}

static struct node *
new_node()
{
	struct node *temp;

	temp = malloc(sizeof(*temp));
	if (temp == NULL)
		errx(EX_SOFTWARE, "Out of memory.");
	memset(temp, 0, sizeof(*temp));
	TAILQ_INIT(&temp->children);
	return (temp);
}

static struct directory *
new_directory()
{
	struct directory *temp;

	temp = malloc(sizeof(*temp));
	if (temp == NULL)
		errx(EX_SOFTWARE, "Out of memory.");
	memset(temp, 0, sizeof(*temp));
	return (temp);
}

static void
new_makefile(char *name)
{
	struct makefile *temp;

	temp = malloc(sizeof(*temp));
	if (temp == NULL)
		errx(EX_SOFTWARE, "Out of memory.");
	memset(temp, 0, sizeof(*temp));
	temp->name = name;
	TAILQ_INSERT_TAIL(&rootMakefile, temp, entry);
}

static void
new_config(char *name, char what)
{
	struct config *c0;

	TAILQ_FOREACH(c0, &rootConfig, entry) {
		if (strcmp(c0->name, name) == 0) {
			c0->value = what;
			free(name);
			return;
		}
	}

	c0 = malloc(sizeof(*c0));
	if (c0 == NULL)
		errx(EX_SOFTWARE, "Out of memory.");
	memset(c0, 0, sizeof(*c0));
	c0->name = name;
	c0->value = what;

	TAILQ_INSERT_TAIL(&rootConfig, c0, entry);
}

static void
free_node(struct node *node)
{
	free(node->name);
	free(node);
}

static char *
load_file(char *path)
{
	int f;
	long off;
	char *ptr;

	if (opt_verbose > 0)
		fprintf(stderr, "Loading %s\n", path);

	f = open(path, O_RDONLY);

	free(path);

	if (f < 0)
		return (NULL);

	off = lseek(f, 0, SEEK_END);
	lseek(f, 0, SEEK_SET);

	if (off < 0) {
		close(f);
		return (NULL);
	}
	ptr = malloc(off + 1);
	if (ptr == NULL) {
		close(f);
		return (NULL);
	}
	if (read(f, ptr, off) != off) {
		close(f);
		return (NULL);
	}
	ptr[off] = 0;

	while (off--) {
		if (ptr[off] == 0)
			ptr[off] = ' ';
	}
	return (ptr);
}

static char *
read_line(char **ptr, int *line)
{
	int c;
	char *old = *ptr;
	int ignore_nl = 0;

	if (*old == 0)
		return (NULL);

	(*line)++;

	while (1) {
		c = **ptr;
		if (c == 0)
			break;

		(*ptr)++;

		if (c == '\\') {
			*((*ptr) - 1) = ' ';
			ignore_nl = 1;
		}
		if (c == '\n') {

			if (ignore_nl) {
				*((*ptr) - 1) = ' ';
				ignore_nl = 0;
			} else {
				*((*ptr) - 1) = 0;
				break;
			}
		}
	}
	return (old);
}

static void
skip_while(char **ptr, char *what)
{
	int c;

	while (1) {
		c = **ptr;
		if ((c != 0) && (strchr(what, c) != NULL)) {
			(*ptr)++;
			continue;
		}
		break;
	}
}

static void
skip_until(char **ptr, char *what)
{
	int c;

	while (1) {
		c = **ptr;
		if ((c != 0) && (strchr(what, c) == NULL)) {
			(*ptr)++;
			continue;
		}
		break;
	}
}

static struct node *
add_node(node_head_t *head, char *name)
{
	struct node *node;

	TAILQ_FOREACH(node, head, entry) {
		if (strcmp(node->name, name) == 0) {
			free(name);
			return (node);
		}
	}
	node = new_node();
	node->name = name;
	TAILQ_INSERT_TAIL(head, node, entry);
	return (node);
}

static void
add_child(struct node *parent, char *name)
{
	struct node *node;

	node = new_node();
	node->name = name;

	TAILQ_INSERT_TAIL(&parent->children, node, entry);
}

static void
remove_duplicate(node_head_t *head)
{
	struct node *n0;
	struct node *n1;
	struct node *n2;

	TAILQ_FOREACH(n0, head, entry) {

		TAILQ_FOREACH_SAFE(n1, head, entry, n2) {

			if (n0 == n1)
				continue;

			if (strcmp(n0->name, n1->name) == 0) {
				TAILQ_REMOVE(head, n1, entry);
				free_node(n1);
			}
		}
	}
}

static void
resolve_nodes(void)
{
	struct node *n0;
	struct node *n1;
	struct node *n2;
	struct node *n3;

	TAILQ_FOREACH(n0, &rootNode, entry) {

		TAILQ_FOREACH_SAFE(n1, &n0->children, entry, n2) {

			if (n1->name[0] == '$') {
				char *ptr = n1->name;
				int len = strlen(n1->name);

				if (ptr[1] == '(') {
					ptr += 2;
					len -= 2;
				}
				if ((len != 0) && (ptr[len - 1] == ')')) {
					ptr[len - 1] = 0;
					len--;
				}
				TAILQ_REMOVE(&n0->children, n1, entry);

				TAILQ_FOREACH(n3, &rootNode, entry) {

					if (n3 == n0)
						continue;

					if (strcmp(n3->name, ptr) == 0) {
						TAILQ_REMOVE(&rootNode, n3, entry);
						TAILQ_CONCAT(&n0->children, &n3->children, entry);
						free_node(n3);
						break;
					}
				}
				free_node(n1);
			}
		}
	}

	TAILQ_FOREACH(n0, &rootNode, entry)
		remove_duplicate(&n0->children);
}

static void
objs_exec(char *ptr, void (*fn) (char *name))
{
	static int recurse;

	struct node *n0;
	struct node *n1;
	char *temp;
	int len;

	if (recurse > 64) {
		errx(EX_SOFTWARE, "Recursive object "
		    "execute limit of 64 exceeded for '%s'.", ptr);
	}

	recurse++;

	temp = strcatdup(ptr, "");
	len = strlen(temp);

	if ((len >= 2) && (temp[len - 2] == '.' && temp[len - 1] == 'o')) {
		temp[len - 2] = 0;
		len -= 2;
	}
	TAILQ_FOREACH(n0, &rootNode, entry) {
		if (strstr(n0->name, temp) == n0->name) {

			if (opt_verbose > 1) {
				fprintf(stderr, "matching %s "
				    "= %s\n", temp, n0->name);
			}
			/* Expecting: <match>-<y/n/m/objs><null> */
			if (strcmp(n0->name + len, "-y") == 0 ||
			    strcmp(n0->name + len, "-n") == 0 ||
			    strcmp(n0->name + len, "-m") == 0 ||
			    strcmp(n0->name + len, "-objs") == 0) {
				TAILQ_FOREACH(n1, &n0->children, entry) {
					objs_exec(n1->name, fn);
				}
				goto done;
			}
		}
	}

	fn(temp);

done:
	free(temp);

	recurse--;
}

static void
dump_nodes(node_head_t *head)
{
	static int level;
	struct node *n0;
	int x;

	TAILQ_FOREACH(n0, head, entry) {

		for (x = 0; x != level; x++)
			fprintf(stderr, "\t");

		fprintf(stderr, "%s%s\n", n0->name,
		    TAILQ_FIRST(&n0->children) ? " =" : "");

		level++;

		dump_nodes(&n0->children);

		level--;
	}
}

static char
get_config_entry(const char *what)
{
	struct config *c0;

	TAILQ_FOREACH(c0, &rootConfig, entry) {
		if (strcmp(c0->name, what) == 0) {
			return (c0->value);
		}
	}

	new_config(strcatdup(what, ""), 'n');

	return ('n');
}

static void
parse_config(char *path)
{
	char *file;
	char *old_file;
	char *ptr;
	char *temp;
	char *keyword;
	int line;
	int c;

	line = 0;
	old_file = file = load_file(strcatdup(path, ""));
	if (file == NULL)
		errx(EX_NOINPUT, "File '%sconfig' does not exist.", path);

	while ((ptr = read_line(&file, &line))) {

		skip_while(&ptr, "\t\r ");

		/* discard comments */
		if (*ptr == '#')
			continue;

		/* get next word */
		temp = ptr;
		skip_until(&temp, "\t\r =");
		if (temp == ptr)
			continue;

		/* duplicate word */
		c = *temp;
		*temp = 0;
		keyword = strcatdup(ptr, "");
		*temp = c;

		/* skip operator */
		ptr = temp;
		skip_while(&temp, "\t\r =");
		if (temp == ptr) {
			free(keyword);
			continue;
		}
		/* figure out value */
		switch (temp[0]) {
		case 'y':
			new_config(keyword, 'y');
			break;
		case 'm':
			new_config(keyword, 'm');
			break;
		case 'n':
			new_config(keyword, 'n');
			break;
		default:
			errx(EX_NOINPUT, "Invalid configuration value '%c' at line %d.", temp[0], line);
			break;
		}
	}
	free(old_file);
}

static void
parse_makefile(char *path)
{
	static int recurse;

	struct node *node;
	char *file;
	char *old_file;
	char *ptr;
	char *temp;
	char *parent;
	char *child;
	char *operator;
	struct directory *dir;
	int line;
	char c;

	if (recurse > 64) {
		errx(EX_SOFTWARE, "Recursive parse Makefile "
		    "limit of 64 exceeded in '%s'.", path);
	}

	recurse++;

	dir = new_directory();
	dir->name = strcatdup(path, "");

	TAILQ_INSERT_TAIL(&rootDirectory, dir, entry);

	line = 0;
	old_file = file = load_file(strcatdup(path, "Makefile"));
	if (file == NULL)
		errx(EX_NOINPUT, "File '%sMakefile' does not exist.", path);

	while ((ptr = read_line(&file, &line))) {

		if (opt_verbose > 1)
			fprintf(stderr, "line %d = %s\n", line, ptr);

		skip_while(&ptr, "\t\r ");

		/* discard comments */
		if (*ptr == '#')
			continue;

		/* get next word */
		temp = ptr;
		skip_until(&temp, "\t\r :+=");
		if (temp == ptr)
			continue;

		/* duplicate word */
		c = *temp;
		*temp = 0;
		parent = strcatdup(ptr, "");
		*temp = c;

		/* perform any variable substitution */
		operator = strchr(parent, '$');
		if (operator != NULL) {

			char *pt;
			int len;

			pt = operator;
			len = strlen(pt);

			if (operator[1] == '(') {
				operator += 2;
				len -= 2;
			}
			if ((len != 0) && (operator[len - 1] == ')')) {
				len--;
				operator[len] = 0;
			}
			pt[0] = get_config_entry(operator);
			pt[1] = 0;
		}
		/* get next operator, if any */
		ptr = temp;
		skip_while(&ptr, "\t\r ");
		temp = ptr;
		skip_while(&temp, ":+=");
		if (temp == ptr) {
			free(parent);
			continue;
		}
		node = add_node(&rootNode, parent);

		while (1) {

			int len;

			ptr = temp;
			skip_while(&ptr, "\t\r ");
			temp = ptr;
			skip_until(&temp, "\t\r ");
			if (temp == ptr)
				break;

			c = *temp;
			*temp = 0;
			child = strcatdup(ptr, "");
			*temp = c;

			len = strlen(child);
			if (child[len - 1] == '/') {

				if (strcmp(node->name, "obj-y") == 0 ||
				    strcmp(node->name, "obj-m") == 0)
					parse_makefile(strcatdup(path, child));

			} else {
				add_child(node, child);
			}
		}
	}

	free(old_file);

	free(path);

	recurse--;
}

static void
print_source(char *name)
{
	printf("\t%s.c \\\n", name);
}

static void
output_makefile(char *name)
{
	struct node *n0;
	struct node *n1;
	struct directory *dir;

	printf("#\n"
	    "# This makefile was automatically generated. Do not edit!\n"
	    "#\n"
	    "\n");

	printf("SRCPATHS+= \\\n");

	TAILQ_FOREACH(dir, &rootDirectory, entry) {
		int c;
		int len;
		len = strlen(dir->name);
		if (len > 0)
			c = dir->name[len-1];
		else
			c = 0;

		if (c == '/')
			dir->name[len-1] = 0;

		printf("%s \\\n", dir->name);

		if (c == '/')
			dir->name[len-1] = c;
	}

	printf("\n"
	    "SRCS+= \\\n");

	n0 = add_node(&rootNode, strcatdup(name, ""));

	TAILQ_FOREACH(n1, &n0->children, entry) {
		objs_exec(n1->name, &print_source);
	}

	printf("\n");

	printf(".if defined(LIB)\n"
	       ".include \"${.CURDIR}/../../Makefile.lib\"\n"
	       ".endif\n");
}

static void
set_stdout(char *name)
{
	static int f = -1;

	fflush(stdout);

	if (f > -1)
		close(f);

	f = open(name, O_WRONLY | O_TRUNC | O_CREAT, 0660);

	free(name);

	if (f < 0)
		err(EX_NOINPUT, "Could not set standard out");

	if (dup2(f, STDOUT_FILENO) < 0)
		err(EX_NOINPUT, "Could not duplicate file descriptor");
}

static char *
add_slashdup(char *ptr)
{
	int len = strlen(ptr);
	if (len && (ptr[len - 1] != '/'))
		return (strcatdup(ptr, "/"));
	return (strcatdup(ptr, ""));
}

static char *
fname(char *ptr)
{
	char *old = ptr;

	while (*ptr) {
		if (*ptr == '/')
			old = ptr + 1;
		ptr++;
	}
	return (old);
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: linux_make -c config -o . -i linuxA/ -i linuxB/\n"
	    "	-c <config-file>\n"
	    "	-i <input-directory>\n"
	    "	-o <output-directory>\n"
	    "	-h show help message\n"
	    "	-v increase verbosity\n"
	);
	exit(EX_USAGE);
}

static const char *targets[] = {
	"all",
	"clean",
	"cleandepend",
	"depend",
	NULL
};

int
main(int argc, char **argv)
{
	const char *params = "c:i:o:hv";
	struct node *n0;
	struct node *n1;
	struct makefile *m0;
	struct config *c0;
	int c;

        while ((c = getopt(argc, argv, params)) != -1) {
                switch (c) {
                case 'c':
			opt_config = optarg;
                        break;
                case 'i':
			opt_input = optarg;
                        break;
                case 'o':
			opt_output = optarg;
                        break;
		case 'v':
			opt_verbose++;
			break;
                default:
                        usage();
                        break;
                }
        }

	if (opt_input == NULL || opt_output == NULL)
		usage();

	opt_output = add_slashdup(opt_output);

	parse_config(opt_config);

	optreset = 1;
	optind = 1;

        while ((c = getopt(argc, argv, params)) != -1) {
                switch (c) {
                case 'i':
			opt_input = add_slashdup(optarg);
			parse_makefile(opt_input);
			opt_input = NULL;
                        break;
                default:
                        break;
                }
        }

	resolve_nodes();

	if (opt_verbose > 1)
		dump_nodes(&rootNode);

	if (mkdir(strcatdup(opt_output, "obj-y"), 0755) != 0 &&
	    errno != EEXIST)
		err(EX_NOINPUT, "Could not create output directory.");

	set_stdout(strcatdup(opt_output, "obj-y/Makefile"));

	new_makefile(strcatdup(opt_output, "obj-y"));

	output_makefile("obj-y");

	n0 = add_node(&rootNode, strcatdup("obj-m", ""));

	TAILQ_FOREACH(n1, &n0->children, entry) {
		char *temp;
		char *name;
		int len;

		temp = strcatdup(n1->name, "");
		len = strlen(temp);

		if ((len >= 2) && (temp[len - 2] == '.' &&
		    temp[len - 1] == 'o')) {
			temp[len - 2] = 0;
			len -= 2;
		}
		name = strcatdup(temp, "-objs");
		free(temp);

		temp = strcatdup(opt_output, name);
		free(name);

		if (mkdir(temp, 0755) != 0 && errno != EEXIST)
			err(EX_NOINPUT, "Could not create output directory.");

		set_stdout(strcatdup(temp, "/Makefile"));

		new_makefile(strcatdup(temp, ""));

		output_makefile(temp + strlen(opt_output));

		free(temp);
	}

	set_stdout(strcatdup(opt_output, "Makefile.top"));

	printf("#\n"
	    "# This makefile was automatically generated. Do not edit!\n"
	    "#\n");

	printf("\nMAKE?=make\n");
	printf("\nMAKE_ARGS?=\n");

	for (c = 0; targets[c]; c++) {
		printf("\n%s:\n", targets[c]);
		TAILQ_FOREACH(m0, &rootMakefile, entry) {
			char *ptr;
			ptr = fname(m0->name);
			printf("\tcd %s; ${MAKE} LIB=%s%s ${MAKE_ARGS} %s\n",
			    ptr, ptr, strcmp(ptr, "obj-y") ?
			    " MODULE=YES" : "", targets[c]);
		}
	}

	printf("\ninstall:\n");
	printf("\t[ -d modules] || mkdir modules\n");
	TAILQ_FOREACH(m0, &rootMakefile, entry) {
	    printf("\tcp -v %s/%s{.a,.so} modules/\n",
	        m0->name, fname(m0->name));
	}

	set_stdout(strcatdup(opt_output, "config.h"));

	printf("/*\n"
	    " * This file was autogenerated. Please do not edit.\n"
	    " */\n\n");

	printf("#ifndef _ROOT_CONFIG_H_\n"
	    "#define\t_ROOT_CONFIG_H_\n\n");

	TAILQ_FOREACH(c0, &rootConfig, entry) {

		if (c0->value == 'm') {
			printf("#define\t%s_MODULE /* %c */\n",
			    c0->name, c0->value);
		} else if (c0->value == 'y') {
			printf("#define\t%s /* %c */\n",
			    c0->name, c0->value);
		} else {
			printf("#undef\t%s /* %c */\n",
			    c0->name, c0->value);
		}
	}

	printf("\n#endif\t\t\t/* _ROOT_CONFIG_H_ */\n");

	return (0);
}
