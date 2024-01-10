package com.arcsoft.myapplication

import android.content.Context
import android.content.res.AssetManager
import android.view.Surface
import kotlin.concurrent.thread

class VulkanTutorial(context:Context) {
    private var mHandle : Long = 0

    init {
        mHandle = create(context.assets)
    }

    fun destroy() {
        destroy(mHandle)
    }

    fun surfaceCreated(surface: Surface) {
        surfaceCreated(mHandle, surface)
    }

    fun surfaceChanged() {
        surfaceChanged(mHandle)
    }

    fun surfaceDestroyed() {
        surfaceDestroyed(mHandle)
    }

    fun pause() {
        pause(mHandle)
    }
    fun resume() {
        resume(mHandle)
    }

    private external fun create(assetManager : AssetManager) : Long
    private external fun destroy(handle: Long)

    private external fun surfaceCreated(handle: Long, surface:Surface)
    private external fun surfaceChanged(handle: Long)
    private external fun surfaceDestroyed(handle: Long)
    private external fun pause(handle: Long)
    private external fun resume(handle: Long)
//    private external fun AOTCompilation(handle: Long, assetManager : AssetManager)

    companion object {
        init {
            System.loadLibrary("myapplication")
        }
    }
}