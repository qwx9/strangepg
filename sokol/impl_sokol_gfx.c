#include "strpg.h"
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define	SOKOL_IMPL
#define	SOKOL_NO_ENTRY

#define GL_VIEWPORT 0x0BA2
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_TYPE_ERROR 0x824C
#define SG_GL_FUNCS_EXT	\
	_SG_XMACRO(glDebugMessageCallback,	void, (GLDEBUGPROC callback, const void * userParam)) \
	_SG_XMACRO(glGetTexImage,	void, (GLenum target, GLint level, GLenum format, GLenum type, void * pixels)) \
	_SG_XMACRO(glReadPixels,	void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)) \

#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
#include "lib/sokol_log.h"
#include "lib/sokol_glue.h"
#include "sokol_gfx_ext.h"
