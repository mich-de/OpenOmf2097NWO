#include <epoxy/gl.h>

void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount) {
    for(GLsizei i = 0; i < drawcount; i++) {
        glDrawArrays(mode, first[i], count[i]);
    }
}
