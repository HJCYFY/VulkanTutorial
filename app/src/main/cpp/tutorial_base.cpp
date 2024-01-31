//
// Created by hj6231 on 2024/1/23.
//
#include "tutorial_base.h"


void* thread_run(void* param) {
    auto* tutorial = (TutorialBase*)param;
    tutorial->Run();
    return nullptr;
}

TutorialBase::TutorialBase(AAssetManager * asset_manager) :
        asset_manager_(asset_manager),
        thread_(0),
        thread_state_(0),
        window_(nullptr) {

}

void TutorialBase::StartThread(ANativeWindow* window) {
    window_  = window;
    std::unique_lock<std::mutex> lock(thread_state_mutex_);
    if (thread_state_ == 0) {
        thread_state_ = 1;
        pthread_create(&thread_, nullptr, thread_run, this);
    }
}

void TutorialBase::StopThread() {
    std::unique_lock<std::mutex> lock(thread_state_mutex_);
    thread_state_ = 0;
    lock.unlock();
    void* ret = nullptr;
    pthread_join(thread_, &ret);
}