#ifdef BUILD_DLL

#ifndef __DORFY_VISION__SHADER_H__
#define __DORFY_VISION__SHADER_H__

#include <string>
#include <gl/gl.h>
#include "Types.h"

using namespace std;

class shader {
private:
    GLuint id;
public:
    friend class shaderprogram;

    shader();
    shader(GLenum type, string file);
    bool compiled() const;
    void printlog() const;
    void destroy();
};

class shaderprogram {
private:
    GLuint id;
public:
    shaderprogram();
    void create();
    void attach(shader s);
    void detach(shader s);
    bool link();
    void apply() const;
    void printlog() const;
    void destroy();
    GLint getUniformLocation(char const * name) const;
    void passUniform4f(GLint id, float f0, float f1, float f2, float f3);
    void passUniform1f(GLint id, float f0);

    static shaderprogram const no_program;
};

#endif

#endif
