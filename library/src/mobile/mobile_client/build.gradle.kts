plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.compose")
}

// Optional CI signing configuration (ANDROID_KEYSTORE_BASE64 and friends).
val mattiSigning = rootProject.file("../../packaging/android/signing.gradle.kts")
if (mattiSigning.exists()) {
    apply(from = mattiSigning)
}

android {
    namespace = "dev.matticraft.mobile"
    compileSdk = 35

    defaultConfig {
        applicationId = "dev.matticraft.mobile"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = (project.findProperty("mattiVersion") as String?) ?: "0.1.0"
    }

    buildFeatures {
        compose = true
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
    implementation(project(":matti-kotlin"))
    implementation("androidx.activity:activity-compose:1.9.3")
    implementation("androidx.compose.ui:ui:1.7.5")
    implementation("androidx.compose.material3:material3:1.3.1")
}
