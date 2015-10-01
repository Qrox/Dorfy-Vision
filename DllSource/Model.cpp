#ifdef BUILD_DLL

#include "Model.h"
#include <iostream>
typedef int64 off64_t;
#include <io.h>
#include <fcntl.h>
#include <gl/gl.h>
#include <windows.h>
#include "Dll.h"
#include "Types.h"

class datastream {
public:
    byte * begin, * end, * cur;

    datastream() {
        cur = begin = end = nullptr;
    }

    datastream(byte * b, byte * e) {
        if (b < e) {
            cur = begin = b;
            end = e;
        } else {
            cur = begin = end = nullptr;
        }
    }

    bool eos() {
        return cur >= end;
    }

    template <typename T>
    datastream & operator >> (T & v) {
        if ((ptrword) end - (ptrword) cur >= sizeof(T)) {
            v = *(T *) cur;
            cur += sizeof(T);
        } else {
//            v = T();
            cur = end;
        }
        return *this;
    }

    template <typename T>
    datastream & skip(ptrword cnt = 1) {
        ptrword sk = sizeof(T) * cnt;
        if ((ptrword) end - (ptrword) cur >= sk) {
            cur += sk;
        } else {
            cur = end;
        }
        return *this;
    }
};

float32 const Model::no_size[3] = {0, 0, 0};

Model::Model(char const * file) : Model(string(file)) {
}

Model::Model(string local) {
    string file = currentDir + local;
    int hfile = open(file.data(), _O_RDONLY | _O_BINARY);
    if (hfile == -1) {
        cout << "failed to load model file \"" << local << "\"!" << endl;
        data = nullptr;
        end = nullptr;
    } else {
        long length = filelength(hfile);
        if (length < 0 || length >= 0x10000) {
            cout << "model file \"" << file << "\" is too large!" << endl;
            data = nullptr;
            end = nullptr;
        } else {
            dword len = length;
            data = new byte[len];
            end = data + len;
            if ((dword) read(hfile, data, len) != len) {
                cout << "failed to complete loading model file \"" << file << "\"!" << endl;
                data = nullptr;
                end = nullptr;
            }
        }
        close(hfile);
    }
}

Model::~Model() {
    if (data) {
        delete [] data;
    }
}

float32 Model::glRender() const {
    datastream in(data, end);
    float32 height = 0;
    in.skip<float32>(2) >> height;
    while (!in.eos()) {
        word type = 0, size = 0;
        in >> type >> size;
        glBegin(type);
        glFunction func = finish;
        float vec[3] = {0, 0, 0};
        bool terminate = false;
        while (!terminate && !in.eos()) {
            in >> func;
            switch (func) {
            case vertex:
                in >> vec[0] >> vec[1] >> vec[2];
                glVertex3fv(vec);
                break;
            case normal:
                in >> vec[0] >> vec[1] >> vec[2];
                glNormal3fv(vec);
                break;
            case finish:
                if (!size) { // size = 0, end command mode
                    terminate = true;
                }
                break;
            }
            if (size && !--size) { // size != 0, array mode
                terminate = true;
            }
        }
        glEnd();
    }
    return height;

//    byte * data = this->data, * end = this->end;
//    if (data + sizeof(float32[3]) <= end) {
//        float32 height = ((float32 *) data)[2];
//        data += sizeof(float32[3]);
//        while (data + sizeof(word) * 2 <= end) {
//            word type = *(word *) data; data += sizeof(word);
//            word size = *(word *) data; data += sizeof(word);
//            bool terminatecommand = (size == 0);
//            if (terminatecommand) {
//                glBegin(type);
//                bool terminate = false;
//                while (!terminate && data + sizeof(glFunction) <= end) {
//                    glFunction func = *(glFunction *) data;
//                    data += sizeof(glFunction);
//                    switch (func) {
//                    case vertex:
//                        if (data + sizeof(float32) * 3 <= end) {
//                            glVertex3fv((float32 *) data);
//                            data += sizeof(float32[3]);
//                        } else {
//                            data = end;
//                        }
//                        break;
//                    case normal:
//                        if (data + sizeof(float32) * 3 <= end) {
//                            glNormal3fv((float32 *) data);
//                            data += sizeof(float32[3]);
//                        } else {
//                            data = end;
//                        }
//                        break;
//                    case finish:
//                        terminate = true;
//                        break;
//                    }
//                }
//                if (!terminate) {
//                    data = end;
//                }
//                glEnd();
//            } else {
//                glBegin(type);
//                while (size-- && data + sizeof(glFunction) <= end) {
//                    glFunction func = *(glFunction *) data;
//                    data += sizeof(glFunction);
//                    switch (func) {
//                    case vertex:
//                        if (data + sizeof(float32) * 3 <= end) {
//                            glVertex3fv((float32 *) data);
//                            data += sizeof(float32) * 3;
//                        }
//                        break;
//                    case normal:
//                        if (data + sizeof(float32) * 3 <= end) {
//                            glNormal3fv((float32 *) data);
//                            data += sizeof(float32) * 3;
//                        }
//                        break;
//                    case end:
//                        break;
//                    }
//                }
//                glEnd();
//            }
//        }
//        return height;
//    } else {
//        return 0;
//    }
}

float const * Model::size() const {
    if (data + sizeof(float[3]) <= end) {
        return (float *) data;
    } else {
        return no_size;
    }
}

#endif
