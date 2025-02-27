<div style="text-align: center;">
  <h1 style="font-size: 3em;">Limo
  <img src="./resources/logo.png" alt="Limo Logo" style="width: 1em; height: 1em; vertical-align: middle;"></h1>
</div>

# General purpose mod manager

Primarily developed for Linux with support for the [NexusMods](https://www.nexusmods.com/) API and [LOOT](https://loot.github.io/).

![image info](./resources/showcase.png)

## Features

* **Multiple Target Directories:** Manage mods for different installations of the same game.
* **Case-Mismatch Prevention:** Automatically adjust mod file names to avoid issues on case-sensitive file systems.
* **Auto-Tagging System:** Easily filter and organize mods using customizable tags.
* **FOMOD Support:** Seamlessly install mods with interactive installers.
* **Load Order Management:**
  * **Conflict Resolution:** Sort load order to minimize conflicts.
  * **LOOT Integration:** Automatically sort plugins, detect issues, and manage load order.
* **Game Integration:**
  * **Steam Import:** Automatically detect and import installed games from Steam.
* **Backup System:** Create simple backups of your mod configurations.
* **NexusMods API Integration:**
  * **Mod Updates:** Check for and download mod updates directly.
  * **Mod Information:** View descriptions, changelogs, and available files.
  * **Direct Downloads:** Download mods directly through Limo.
* **OpenMW Support:**
  * **Plugin and Archive Management:** Manage plugins and BSA archives.
  * **LOOT Compatibility:** Utilize LOOT for OpenMW load order management.

## How Limo Works

Limo's functionality revolves around two key concepts: the **Staging Directory** and **Deployers**.

### Staging Directory: Managing Mod Files

When you install a mod, Limo stores its files in the *Staging Directory* instead of directly modifying the game's files. This approach allows for quick and easy mod management, enabling you to enable, disable, and resolve conflicts efficiently.

### Deployers: Linking Mods to the Game

To apply mods to your game, Limo uses *Deployers*. Each *Deployer* manages a set of mods and links them to its *Target Directory*, which is the game's installation folder. If mod conflicts arise, the mod positioned lower in the *Deployer's* load order takes precedence. Limo automatically creates backups of any files in the *Target Directory* that need to be overwritten, and these backups are restored when the mod is deactivated.

For a comprehensive guide on using Limo, please refer to the [**Limo Wiki**](https://github.com/limo-app/limo/wiki). This resource provides detailed instructions and is applicable even if you are not modding Skyrim.

## Installation

Limo is primarily distributed as a Flatpak application for ease of use across Linux distributions. We also provide an AUR package for Arch Linux users.

### Flatpak (Recommended)

For the best experience, we recommend installing Limo via Flatpak. This ensures consistent performance and automatic updates.

[![Download on Flathub](https://flathub.org/api/badge?locale=en)](https://flathub.org/apps/io.github.limo_app.limo)

**Installation Instructions:**

1. **Enable Flatpak:** If you haven't already, ensure Flatpak is installed and configured on your system. Refer to the [Flatpak setup guide](https://flatpak.org/setup/) for instructions.
2. **Install Limo:** Click the "Download on Flathub" button above, or use your software center's Flatpak integration to find and install Limo.

### Arch Linux (AUR)

If you're an Arch Linux user and prefer to use the AUR, Limo is also available there.

[![Get on AUR](https://upload.wikimedia.org/wikipedia/commons/e/e8/Archlinux-logo-standard-version.png)](https://aur.archlinux.org/packages/limo-git)

**Installation Instructions:**

1. **Use an AUR Helper:** If you're not familiar with the AUR, we recommend using an AUR helper like `yay` or `paru`.
2. **Install Limo:** Use your preferred AUR helper to search for and install the `limo-git` package.

**Note:** The AUR version might not be as up-to-date as the Flatpak version, and support may be limited. We recommend the Flatpak version for most users.

### Build from Source

To build Limo from source, you'll need to install the following dependencies:

* [Qt5](https://doc.qt.io/qt-5/index.html)
* [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
* [libarchive](https://github.com/libarchive/libarchive)
* [pugixml](https://github.com/zeux/pugixml)
* [OpenSSL](https://github.com/openssl/openssl)
* [cpr](https://github.com/libcpr/cpr)
* [libloot](https://github.com/loot/libloot)
* (Optional, for running tests) [Catch2](https://github.com/catchorg/Catch2)
* (Optional, for generating documentation) [doxygen](https://github.com/doxygen/doxygen)

**Note:** The installation process for these dependencies will vary depending on your operating system and distribution. Consult your distribution's package manager or the respective project's documentation for installation instructions.

On Debian-based systems, most dependencies (excluding cpr and libloot) can be installed using your system's package manager.

**Dependencies to install:**

* `build-essential`
* `cmake`
* `git`
* `libpugixml-dev`
* `libjsoncpp-dev`
* `libarchive-dev`
* `pkg-config`
* `libssl-dev`
* `qtbase5-dev`
* `qtchooser`
* `qt5-qmake`
* `qtbase5-dev-tools`
* `libqt5svg5-dev`
* `libboost-all-dev`
* `libtbb-dev`
* `cargo`
* `cbindgen`
* `catch2`
* `doxygen`

**Note:** You will still need to install `cpr` and `libloot` separately. Please refer to their respective project documentation for installation instructions.

#### Clone the Limo Repository

git clone [https://github.com/limo-app/limo.git](https://github.com/limo-app/limo.git)
cd limo

#### Build libunrar (Required)

git clone [https://github.com/aawc/unrar.git](https://github.com/aawc/unrar.git)
cd unrar
make lib
cd ..

#### Build Limo

mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build

#### (Optional) Run Unit Tests

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build

#### (Optional) Build Documentation

doxygen src/lmm_Doxyfile

## Usage Notes

### Flatpak Version of Limo

Starting with version 1.0.7, Limo offers specialized deployer and auto-tag import features for Steam games. Currently, this support is limited to Bethesda titles like Skyrim, Skyrim Special Edition (SE), and Skyrim VR.

**Note for Flatpak Users:** If you're using the Flatpak version of Limo and want to mod these Bethesda games, the necessary configurations are applied automatically. However, we strongly recommend consulting the [**Limo Wiki**](https://github.com/limo-app/limo/wiki) for detailed instructions and troubleshooting tips, regardless of which games you're modding.

#### Running Tools Directly in Limo (Flatpak)

To add and run external tools directly within Limo's Flatpak environment, use the following command structure:

--directory="/tool/directory/" protontricks-launch --appid [Steam App ID, e.g., 489830 for Skyrim SE] tool.exe

### AUR Version of Limo

The Flatpak version is the officially supported distribution of Limo. While an AUR (Arch User Repository) version is available, its use is generally discouraged. The primary advantages of the AUR version are:

* **Faster Download Initialization:** Slightly quicker start to downloads from NexusMods.
* **Early Access to Features:** Access to pre-release features like the Reverse Deployer (as of version 1.0.7).

**Important Considerations:**

* If you choose to use the AUR version, please refrain from contacting the main developer with excessive support requests.
* The AUR version's user interface may differ slightly from the Flatpak version.
* When importing Steam games, such as Skyrim Special Edition, the deployers may not be automatically configured correctly. **Therefore, it is essential to consult the [Limo Wiki](https://github.com/limo-app/limo/wiki) for proper configuration instructions.**

**Recommendation:**

* For the most stable and supported experience, the Flatpak version of Limo is highly recommended.

#### Deployer Configuration for AUR Version

To ensure proper functionality with the AUR version of Limo, particularly for Skyrim Special Edition (SE) on Steam, you'll need to create specific deployers and configure their target directories. Here's a quick reference:

| Name    | Deployer Type          | Target Directory                                                                                                    | Deployment Method |
| --------- | ------------------------ | --------------------------------------------------------------------------------------------------------------------- | ------------------- |
| Bin     | Case Matching Deployer | `~/.local/share/Steam/steamapps/common/Skyrim Special Edition`                                                      | Any               |
| Data    | Case Matching Deployer | `~/.local/share/Steam/steamapps/common/Skyrim Special Edition/Data`                                                 | Any               |
| Plugins | Loot Deployer          | `~/.local/share/Steam/steamapps/compatdata/489830/pfx/drive_c/users/steamuser/AppData/Local/Skyrim Special Edition` | Any               |

#### Running Tools Directly in Limo (AUR Version)

To add and run external tools directly within the AUR version of Limo, use the following command structure:

cd "/tool/directory"; protontricks-launch --appid [Steam App ID, e.g., 489830 for SkyrimSE] tool.exe

## Contributing configurations

From version 1.0.7 onwards, Limo provides specialized deployer and auto-tag import configurations for Steam games. These configurations are stored in JSON files named `<STEAM_APP_ID>.json` within the `steam_app_configs` directory of this repository.

If you're using Limo to mod a Steam game that lacks a configuration file, or if you wish to enhance an existing configuration, we encourage you to contribute by submitting a Pull Request.

**Exporting and Contributing Configurations:**

1. **Export Configuration:** Click the "Export" button within the "App" tab. This will generate a file named `exported_config.json` in Limo's staging directory.
2. **Rename File:** Rename `exported_config.json` to `<STEAM_APP_ID>.json`, replacing `<STEAM_APP_ID>` with the appropriate Steam App ID (e.g., `489830.json` for Skyrim SE).
3. **Move File:** Move the renamed file to the `steam_app_configs` directory within this repository.
4. **Submit Pull Request:** Open a Pull Request to contribute your configuration.

**Important Note:**

* Within these configuration files, deploy modes will default to "hard link," even if you're currently using symbolic links.
* When your configuration is imported by other Limo users, symbolic links will be automatically utilized if hard links are not supported on their system.

## Planned Features

**Game-Specific Deployers:**

* **Baldur's Gate 3 (BG3) Deployer:**
  * Similar to the current *LOOT Deployer*, this will automate the addition of mods to the `modsettings.lsx` file, enhancing Baldur's Gate 3 modding support.

**Enhanced Mod Management:**

* **Bethesda Base Plugin Handling:**
  * For Bethesda games (e.g., Skyrim), automatically exclude essential plugins (e.g., `Skyrim.esm`, Creation Club content) from the *LOOT Deployer* list, preventing unnecessary clutter.
* **Mod Grouping:**
  * Implement a tree-view structure to group split mods and patches under their base mod, simplifying management and uninstallation.
* **Installation Rules:**
  * Introduce user-defined rules to automate file/directory manipulation during mod installation.
  * Enable actions like moving or deleting files based on patterns, streamlining tasks like removing screenshots or resolving mod compatibility issues (e.g., *Nemesis* and *Pandora* in Skyrim).

**Configuration and Integration:**

* **Config File Detection:**
  * Add a dedicated tab to list mod and game configuration files (e.g., `.ini`).
  * Provide a button to open these files in the default editor.
  * Allow users to define rules for detecting configuration files, similar to auto-tag rules.
* **API Support for Other Modding Sites:**
  * Expand API support to automatically check for updates and download mods from platforms like *Thunderstore* and *Gamebanana*.
* **Deployers for Additional Games:**
  * Develop specialized deployers for games requiring unique modding workflows, based on user demand (e.g., similar to the *LOOT Deployer* for Skyrim).

**Key Improvements:**

* **Categorization:**
  * Grouped related features under clear subheadings for better organization.
* **Clarity and Conciseness:**
  * Refined descriptions for improved readability.
* **Emphasis on Benefits:**
  * Highlighted the advantages of each planned feature.
* **Consistent Formatting:**
  * Used consistent formatting for bullet points and code snippets.
* **User-Friendly Language:**
  * Used more accessible language throughout the section.
* **Code Snippets:**
  * Used backticks to highlight file names.
