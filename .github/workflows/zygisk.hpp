#pragma once
#include <jni.h>
#include <sys/types.h>

namespace zygisk {
    struct Api {
        int version;
        void (*pltHookRegister)(const char* regex, const char* symbol, void* newFunc, void** oldFunc);
        void (*pltHookExclude)(const char* regex, const char* symbol);
        void (*pltHookCommit)();
        bool (*exemptFd)(int fd);
    };
    extern Api* api;
    class ModuleBase {
    public:
        virtual void onLoad(void* env, void* api) = 0;
        virtual void preAppSpecialize(void* args) = 0;
        virtual void postAppSpecialize(void* args) = 0;
    };
}
