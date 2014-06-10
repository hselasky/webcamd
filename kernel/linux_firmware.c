/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

extern char global_fw_prefix[];

#define	FW_DECONST(ptr) ((void *)(long)(ptr))

struct firmware_cb_arg {
	const struct firmware *pfw;
	firmware_cb_t *pfunc;
	void   *ctx;
};

static int
request_firmware_nowait_task(void *arg)
{
	struct firmware_cb_arg *pfcbarg = arg;

	(pfcbarg->pfunc) (pfcbarg->pfw, pfcbarg->ctx);

	free(pfcbarg);

	return (0);
}

int
request_firmware_nowait(struct module *module, bool uevent,
    const char *name, struct device *device, gfp_t gfp, void *context,
    firmware_cb_t *pfunc)
{
	int error;
	struct firmware_cb_arg *pfcbarg;
	struct task_struct *task;

	if (pfunc == NULL)
		return (-EINVAL);

	pfcbarg = malloc(sizeof(*pfcbarg));
	if (pfcbarg == NULL)
		return (-ENOMEM);

	pfcbarg->pfw = NULL;
	pfcbarg->pfunc = pfunc;
	pfcbarg->ctx = context;

	error = request_firmware(&pfcbarg->pfw, name, device);
	if (error) {
		free(pfcbarg);
		return (error);
	}
	task = kthread_run(&request_firmware_nowait_task,
	    pfcbarg, "ASYNC FW LOAD");

	if (IS_ERR(task)) {
		release_firmware(pfcbarg->pfw);
		free(pfcbarg);
		return (PTR_ERR(task));
	}
	return (0);
}

int
request_firmware(const struct firmware **ppfw, const char *name,
    struct device *device)
{
	struct firmware *fw;
	ssize_t size;
	char path[256];
	int f;

	*ppfw = NULL;

	fw = malloc(sizeof(*fw));

	memset(fw, 0, sizeof(*fw));

	if (fw == NULL) {
		return (-ENOMEM);
	}
	snprintf(path, sizeof(path), "%s/%s", global_fw_prefix, name);

	f = open(path, O_RDONLY);

	printf("Loading firmware at '%s', f=%d\n", path, f);

	if (f < 0) {
		free(fw);
		return (-EINVAL);
	}
	size = lseek(f, 0, SEEK_END);
	if (size < 0) {
		free(fw);
		close(f);
		return (-EINVAL);
	}
	lseek(f, 0, SEEK_SET);

	fw->size = size;
	fw->data = malloc(size);

	if (fw->data == NULL) {
		free(fw);
		close(f);
		return (-ENOMEM);
	}
	if (read(f, FW_DECONST(fw->data), fw->size) != fw->size) {
		free(FW_DECONST(fw->data));
		free(fw);
		close(f);
		return (-EINVAL);
	}
	close(f);

	*ppfw = fw;

	return (0);
}

void
release_firmware(const struct firmware *fw)
{
	if (fw == NULL)
		return;

	if (fw->data != NULL)
		free(FW_DECONST(fw->data));

	free(FW_DECONST(fw));
}
