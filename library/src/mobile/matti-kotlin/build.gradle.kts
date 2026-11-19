plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "dev.matticraft.libmatti"
    compileSdk = 35
    ndkVersion = "27.0.12077973"

    defaultConfig {
        minSdk = 26
        externalNativeBuild {
            cmake {
                // The main CMake project is added as a subdirectory by the bridge
                // CMakeLists; LIBMATTI_ROOT points at the repository root.
                arguments += "-DLIBMATTI_ROOT=${rootProject.projectDir}/../.."
                arguments += "-DMATTI_MOBILE_ONLY=ON"
                arguments += "-DMATTI_BUILD_LAUNCHER=OFF"
                cppFlags += ""
            }
        }
        ndk {
            // 64-bit first; the emulator uses x86_64.
            abiFilters += listOf("arm64-v8a", "x86_64")
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
}
