package dev.matticraft.mobile

import android.content.Intent
import android.content.pm.ActivityInfo
import android.os.Build
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import androidx.activity.ComponentActivity
import androidx.activity.OnBackPressedCallback
import dev.matticraft.libmatti.LibMatti
import kotlin.concurrent.thread

/**
 * The game itself is landscape; the launcher stays portrait. The native renderer
 * draws into this SurfaceView through the JNI surface handoff, so Android no
 * longer tries to create a desktop GLFW window (which returned status 1).
 */
class GameActivity : ComponentActivity(), SurfaceHolder.Callback {
    companion object {
        const val EXTRA_ARGS = "matticraft.game.args"
        const val EXTRA_EXIT_CODE = "matticraft.game.exit_code"
        const val EXTRA_KEEP_SCREEN_ON = "matticraft.game.keep_screen_on"
    }

    private lateinit var surfaceView: SurfaceView
    private var gameThread: Thread? = null
    private var started = false
    private var finished = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        if (intent.getBooleanExtra(EXTRA_KEEP_SCREEN_ON, true)) {
            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        }
        hideSystemBars()

        surfaceView = SurfaceView(this)
        surfaceView.holder.addCallback(this)
        setContentView(surfaceView)

        onBackPressedDispatcher.addCallback(
            this,
            object : OnBackPressedCallback(true) {
                override fun handleOnBackPressed() {
                    // Ask the native client to close cleanly; it returns through
                    // the same exit-code path as the window close button.
                    LibMatti.shutdown()
                }
            },
        )
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        LibMatti.setSurface(holder.surface)
        startGame()
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        LibMatti.resize(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        LibMatti.shutdown()
        gameThread?.join(1500)
        LibMatti.setSurface(null)
    }

    private fun startGame() {
        if (started) return
        started = true
        val args = intent.getStringArrayListExtra(EXTRA_ARGS)?.toTypedArray() ?: emptyArray()
        gameThread = thread(name = "matticraft-game") {
            val code = runCatching { LibMatti.run(args) }.getOrElse { 1 }
            runOnUiThread { finishWithCode(code) }
        }
    }

    private fun finishWithCode(code: Int) {
        if (finished) return
        finished = true
        setResult(RESULT_OK, Intent().putExtra(EXTRA_EXIT_CODE, code))
        finish()
    }

    private fun hideSystemBars() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.systemBars())
                controller.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    or android.view.View.SYSTEM_UI_FLAG_FULLSCREEN
                    or android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    or android.view.View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    or android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    or android.view.View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                )
        }
    }

    override fun onDestroy() {
        LibMatti.shutdown()
        window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        super.onDestroy()
    }
}
