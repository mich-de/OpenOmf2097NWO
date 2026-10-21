/* Emscripten stand-in for libepoxy's <epoxy/gl.h>.
 *
 * OpenOMF's OpenGL renderer only needs entry points and enums, no epoxy
 * runtime: it never asks epoxy which version is supported (the version is
 * probed with a real context in sdl_window.c). So this header just forwards
 * to OpenGL ES 3.0, which WebGL2 implements 1:1 in the browser.
 */
#ifndef OMF_EMSCRIPTEN_EPOXY_GL_H
#define OMF_EMSCRIPTEN_EPOXY_GL_H

#include <GLES3/gl3.h>
#include <stdbool.h> /* libepoxy's gl.h provides this transitively; shaders.h relies on it */

/* GLES3/gl3.h misses this sized internal format; WebGL2 supports it. */
#ifndef GL_RGBA16
#define GL_RGBA16 0x805B
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* glMultiDrawArrays is desktop GL 4.0 / OpenGL ES 3.2 and does not exist in
 * WebGL2. See epoxy_shim.c for the glDrawArrays-loop implementation. */
void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);

#ifdef __cplusplus
}
#endif

#endif
