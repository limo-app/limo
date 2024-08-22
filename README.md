<h1 align="center">Limo <img src="resources/logo.png" alt="logo" width="40"/></h1>

General purpose mod manager primarily developed for Linux with support for the [NexusMods](https://www.nexusmods.com/) API and [LOOT](https://loot.github.io/).

<p align="center">
<img src="resources/showcase.png" alt="logo" width="800"/>
</p>

## Features

- Multiple target directories per application
- Automatic adaptation of mod file names to prevent issues with case mismatches
- Auto-Tagging system for filtering
- FOMOD support
- Sort load order according to conflicts
- Import installed games from Steam
- Simple backup system
- LOOT integration:
    - Manage installed plugins
    - Automatically sort the load order
    - Check for issues with installed plugins
- NexusMods API support:
    - Check for mod updates
    - View description, changelogs and available files
    - Download mods through Limo
    
***For a guide on how to use Limo, refer to the wiki.***

## Installation

### Flatpak

<a href='https://flathub.org/apps/io.github.limo_app.limo'>
    <img width='240' alt='Download on Flathub' src='https://flathub.org/api/badge?locale=en'/>
</a>

### Build from source

####  Install the dependencies

 - [Qt5](https://doc.qt.io/qt-5/index.html)
 - [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
 - [libarchive](https://github.com/libarchive/libarchive)
 - [pugixml](https://github.com/zeux/pugixml)
 - [OpenSSL](https://github.com/openssl/openssl)
 - [cpr](https://github.com/libcpr/cpr)
 - [libloot](https://github.com/loot/libloot)
 - (Optional, for tests) [Catch2](https://github.com/catchorg/Catch2)
 - (Optional, for docs) [doxygen](https://github.com/doxygen/doxygen)

On Debian based systems most dependencies, with the exception of cpr and libloot, can be installed with the following command:

```
sudo apt install \
		build-essential \
		cmake \
		git \
		libpugixml-dev \
		libjsoncpp-dev \
		libarchive-dev \
		pkg-config \
		libssl-dev \
		qtbase5-dev \
		qtchooser \
		qt5-qmake \
		qtbase5-dev-tools \
		libqt5svg5-dev \
		libbost-all-dev \
		libtbb-dev \
		cargo \
		cbindgen \
		catch2 \
		doxygen		
```

#### Clone this repository:

```
git clone https://github.com/limo-app/limo.git
cd limo
```

#### Build libunrar:

```
git clone https://github.com/aawc/unrar.git
cd unrar
make lib
cd ..
```

#### Build Limo:

```
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build
```
 
#### (Optional) Run the tests:

```
mkdir tests/build
cmake -DCMAKE_BUILD_TYPE=Release -S tests -B tests/build
cmake --build tests/build
tests/build/tests
```

#### (Optional) Build the documentation:

```
doxygen src/lmm_Doxyfile
```
