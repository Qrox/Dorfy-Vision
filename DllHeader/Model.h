#ifdef BUILD_DLL

#ifndef __DORFY_VISION__MODEL_H__
#define __DORFY_VISION__MODEL_H__

#include <string>
#include "Types.h"

using namespace std;

enum glFunction : dword {
    vertex,
    normal,
    finish = ((dword) 0) - 1,
};

class Model {
    static float32 const no_size[3];
    byte * data, * end;

public:
    Model(char const * file);
    Model(string file);
    ~Model();

    float32 glRender() const;
    float32 const * size() const;
};

#endif

#endif
