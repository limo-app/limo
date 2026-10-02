{
  lib,
  stdenv,

  cmake,
  pkg-config,

  jsoncpp,
  libarchive,
  libcpr,
  libloot,
  lz4,
  pugixml,

  wrapQtAppsHook,
  qtbase,
  qtsvg,
  qtwayland,

  kdePackages,

  withUnrar ? false,
  unrar, # has an unfree license
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "limo";
  version = "1.2.2";

  src = ../.;

  patches = lib.optionals (!withUnrar) [
    # remove `unrar` as fallback when libarchive fails
    ./remove-unrar.patch
  ];

  strictDeps = true;

  nativeBuildInputs = [
    cmake
    pkg-config
    wrapQtAppsHook
  ];

  buildInputs = [
    jsoncpp
    libarchive
    libcpr
    libloot
    lz4
    pugixml

    qtbase
    qtsvg
    qtwayland

    kdePackages.breeze-icons
    kdePackages.plasma-integration
  ]
  ++ lib.optionals withUnrar [ unrar ];

  cmakeFlags = [
    (lib.cmakeFeature "LIMO_INSTALL_PREFIX" (placeholder "out"))
  ]
  ++ lib.optionals withUnrar [
    (lib.cmakeBool "USE_SYSTEM_LIBUNRAR" true)
  ]
  ++ lib.optionals (!withUnrar) [
    (lib.cmakeFeature "LIBUNRAR_PATH" "")
    (lib.cmakeFeature "LIBUNRAR_INCLUDE_DIR" "")
  ];

  qtWrapperArgs = [
    "--prefix XDG_DATA_DIRS : ${kdePackages.breeze-icons}/share"
  ];

  meta = {
    description = "General purpose mod manager with support for the NexusMods API and LOOT";
    homepage = "https://github.com/limo-app/limo";
    license = lib.licenses.gpl3Plus;
    mainProgram = "limo";
    maintainers = with lib.maintainers; [
      tomasajt
      MattSturgeon
    ];
    platforms = lib.platforms.linux;
  };
})
