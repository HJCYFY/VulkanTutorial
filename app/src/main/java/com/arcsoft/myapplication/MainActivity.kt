package com.arcsoft.myapplication

import android.content.Context
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.SurfaceHolder
import com.arcsoft.myapplication.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var vulkanTutorial : VulkanTutorial

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        vulkanTutorial = VulkanTutorial(this)
        // Example of a call to a native method
        val surfaceCallback = SurfaceCallback(vulkanTutorial)
        binding.surfaceView.holder.addCallback(surfaceCallback)
    }

    override fun onDestroy() {
        super.onDestroy()
        vulkanTutorial.destroy()
    }

    override fun onResume() {
        super.onResume()
        vulkanTutorial.resume()
    }

    override fun onPause() {
        super.onPause()
        vulkanTutorial.pause()
    }


    class SurfaceCallback(private val vulkanTutorial:VulkanTutorial) : SurfaceHolder.Callback {
        override fun surfaceCreated(holder: SurfaceHolder) {
            vulkanTutorial.surfaceCreated(holder.surface)
        }

        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
            vulkanTutorial.surfaceChanged()
        }

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            vulkanTutorial.surfaceDestroyed()
        }
    }
}