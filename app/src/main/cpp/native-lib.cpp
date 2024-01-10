#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "tutorial.h"
#include "log.h"

extern "C" JNIEXPORT jlong JNICALL
Java_com_arcsoft_myapplication_VulkanTutorial_create (
        JNIEnv* env,
        jobject,
        jobject asset_manager) {
    AAssetManager * a_asset_manager = AAssetManager_fromJava(env, asset_manager);
    auto* tutorial = new Tutorial(a_asset_manager);
    tutorial->CreateInstance();
    tutorial->pickPhysicalDevice();
    return  (jlong)tutorial;
}

extern "C" JNIEXPORT void JNICALL
Java_com_arcsoft_myapplication_VulkanTutorial_destroy (
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    (void)env;
    auto* tutorial = (Tutorial*)handle;
    tutorial->DestroyInstance();
    delete tutorial;
}

extern "C" JNIEXPORT void JNICALL
Java_com_arcsoft_myapplication_VulkanTutorial_surfaceCreated (
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jobject surface) {
    (void)env;
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        return;
    }

    auto* tutorial = (Tutorial*)handle;
    tutorial->StartThread(window);
}
extern "C" JNIEXPORT void JNICALL
Java_com_arcsoft_myapplication_VulkanTutorial_surfaceChanged (
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    (void)env;
}

extern "C" JNIEXPORT void JNICALL
Java_com_arcsoft_myapplication_VulkanTutorial_surfaceDestroyed (
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    (void)env;
    auto* tutorial = (Tutorial*)handle;
    tutorial->StopThread();
}

extern "C" JNIEXPORT void JNICALL
Java_com_arcsoft_myapplication_VulkanTutorial_pause (
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    (void)env;

}

extern "C" JNIEXPORT void JNICALL
Java_com_arcsoft_myapplication_VulkanTutorial_resume (
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    (void)env;
}

//
//extern "C" JNIEXPORT void JNICALL
//Java_com_arcsoft_myapplication_VulkanTutorial_AOTCompilation(JNIEnv* env,
//                                                             jobject /* this */,
//                                                             jlong handle,
//                                                             jobject asset_manager) {
//    AAssetManager * aAssetManager = AAssetManager_fromJava(env, asset_manager);
//    Tutorial* tutorial = (Tutorial*)handle;
//    tutorial->TestAOTCompilation(aAssetManager);
//}