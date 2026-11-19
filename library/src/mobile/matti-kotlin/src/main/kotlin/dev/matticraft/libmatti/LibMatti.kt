package dev.matticraft.libmatti

import android.view.Surface

/**
 * Kotlin face of libmatti. The C library owns the window/render thread and the
 * whole UI; this object only starts it, hands over the Android surface and
 * forwards lifecycle/input.
 *
 * The same `libmatti` sources are compiled for Android; the platform and the
 * 32/64-bit pointer width are detected inside the library, no Kotlin flags.
 */
object LibMatti {
    init {
        System.loadLibrary("matti_mobile")
    }

    /** Default renderer preset from the C library (Android: sdl:vulkan on 64-bit). */
    external fun defaultSelection(): String

    /** Platform name from the C library ("android"). */
    external fun platformName(): String

    /** Runs the client loop; returns the process status. Blocks the caller. */
    external fun run(args: Array<String>): Int

    /** The SurfaceView surface, or null when it is destroyed. */
    external fun setSurface(surface: Surface?)

    /** Pixel size of the drawing surface. */
    external fun resize(width: Int, height: Int)

    /** Back button / controller menu. */
    external fun pressEscape()

    external fun shutdown()
}
