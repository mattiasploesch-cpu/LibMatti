package dev.matticraft.mobile

import android.content.Context
import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
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
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import dev.matticraft.libmatti.LibMatti
import java.io.File
import java.security.MessageDigest
import org.json.JSONArray
import org.json.JSONObject

private val ANDROID_SELECTIONS = listOf("native:opengl", "native:auto")

private val MatticraftGreen = Color(0xFF9BE15D)
private val MatticraftGreenDark = Color(0xFF17300F)
private val MatticraftBackground = Color(0xFF0C110D)
private val MatticraftSurface = Color(0xFF171F18)
private val MatticraftSurfaceHigh = Color(0xFF223026)
private val MatticraftInk = Color(0xFFE8F2E4)
private val MatticraftMuted = Color(0xFFA9BBAA)
private val MatticraftBlue = Color(0xFF7FB7FF)

private val MatticraftColors = darkColorScheme(
    primary = MatticraftGreen,
    onPrimary = MatticraftGreenDark,
    primaryContainer = MatticraftGreenDark,
    onPrimaryContainer = MatticraftGreen,
    secondary = MatticraftBlue,
    onSecondary = Color(0xFF06172D),
    background = MatticraftBackground,
    onBackground = MatticraftInk,
    surface = MatticraftSurface,
    onSurface = MatticraftInk,
    surfaceVariant = MatticraftSurfaceHigh,
    onSurfaceVariant = MatticraftMuted,
    outline = Color(0xFF4C6552),
    error = Color(0xFFFFB4AB),
    onError = Color(0xFF690005),
    errorContainer = Color(0xFF93000A),
    onErrorContainer = Color(0xFFFFDAD6),
)

@Composable
private fun MatticraftTheme(content: @Composable () -> Unit) {
    MaterialTheme(colorScheme = MatticraftColors, content = content)
}

private data class LauncherProfile(
    val name: String = "Default",
    val gameVersion: String = "1.21.11",
    val selection: String = defaultSelection(),
    val username: String = "Player",
    val width: Int = 1280,
    val height: Int = 720,
    val fullscreen: Boolean = true,
    val keepScreenOn: Boolean = true,
    val extraArgs: String = "",
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
    .getOrDefault("native:opengl")

private fun platformLabel(): String = runCatching { LibMatti.platformName() }
    .getOrDefault("android")

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MatticraftTheme {
                Surface(modifier = Modifier.fillMaxSize()) {
                    Launcher()
                }
            }
        }
    }

    @OptIn(ExperimentalMaterial3Api::class)
    @Composable
    private fun Launcher() {
        val context = this
        var profile by remember { mutableStateOf(loadProfile(context)) }
        var tab by remember { mutableStateOf(0) }
        var selectionMenu by remember { mutableStateOf(false) }
        var resolutionMenu by remember { mutableStateOf(false) }
        var status by remember { mutableStateOf("Ready") }
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

        val gameLauncher = rememberLauncherForActivityResult(
            ActivityResultContracts.StartActivityForResult(),
        ) { result ->
            val code = result.data?.getIntExtra(GameActivity.EXTRA_EXIT_CODE, 0) ?: 0
            status = if (code == 0) "Game closed" else "Game exited with $code"
            refresh()
        }

        fun startGame() {
            if (profile.name.isBlank()) {
                status = "Profile name is empty"
                return
            }
            saveProfile(context, profile)
            status = "Starting ${profile.selection}"
            val args = buildList {
                add("--no-tests")
                add("--gameDir")
                add(profile.instanceDir(context).absolutePath)
                add("--version")
                add(profile.gameVersion)
                add("--username")
                add(profile.username.ifBlank { "Player" })
                add("--layer")
                add(profile.layer())
                add("--api")
                add(profile.api())
                add("--width")
                add(profile.width.toString())
                add("--height")
                add(profile.height.toString())
                if (profile.fullscreen) add("--fullscreen")
                profile.extraArgs.trim()
                    .split(Regex("\\s+"))
                    .filter { it.isNotEmpty() }
                    .forEach { add(it) }
            }
            val intent = Intent(context, GameActivity::class.java).apply {
                putStringArrayListExtra(GameActivity.EXTRA_ARGS, ArrayList(args))
                putExtra(GameActivity.EXTRA_KEEP_SCREEN_ON, profile.keepScreenOn)
            }
            gameLauncher.launch(intent)
        }

        LaunchedEffect(Unit) { refresh() }
        LaunchedEffect(profile) {
            saveProfile(context, profile)
            refresh()
        }

        Scaffold(
            containerColor = MaterialTheme.colorScheme.background,
            topBar = {
                TopAppBar(
                    title = {
                        Column {
                            Text(
                                "MATTICRAFT",
                                fontSize = 22.sp,
                                fontWeight = FontWeight.Black,
                                letterSpacing = 2.sp,
                                color = MaterialTheme.colorScheme.primary,
                            )
                            Text(
                                "${platformLabel()} · ${profile.selection}",
                                fontSize = 11.sp,
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                            )
                        }
                    },
                    colors = TopAppBarDefaults.topAppBarColors(
                        containerColor = MaterialTheme.colorScheme.background,
                    ),
                )
            },
            bottomBar = {
                NavigationBar(containerColor = MaterialTheme.colorScheme.surface) {
                    NavigationBarItem(
                        selected = tab == 0,
                        onClick = { tab = 0 },
                        icon = { Text("▶", color = if (tab == 0) MatticraftGreen else MatticraftMuted) },
                        label = { Text("Play") },
                    )
                    NavigationBarItem(
                        selected = tab == 1,
                        onClick = { tab = 1; refresh() },
                        icon = { Text("⚙", color = if (tab == 1) MatticraftGreen else MatticraftMuted) },
                        label = { Text("Settings") },
                    )
                    NavigationBarItem(
                        selected = tab == 2,
                        onClick = { tab = 2; refresh() },
                        icon = { Text("▤", color = if (tab == 2) MatticraftGreen else MatticraftMuted) },
                        label = { Text("Mods") },
                    )
                    NavigationBarItem(
                        selected = tab == 3,
                        onClick = { tab = 3; refresh() },
                        icon = { Text("≡", color = if (tab == 3) MatticraftGreen else MatticraftMuted) },
                        label = { Text("Logs") },
                    )
                }
            },
        ) { innerPadding ->
            Box(Modifier.fillMaxSize().padding(innerPadding)) {
                when (tab) {
                    0 -> HomeTab(
                        profile = profile,
                        status = status,
                        onPlay = { startGame() },
                        onSettings = { tab = 1 },
                        selectionMenu = selectionMenu,
                        onSelectionMenu = { selectionMenu = it },
                        onChange = { profile = it },
                    )
                    1 -> SettingsTab(
                        profile = profile,
                        onChange = { profile = it },
                        onPlay = { startGame() },
                        onReset = {
                            profile = LauncherProfile()
                            status = "Profile reset"
                        },
                    )
                    2 -> ModsTab(
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
    }

    @Composable
    private fun HomeTab(
        profile: LauncherProfile,
        status: String,
        onPlay: () -> Unit,
        onSettings: () -> Unit,
        selectionMenu: Boolean,
        onSelectionMenu: (Boolean) -> Unit,
        onChange: (LauncherProfile) -> Unit,
    ) {
        Column(
            modifier = Modifier.fillMaxSize().verticalScroll(rememberScrollState()).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(14.dp),
        ) {
            Card(
                modifier = Modifier.fillMaxWidth().height(164.dp),
                shape = RoundedCornerShape(24.dp),
                colors = CardDefaults.cardColors(containerColor = Color.Transparent),
            ) {
                Box(
                    modifier = Modifier.fillMaxSize().background(
                        Brush.linearGradient(
                            listOf(Color(0xFF162B1D), Color(0xFF2D5A35), Color(0xFF101A12)),
                        ),
                    ).padding(22.dp),
                ) {
                    Column(
                        modifier = Modifier.align(Alignment.BottomStart),
                        verticalArrangement = Arrangement.spacedBy(4.dp),
                    ) {
                        Text(
                            "JAVA EDITION",
                            fontSize = 12.sp,
                            fontWeight = FontWeight.Bold,
                            letterSpacing = 3.sp,
                            color = MatticraftGreen,
                        )
                        Text(
                            "Your world.\nYour rules.",
                            fontSize = 28.sp,
                            lineHeight = 31.sp,
                            fontWeight = FontWeight.Black,
                            color = MatticraftInk,
                        )
                        Text(
                            "C port · NeoForge toolchain · ${profile.gameVersion}",
                            fontSize = 11.sp,
                            color = MatticraftMuted,
                        )
                    }
                }
            }

            Button(
                onClick = onPlay,
                modifier = Modifier.fillMaxWidth().height(62.dp),
                shape = RoundedCornerShape(18.dp),
            ) {
                Text("▶  PLAY", fontSize = 22.sp, fontWeight = FontWeight.Black)
            }

            Text(
                status,
                fontSize = 12.sp,
                color = if (status.startsWith("Game exited")) MatticraftBlue else MatticraftMuted,
                modifier = Modifier.padding(horizontal = 4.dp),
            )

            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(18.dp),
                colors = CardDefaults.cardColors(
                    containerColor = MaterialTheme.colorScheme.surfaceVariant,
                ),
            ) {
                Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text("Quick settings", fontWeight = FontWeight.Bold, fontSize = 16.sp)
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text("Profile", modifier = Modifier.weight(1f))
                        Text(profile.name, color = MatticraftGreen, fontWeight = FontWeight.Bold)
                    }
                    Box {
                        OutlinedButton(
                            onClick = { onSelectionMenu(true) },
                            modifier = Modifier.fillMaxWidth(),
                        ) {
                            Text("Renderer · ${profile.selection}")
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
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Column(Modifier.weight(1f)) {
                            Text("Fullscreen")
                            Text(
                                "${profile.width} × ${profile.height}",
                                fontSize = 11.sp,
                                color = MatticraftMuted,
                            )
                        }
                        Switch(
                            checked = profile.fullscreen,
                            onCheckedChange = { onChange(profile.copy(fullscreen = it)) },
                        )
                    }
                }
            }

            OutlinedButton(
                onClick = onSettings,
                modifier = Modifier.fillMaxWidth().height(52.dp),
                shape = RoundedCornerShape(16.dp),
            ) {
                Text("More settings")
            }

            Text(
                "Instance: ${profile.instanceDir(this@MainActivity).absolutePath}",
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
                color = MatticraftMuted,
            )
        }
    }

    @Composable
    private fun SettingsTab(
        profile: LauncherProfile,
        onChange: (LauncherProfile) -> Unit,
        onPlay: () -> Unit,
        onReset: () -> Unit,
    ) {
        var selectionMenu by remember { mutableStateOf(false) }
        var resolutionMenu by remember { mutableStateOf(false) }
        val resolutions = listOf(1280 to 720, 1920 to 1080, 854 to 480, 2560 to 1440)

        Column(
            modifier = Modifier.fillMaxSize().verticalScroll(rememberScrollState()).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(14.dp),
        ) {
            Text("Settings", fontSize = 26.sp, fontWeight = FontWeight.Black)
            Text(
                "Launcher and game options are saved per profile.",
                fontSize = 12.sp,
                color = MatticraftMuted,
            )

            Card(shape = RoundedCornerShape(18.dp)) {
                Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text("Profile", fontWeight = FontWeight.Bold)
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
                }
            }

            Card(shape = RoundedCornerShape(18.dp)) {
                Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text("Display", fontWeight = FontWeight.Bold)
                    Box {
                        OutlinedButton(
                            onClick = { selectionMenu = true },
                            modifier = Modifier.fillMaxWidth(),
                        ) {
                            Text("Renderer · ${profile.selection}")
                        }
                        DropdownMenu(
                            expanded = selectionMenu,
                            onDismissRequest = { selectionMenu = false },
                        ) {
                            ANDROID_SELECTIONS.forEach { selection ->
                                DropdownMenuItem(
                                    text = { Text(selection) },
                                    onClick = {
                                        onChange(profile.copy(selection = selection))
                                        selectionMenu = false
                                    },
                                )
                            }
                        }
                    }
                    Box {
                        OutlinedButton(
                            onClick = { resolutionMenu = true },
                            modifier = Modifier.fillMaxWidth(),
                        ) {
                            Text("Resolution · ${profile.width} × ${profile.height}")
                        }
                        DropdownMenu(
                            expanded = resolutionMenu,
                            onDismissRequest = { resolutionMenu = false },
                        ) {
                            resolutions.forEach { (width, height) ->
                                DropdownMenuItem(
                                    text = { Text("$width × $height") },
                                    onClick = {
                                        onChange(profile.copy(width = width, height = height))
                                        resolutionMenu = false
                                    },
                                )
                            }
                        }
                    }
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text("Fullscreen", modifier = Modifier.weight(1f))
                        Switch(
                            checked = profile.fullscreen,
                            onCheckedChange = { onChange(profile.copy(fullscreen = it)) },
                        )
                    }
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Column(Modifier.weight(1f)) {
                            Text("Keep screen on")
                            Text("While the game is running", fontSize = 11.sp, color = MatticraftMuted)
                        }
                        Switch(
                            checked = profile.keepScreenOn,
                            onCheckedChange = { onChange(profile.copy(keepScreenOn = it)) },
                        )
                    }
                }
            }

            Card(shape = RoundedCornerShape(18.dp)) {
                Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text("Advanced", fontWeight = FontWeight.Bold)
                    OutlinedTextField(
                        value = profile.extraArgs,
                        onValueChange = { onChange(profile.copy(extraArgs = it)) },
                        label = { Text("Extra game arguments") },
                        supportingText = { Text("Space separated, for example --no-render") },
                        modifier = Modifier.fillMaxWidth(),
                    )
                    Text(
                        "Instance folder",
                        fontSize = 11.sp,
                        color = MatticraftMuted,
                    )
                    Text(
                        profile.instanceDir(this@MainActivity).absolutePath,
                        fontSize = 10.sp,
                        fontFamily = FontFamily.Monospace,
                    )
                }
            }

            Button(
                onClick = onPlay,
                modifier = Modifier.fillMaxWidth().height(56.dp),
                shape = RoundedCornerShape(16.dp),
            ) {
                Text("▶  PLAY", fontSize = 19.sp, fontWeight = FontWeight.Bold)
            }
            OutlinedButton(
                onClick = onReset,
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp),
            ) {
                Text("Reset profile")
            }
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
        Column(Modifier.fillMaxSize().padding(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            Text("Mods", fontSize = 26.sp, fontWeight = FontWeight.Black)
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedButton(onClick = onRefresh) { Text("Rescan") }
                OutlinedButton(onClick = onWriteManifest) { Text("Write mods.json") }
            }
            Text(
                profile.modsDir(this@MainActivity).absolutePath,
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
                color = MatticraftMuted,
            )
            if (issues.isNotEmpty()) {
                Card(
                    colors = CardDefaults.cardColors(
                        containerColor = MaterialTheme.colorScheme.errorContainer,
                    ),
                ) {
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
                                color = MatticraftMuted,
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
        Column(Modifier.fillMaxSize().padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Text("Logs", fontSize = 26.sp, fontWeight = FontWeight.Black)
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
            HorizontalDivider()
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
            keepScreenOn = json.optBoolean("keep_screen_on", true),
            extraArgs = json.optString("extra_args_text", ""),
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
        .put("keep_screen_on", profile.keepScreenOn)
        .put("extra_args_text", profile.extraArgs)
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
