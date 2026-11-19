package dev.matticraft.mobile

import android.app.Activity
import android.content.Context
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Divider
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Tab
import androidx.compose.material3.TabRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import dev.matticraft.libmatti.LibMatti
import java.io.File
import java.security.MessageDigest
import kotlin.concurrent.thread
import org.json.JSONArray
import org.json.JSONObject

private val ANDROID_SELECTIONS = listOf("sdl:vulkan", "sdl:opengl", "sdl:auto")

private data class LauncherProfile(
    val name: String = "Default",
    val gameVersion: String = "1.21.11",
    val selection: String = defaultSelection(),
    val username: String = "Player",
    val width: Int = 1280,
    val height: Int = 720,
    val fullscreen: Boolean = true,
) {
    fun slug(): String = name.lowercase()
        .replace(Regex("[^a-z0-9]+"), "-")
        .trim('-')
        .ifEmpty { "default" }

    fun instanceDir(context: Context): File =
        File(context.filesDir, "instances/${slug()}")

    fun modsDir(context: Context): File = File(instanceDir(context), "mods")
    fun logsDir(context: Context): File = File(instanceDir(context), "logs")
    fun crashDir(context: Context): File = File(instanceDir(context), "crash-reports")

    fun layer(): String = selection.substringBefore(':', "auto").ifEmpty { "auto" }
    fun api(): String = selection.substringAfter(':', "auto").ifEmpty { "auto" }
}

private fun defaultSelection(): String = runCatching { LibMatti.defaultSelection() }
    .getOrDefault("sdl:auto")

private fun platformLabel(): String = runCatching { LibMatti.platformName() }
    .getOrDefault("android")

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme {
                Surface(modifier = Modifier.fillMaxSize()) {
                    Launcher()
                }
            }
        }
    }

    @Composable
    private fun Launcher() {
        val context = this
        var profile by remember { mutableStateOf(loadProfile(context)) }
        var tab by remember { mutableStateOf(0) }
        var selectionMenu by remember { mutableStateOf(false) }
        var status by remember { mutableStateOf("Ready") }
        var running by remember { mutableStateOf(false) }
        var mods by remember { mutableStateOf(emptyList<ModInfo>()) }
        var issues by remember { mutableStateOf(emptyList<String>()) }
        var logText by remember { mutableStateOf("") }
        var crashes by remember { mutableStateOf(emptyList<File>()) }
        var crashText by remember { mutableStateOf("") }

        fun refresh() {
            profile.instanceDir(context).mkdirs()
            mods = scanMods(profile.modsDir(context))
            issues = validateManifest(profile.modsDir(context), mods)
            logText = tailFile(File(profile.logsDir(context), "latest.log"), 250)
            crashes = profile.crashDir(context).listFiles()
                ?.filter { it.isFile && it.name.endsWith(".txt") }
                ?.sortedByDescending { it.name }
                ?.toList()
                .orEmpty()
        }

        fun launch() {
            saveProfile(context, profile)
            running = true
            status = "Starting ${profile.selection}"
            thread(name = "matticraft-client") {
                val args = buildList {
                    add("--layer"); add(profile.layer())
                    add("--api"); add(profile.api())
                    add("--width"); add(profile.width.toString())
                    add("--height"); add(profile.height.toString())
                    add("--username"); add(profile.username)
                    if (profile.fullscreen) add("--fullscreen")
                }
                val code = runCatching { LibMatti.run(args.toTypedArray()) }
                    .getOrElse { error ->
                        (context as Activity).runOnUiThread {
                            status = "Could not start: ${error.message}"
                        }
                        -1
                    }
                (context as Activity).runOnUiThread {
                    running = false
                    status = "Game exited with $code"
                    refresh()
                }
            }
        }

        LaunchedEffect(Unit) { refresh() }
        LaunchedEffect(profile) {
            saveProfile(context, profile)
            refresh()
        }

        Column(modifier = Modifier.fillMaxSize().padding(18.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        "MATTICRAFT",
                        fontSize = 26.sp,
                        fontWeight = FontWeight.Bold,
                        color = MaterialTheme.colorScheme.primary,
                    )
                    Text(
                        "Launcher · ${platformLabel()} · ${profile.selection}",
                        fontSize = 11.sp,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                    )
                }
                Text(status, fontSize = 11.sp)
            }

            Spacer(Modifier.height(12.dp))
            TabRow(selectedTabIndex = tab) {
                listOf("Profile", "Mods", "Logs").forEachIndexed { index, label ->
                    Tab(
                        selected = tab == index,
                        onClick = { tab = index; refresh() },
                        text = { Text(label) },
                    )
                }
            }
            Spacer(Modifier.height(12.dp))

            when (tab) {
                0 -> ProfileTab(
                    profile = profile,
                    running = running,
                    selectionMenu = selectionMenu,
                    onSelectionMenu = { selectionMenu = it },
                    onChange = { profile = it },
                    onPlay = { launch() },
                )
                1 -> ModsTab(
                    profile = profile,
                    mods = mods,
                    issues = issues,
                    onRefresh = { refresh() },
                    onWriteManifest = {
                        writeManifest(profile.modsDir(context), mods)
                        refresh()
                        status = "mods.json written"
                    },
                )
                else -> LogsTab(
                    logText = logText,
                    crashes = crashes,
                    crashText = crashText,
                    onRefresh = { refresh() },
                    onSelect = { file ->
                        crashText = runCatching { file.readText() }.getOrDefault("")
                        status = file.name
                    },
                )
            }
        }
    }

    @Composable
    private fun ProfileTab(
        profile: LauncherProfile,
        running: Boolean,
        selectionMenu: Boolean,
        onSelectionMenu: (Boolean) -> Unit,
        onChange: (LauncherProfile) -> Unit,
        onPlay: () -> Unit,
    ) {
        Column(
            modifier = Modifier.fillMaxSize().verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Card {
                Column(Modifier.padding(14.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
                    OutlinedTextField(
                        value = profile.name,
                        onValueChange = { onChange(profile.copy(name = it)) },
                        label = { Text("Profile name") },
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth(),
                    )
                    OutlinedTextField(
                        value = profile.gameVersion,
                        onValueChange = { onChange(profile.copy(gameVersion = it)) },
                        label = { Text("Game version") },
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth(),
                    )
                    OutlinedTextField(
                        value = profile.username,
                        onValueChange = { onChange(profile.copy(username = it)) },
                        label = { Text("Offline player name") },
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth(),
                    )

                    Box {
                        OutlinedButton(
                            onClick = { onSelectionMenu(true) },
                            modifier = Modifier.fillMaxWidth(),
                        ) {
                            Text("Renderer: ${profile.selection}")
                        }
                        DropdownMenu(
                            expanded = selectionMenu,
                            onDismissRequest = { onSelectionMenu(false) },
                        ) {
                            ANDROID_SELECTIONS.forEach { selection ->
                                DropdownMenuItem(
                                    text = { Text(selection) },
                                    onClick = {
                                        onChange(profile.copy(selection = selection))
                                        onSelectionMenu(false)
                                    },
                                )
                            }
                        }
                    }

                    Row(
                        horizontalArrangement = Arrangement.spacedBy(10.dp),
                        verticalAlignment = Alignment.CenterVertically,
                    ) {
                        OutlinedTextField(
                            value = profile.width.toString(),
                            onValueChange = {
                                it.toIntOrNull()?.let { value ->
                                    onChange(profile.copy(width = value.coerceIn(320, 7680)))
                                }
                            },
                            label = { Text("Width") },
                            singleLine = true,
                            modifier = Modifier.weight(1f),
                        )
                        OutlinedTextField(
                            value = profile.height.toString(),
                            onValueChange = {
                                it.toIntOrNull()?.let { value ->
                                    onChange(profile.copy(height = value.coerceIn(240, 4320)))
                                }
                            },
                            label = { Text("Height") },
                            singleLine = true,
                            modifier = Modifier.weight(1f),
                        )
                    }
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text("Fullscreen", modifier = Modifier.weight(1f))
                        Switch(
                            checked = profile.fullscreen,
                            onCheckedChange = { onChange(profile.copy(fullscreen = it)) },
                        )
                    }
                }
            }

            Button(
                onClick = onPlay,
                enabled = !running,
                modifier = Modifier.fillMaxWidth().height(54.dp),
            ) {
                Text(if (running) "Running…" else "▶  Play", fontSize = 18.sp)
            }

            Text(
                "Instance: ${profile.instanceDir(this@MainActivity).absolutePath}",
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
            )
            Text(
                "Renderer preset comes from libmatti: ${defaultSelection()}",
                fontSize = 10.sp,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }
    }

    @Composable
    private fun ModsTab(
        profile: LauncherProfile,
        mods: List<ModInfo>,
        issues: List<String>,
        onRefresh: () -> Unit,
        onWriteManifest: () -> Unit,
    ) {
        Column(Modifier.fillMaxSize(), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedButton(onClick = onRefresh) { Text("Rescan") }
                OutlinedButton(onClick = onWriteManifest) { Text("Write mods.json") }
            }
            Text(profile.modsDir(this@MainActivity).absolutePath, fontSize = 10.sp, fontFamily = FontFamily.Monospace)
            if (issues.isNotEmpty()) {
                Card(colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.errorContainer)) {
                    Column(Modifier.padding(10.dp), verticalArrangement = Arrangement.spacedBy(2.dp)) {
                        Text("Manifest problems", fontWeight = FontWeight.Bold)
                        issues.forEach { Text(it, fontSize = 11.sp, fontFamily = FontFamily.Monospace) }
                    }
                }
            }
            LazyColumn(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                items(mods) { mod ->
                    Card {
                        Column(Modifier.padding(10.dp)) {
                            Text(mod.name, fontWeight = FontWeight.Bold)
                            Text(
                                "${mod.size} B · ${mod.sha256.take(16)}…",
                                fontSize = 10.sp,
                                fontFamily = FontFamily.Monospace,
                            )
                        }
                    }
                }
            }
        }
    }

    @Composable
    private fun LogsTab(
        logText: String,
        crashes: List<File>,
        crashText: String,
        onRefresh: () -> Unit,
        onSelect: (File) -> Unit,
    ) {
        Column(Modifier.fillMaxSize(), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedButton(onClick = onRefresh) { Text("Refresh") }
            }
            Text("latest.log", fontWeight = FontWeight.Bold)
            Card(Modifier.fillMaxWidth().weight(1f)) {
                Text(
                    logText.ifEmpty { "(no log yet)" },
                    modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState()),
                    fontSize = 9.sp,
                    fontFamily = FontFamily.Monospace,
                )
            }
            Divider()
            Text("Crash reports", fontWeight = FontWeight.Bold)
            LazyColumn(Modifier.height(90.dp), verticalArrangement = Arrangement.spacedBy(2.dp)) {
                items(crashes) { file ->
                    TextButton(onClick = { onSelect(file) }) {
                        Text(file.name, fontSize = 11.sp, fontFamily = FontFamily.Monospace)
                    }
                }
            }
            if (crashText.isNotEmpty()) {
                Card(Modifier.fillMaxWidth().height(140.dp)) {
                    Text(
                        crashText,
                        modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState()),
                        fontSize = 9.sp,
                        fontFamily = FontFamily.Monospace,
                    )
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Profile and mod storage (the JSON shape matches launcher-core's profile.json)
// ---------------------------------------------------------------------------

private fun profileFile(context: Context): File =
    File(context.filesDir, "launcher/profile.json")

private fun loadProfile(context: Context): LauncherProfile {
    val file = profileFile(context)
    if (!file.exists()) return LauncherProfile()
    return runCatching {
        val json = JSONObject(file.readText())
        LauncherProfile(
            name = json.optString("name", "Default"),
            gameVersion = json.optString("game_version", "1.21.11"),
            selection = json.optString("selection", defaultSelection()),
            username = json.optString("username", "Player"),
            width = json.optInt("width", 1280),
            height = json.optInt("height", 720),
            fullscreen = json.optBoolean("fullscreen", true),
        )
    }.getOrDefault(LauncherProfile())
}

private fun saveProfile(context: Context, profile: LauncherProfile) {
    val instance = profile.instanceDir(context)
    val mods = profile.modsDir(context)
    val logs = profile.logsDir(context)
    listOf(instance, mods, logs, profile.crashDir(context)).forEach { it.mkdirs() }
    val json = JSONObject()
        .put("schema", 1)
        .put("id", profile.slug())
        .put("name", profile.name)
        .put("game_version", profile.gameVersion)
        .put("selection", profile.selection)
        .put("username", profile.username)
        .put("instance_dir", instance.absolutePath)
        .put("game_dir", instance.absolutePath)
        .put("mods_dir", mods.absolutePath)
        .put("logs_dir", logs.absolutePath)
        .put("memory_mb", 2048)
        .put("fullscreen", profile.fullscreen)
        .put("width", profile.width)
        .put("height", profile.height)
        .put("extra_args", JSONArray())
    profileFile(context).parentFile?.mkdirs()
    profileFile(context).writeText(json.toString(2))
    File(instance, "profile.json").writeText(json.toString(2))
}

private data class ModInfo(val name: String, val size: Long, val sha256: String)

private fun scanMods(dir: File): List<ModInfo> {
    dir.mkdirs()
    return dir.listFiles()
        ?.filter {
            it.isFile && it.extension.lowercase() in setOf("so", "dll", "dylib", "matti", "jar")
        }
        ?.sortedBy { it.name }
        ?.map { ModInfo(it.name, it.length(), sha256(it)) }
        .orEmpty()
}

private fun sha256(file: File): String {
    val digest = MessageDigest.getInstance("SHA-256")
    file.inputStream().use { input ->
        val buffer = ByteArray(64 * 1024)
        while (true) {
            val read = input.read(buffer)
            if (read <= 0) break
            digest.update(buffer, 0, read)
        }
    }
    return digest.digest().joinToString("") { "%02x".format(it) }
}

private fun writeManifest(dir: File, mods: List<ModInfo>) {
    val entries = JSONArray()
    mods.forEach { mod ->
        val id = mod.name.substringBeforeLast('.')
        entries.put(
            JSONObject()
                .put("id", id)
                .put("name", id)
                .put("version", "")
                .put("file", mod.name)
                .put("sha256", mod.sha256)
                .put("required", true),
        )
    }
    JSONObject().put("schema", 1).put("mods", entries)
        .let { File(dir, "mods.json").writeText(it.toString(2)) }
}

private fun validateManifest(dir: File, mods: List<ModInfo>): List<String> {
    val manifestFile = File(dir, "mods.json")
    if (!manifestFile.exists()) return emptyList()
    return runCatching {
        val root = JSONObject(manifestFile.readText())
        val entries = root.optJSONArray("mods") ?: JSONArray()
        val installed = mods.associateBy { it.name }
        val listed = mutableSetOf<String>()
        val issues = mutableListOf<String>()
        for (index in 0 until entries.length()) {
            val entry = entries.getJSONObject(index)
            val name = entry.optString("file")
            listed.add(name)
            val expected = entry.optString("sha256")
            val actual = installed[name]
            when {
                actual == null -> issues.add("missing: $name")
                !actual.sha256.equals(expected, ignoreCase = true) ->
                    issues.add("changed: $name")
            }
        }
        mods.map { it.name }.filter { it !in listed }
            .forEach { issues.add("unlisted: $it") }
        issues
    }.getOrDefault(listOf("could not read mods.json"))
}

private fun tailFile(file: File, maxLines: Int): String {
    if (!file.exists()) return ""
    return runCatching {
        val lines = file.readLines()
        lines.takeLast(maxLines).joinToString("\n")
    }.getOrDefault("")
}
