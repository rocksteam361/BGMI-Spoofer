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

    struct AppSpecializeArgs {
        jstring* nice_name;
        jstring* app_data_dir;
        uid_t* uid;
        gid_t* gid;
    };

    extern Api* api;
    class ModuleBase {
    public:
        virtual void onLoad(void* env, void* api) = 0;
        virtual void preAppSpecialize(void* env, AppSpecializeArgs* args) = 0;
        virtual void postAppSpecialize(void* env, AppSpecializeArgs* args) = 0;
    };
}
