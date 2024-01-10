//
// Created by hj6231 on 2023/12/19.
//

#ifndef MY_APPLICATION_LOG_H
#define MY_APPLICATION_LOG_H
#include <android/log.h>
#define LOG_OUTPUT
#define COMMON_TAG
#ifdef LOG_OUTPUT

#ifdef COMMON_TAG
#define LOG_E(TAG, FMT, ...) __android_log_print(ANDROID_LOG_ERROR, "HJ", FMT, ##__VA_ARGS__)
#define LOG_W(TAG, FMT, ...) __android_log_print(ANDROID_LOG_WARN, "HJ", FMT, ##__VA_ARGS__)
#define LOG_D(TAG, FMT, ...) __android_log_print(ANDROID_LOG_DEBUG, "HJ", FMT, ##__VA_ARGS__)
#else

#define LOG_E(TAG, FMT, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, FMT, ##__VA_ARGS__)
#define LOG_W(TAG, FMT, ...) __android_log_print(ANDROID_LOG_WARN, TAG, FMT, ##__VA_ARGS__)
#define LOG_D(TAG, FMT, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, FMT, ##__VA_ARGS__)
#endif
#else

#define LOG_E(TAG, FMT, ...)
#define LOG_W(TAG, FMT, ...)
#define LOG_D(TAG, FMT, ...)
#endif

#endif //MY_APPLICATION_LOG_H
