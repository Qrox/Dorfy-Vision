#ifdef BUILD_DLL

#include "gl/glew.h"
#include "Shader.h"
typedef int64 off64_t;
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include "Dll.h"

shader::shader() : id(0) {}

shader::shader(GLenum type, string file) {
    cout << "loading shader file \"" << file << "\"" << endl;
    file = currentDir + file;
    int hfile = open(file.data(), _O_RDONLY | _O_BINARY);
    id = 0;
    if (hfile == -1) {
        cout << "failed to load shader file \"" << file << "\"!" << endl;
    } else {
        long length = filelength(hfile);
        if (length < 0 || length > 0x7FFFFFFF) {
            cout << "failed to load shader file \"" << file << "\"!" << endl;
        } else {
            GLint len = length;
            GLchar * data = new GLchar[len];
            if (read(hfile, data, len) != len) {
                cout << "failed to complete loading shader file \"" << file << "\"!" << endl;
            } else {
                id = glCreateShader(type);
                glShaderSource(id, 1, &data, &len);
                glCompileShader(id);
                GLint compiled = 0;
                glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
                if (!compiled) {
                    cout << "failed to compile shader!" << endl
                         << "compiler logs:" << endl;
                    printlog();
                    glDeleteShader(id);
                    id = 0;
                }
#ifdef DEBUG
                else {
                    cout << "compiler logs:" << endl;
                    printlog();
                }
#endif
            }
            delete [] data;
        }
        close(hfile);
    }
}

bool shader::compiled() const {
    return id != 0;
}

void shader::printlog() const {
    GLint info_len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 0) {
        char * log = new char[info_len];
        glGetShaderInfoLog(id, info_len, nullptr, log);
        cout << log << endl;
        delete [] log;
    }
}

void shader::destroy() {
    if (id != 0) {
        glDeleteShader(id);
        id = 0;
    }
}

shaderprogram::shaderprogram() : id(0) {}

void shaderprogram::create() {
    id = glCreateProgram();
}

void shaderprogram::attach(shader s) {
    if (s.id != 0) {
        glAttachShader(id, s.id);
    }
}

void shaderprogram::detach(shader s) {
    if (s.id != 0) {
        glDetachShader(id, s.id);
    }
}

bool shaderprogram::link() {
    glLinkProgram(id);
    GLint linked = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &linked);
    if (!linked) {
        cout << "failed to link shader program!" << endl
             << "linker logs:" << endl;
        printlog();
        return false;
    }
#ifdef DEBUG
    else {
        cout << "linker logs:" << endl;
        printlog();
    }
#endif
    return true;
}

void shaderprogram::apply() const {
    glUseProgram(id);
}

void shaderprogram::printlog() const {
    GLint info_len;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 0) {
        char * log = new char[info_len];
        glGetProgramInfoLog(id, info_len, nullptr, log);
        cout << log << endl;
        delete [] log;
    }
}

void shaderprogram::destroy() {
    glDeleteProgram(id);
    id = 0;
}

GLint shaderprogram::getUniformLocation(char const * name) const {
    GLint loc = glGetUniformLocation(id, name);
    if (loc == -1) {
        cout << "can't find uniform named " << name << endl;
    }
    return loc;
}

void shaderprogram::passUniform4f(GLint uni, float f0, float f1, float f2, float f3) {
    if (uni != -1) {
        glUniform4f(uni, f0, f1, f2, f3);
    }
}

void shaderprogram::passUniform1f(GLint uni, float f0) {
    if (uni != -1) {
        glUniform1f(uni, f0);
    }
}

shaderprogram const shaderprogram::no_program;

#endif
