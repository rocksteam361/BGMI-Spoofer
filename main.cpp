#include <jni.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include <sys/stat.h>
#include "zygisk.hpp"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PUBGSpoofer", __VA_ARGS__)

// Spoofed values (Change these to whatever you want)
const char* SPOOFED_MAC = "02:00:00:00:00:01";
const char* SPOOFED_SERIAL = "SPOOFED_SERIAL_123";
const char* SPOOFED_BOOT_ID = "12345678-1234-1234-1234-123456789012";

static int (*orig_openat)(int, const char*, int, ...) = nullptr;
static ssize_t (*orig_read)(int, void*, size_t) = nullptr;
static void (*orig_system_property_get)(const char*, char*) = nullptr;

int hooked_openat(int dirfd, const char* pathname, int flags, ...) {
    if (strstr(pathname, "/sys/class/net/wlan0/address") || 
        strstr(pathname, "/proc/sys/kernel/random/boot_id")) {
        return orig_openat(dirfd, "/dev/zero", flags);
    }
    return orig_openat(dirfd, pathname, flags);
}

ssize_t hooked_read(int fd, void* buf, size_t count) {
    ssize_t res = orig_read(fd, buf, count);
    if (res > 0) {
        char* cbuf = (char*)buf;
        if (count >= 17 && cbuf[2] == ':' && cbuf[5] == ':') {
            strncpy(cbuf, SPOOFED_MAC, 17);
        } else if (count >= 36 && cbuf[8] == '-' && cbuf[13] == '-') {
            strncpy(cbuf, SPOOFED_BOOT_ID, 36);
        }
    }
    return res;
}

void hooked_system_property_get(const char* name, char* value) {
    if (strcmp(name, "ro.serialno") == 0) {
        strcpy(value, SPOOFED_SERIAL);
    } else {
        orig_system_property_get(name, value);
    }
}

namespace zygisk { Api* api = nullptr; }

class SpooferModule : public zygisk::ModuleBase {
    bool is_target = false;

public:
    void onLoad(void* env, void* api_ptr) override {
        zygisk::api = static_cast<zygisk::Api*>(api_ptr);
    }
    
    void preAppSpecialize(void* env_ptr, zygisk::AppSpecializeArgs* args) override {
        is_target = false;
        JNIEnv* env = (JNIEnv*)env_ptr;
        
        if (args && args->nice_name) {
            const char* name = env->GetStringUTFChars(*(args->nice_name), nullptr);
            if (name) {
                // BGMI, Global, KR, JP, VN/TW
                if (strcmp(name, "com.pubg.imobile") == 0 ||
                    strcmp(name, "com.tencent.ig") == 0 ||
                    strcmp(name, "com.pubg.krmobile") == 0 ||
                    strcmp(name, "com.rekoo.pubgm") == 0 ||
                    strcmp(name, "com.vng.pubgmobile") == 0) {
                    is_target = true;
                    LOGD("PUBG/BGMI Detected: %s", name);
                }
                env->ReleaseStringUTFChars(*(args->nice_name), name);
            }
        }
    }

    void postAppSpecialize(void* env, zygisk::AppSpecializeArgs* args) override {
        if (is_target && zygisk::api) {
            LOGD("Applying HWID hooks.");
            zygisk::api->pltHookRegister(".*libc\\.so$", "openat", (void*)hooked_openat, (void**)&orig_openat);
            zygisk::api->pltHookRegister(".*libc\\.so$", "read", (void*)hooked_read, (void**)&orig_read);
            zygisk::api->pltHookRegister(".*libc\\.so$", "__system_property_get", (void*)hooked_system_property_get, (void**)&orig_system_property_get);
            zygisk::api->pltHookCommit();
        }
    }
};
REGISTER_ZYGISK_MODULE(SpooferModule)
