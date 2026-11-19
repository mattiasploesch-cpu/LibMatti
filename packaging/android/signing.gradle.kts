// Optional release signing for CI. The secrets are injected as environment
// variables by .github/workflows/portable-release.yml; without them the build
// stays unsigned and the workflow marks the artifacts as unsigned.
import java.util.Base64

val keystoreBase64 = System.getenv("ANDROID_KEYSTORE_BASE64")
val keystoreFile = layout.buildDirectory.file("matticraft-release.keystore").get().asFile

android {
    signingConfigs {
        if (!keystoreBase64.isNullOrEmpty()) {
            create("release") {
                storeFile = keystoreFile.also { file ->
                    file.parentFile.mkdirs()
                    file.writeBytes(Base64.getDecoder().decode(keystoreBase64))
                }
                storePassword = System.getenv("ANDROID_KEYSTORE_PASSWORD")
                keyAlias = System.getenv("ANDROID_KEY_ALIAS")
                keyPassword = System.getenv("ANDROID_KEY_PASSWORD")
            }
        }
    }
    buildTypes {
        if (!keystoreBase64.isNullOrEmpty()) {
            getByName("release") {
                signingConfig = signingConfigs.getByName("release")
                isMinifyEnabled = false
            }
        }
    }
}
