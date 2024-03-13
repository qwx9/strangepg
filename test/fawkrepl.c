#include "strpg.h"
#include <bio.h>
#include "fs.h"

typedef struct Cmd Cmd;
struct Cmd{
	char *s;
	char *p;
	char *e;
	ssize n;
	int fd;
};

/* libfawk configuration */
//typedef ssize_t fawk_num_t;	/* doesn't work */
typedef double fawk_num_t;	/* many things internally are still doubles */
typedef ssize_t fawk_refco_t;
#define FAWK_API static
#define fawk_malloc(ctx,n)		malloc((n))
#define fawk_calloc(ctx,n,m)	mallocz((n)*(m), 1)
#define fawk_realloc(ctx,p,n)	realloc((p), (n))
#define fawk_free(ctx,p)		free(p)
/*#define FAWK_DISABLE_FAWK_PRINT*/

#include "libfawk_sc_fawk.c"

FAWK_API void libfawk_error(fawk_ctx_t *ctx, const char *str, const char *loc_fn, long loc_line, long loc_col)
{
	fprintf(stderr, "Error: %s\n", str);
}

static int getch1(fawk_ctx_t *ctx, fawk_src_t *src)
{
	FILE *f = ctx->parser.isp->user_data;
	return fgetc(f);
}

static int include1(fawk_ctx_t *ctx, fawk_src_t *src, int opening, fawk_src_t *from)
{
	if (opening) {
		FILE *f;
		if ((*src->fn != '/') && (from != NULL)) { /* calculate relative address from 'from' */
			int l1 = strlen(src->fn), l2 = strlen(from->fn);
			char *end, *fn = malloc(l1+l2+4);
			memcpy(fn, from->fn, l2+1);
			end = strrchr(fn, '/');
			if (end != NULL) {
				end++;
				memcpy(end, src->fn, l1+1);
				f = fopen(fn, "r");
			}
			else
				f = fopen(src->fn, "r");
			free(fn);
		}
		else
			f = fopen(src->fn, "r");
		src->user_data = f;
		if (f == NULL) {
			fprintf(stderr, "Can't find %s for include\n", src->fn);
			return -1;
		}
	}
	else
		fclose(src->user_data);

	return 0;
}

// FIXME: take a look at the full api


int main(int argc, char *argv[])
{
	FILE *fsc;
	fawk_ctx_t ctx;
	const char *script_fn;
	char *mainfn;

	/* open the script file */
	script_fn = argv[1];
	if (argc < 2) {
		fprintf(stderr, "need a script file name\n");
		return 1;
	}
	fsc = fopen(script_fn, "r");
	if (fsc == NULL) {
		fprintf(stderr, "Failed to open the script: %s\n", script_fn);
		return 1;
	}

	/* set up the script context */
	fawk_init(&ctx);
	mainfn = "main";

	ctx.parser.get_char = getch1;
	ctx.parser.include = include1;
	ctx.parser.isp->user_data = fsc;
	ctx.parser.isp->fn = fawk_strdup(&ctx, script_fn);

	if (fawk_parse_fawk(&ctx) != 0) {
		fclose(fsc);
		fawk_uninit(&ctx);
		return 1;
	}
	/* call main() - call1 prepares the stack */
	if (fawk_call1(&ctx, mainfn) == 0) {
		fawk_cell_t res;
		fawk_execret_t er;

		/* arguments could be pushed on the stack here */

		if (fawk_call2(&ctx, 0) == 0) { /* call2: perform call, with 0 arguments on stack */
			er = fawk_execute(&ctx, -1); /* execute the VM; will return once the function has finished */
			if (er == FAWK_ER_FIN) { /* read and free the return value of script's main() */
				fawk_pop(&ctx, &res);
				printf("\nResult: ");
				fawk_dump_cell(&res, 1);
				printf("\n");
				fawk_cell_free(&ctx, &res);
			}
		}
		else
			fprintf(stderr, "Failed to call main() [#2]\n");
	}
	else
		fprintf(stderr, "Failed to call main() [#1]\n");
	fsc = fopen("/fd/0", "r");
	if (fsc == NULL) {
		fprintf(stderr, "Failed to open the script: %s\n", "/fd/0");
		return 1;
	}
	mainfn = "bar";

	fawk_uninit(&ctx);

	return 0;
}
