//
// Created by hj6231 on 2024/1/23.
//

#pragma once
#include <pthread.h>
#include <mutex>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>

class TutorialBase {
public:
    TutorialBase(AAssetManager * asset_manager);
    virtual ~TutorialBase() = default;
    void StartThread(ANativeWindow* window);
    void StopThread();
    virtual void Run() = 0;

    virtual void CreateInstance() = 0;
    virtual void DestroyInstance() = 0;
    virtual bool PickPhysicalDevice() = 0;

protected:
    AAssetManager * asset_manager_;
    ANativeWindow* window_;

    pthread_t thread_;
    std::mutex thread_state_mutex_;
    int thread_state_;
};


