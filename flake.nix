{
  description = "A flake providing a development environment for Limo";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    make-shell.url = "github:nicknovitski/make-shell";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      imports = with inputs; [
        make-shell.flakeModules.default
      ];

      perSystem =
        {
          config,
          pkgs,
          system,
          ...
        }:
        {
          _module.args.pkgs = import inputs.nixpkgs {
            inherit system;
            config.allowUnfree = true;
          };

          packages.default = pkgs.qt6Packages.callPackage ./nix/package.nix {};

          make-shells.default = {
            packages =
              with pkgs;
              [
                clang-tools
                gcc
                gdb
                git
              ]
              ++ config.packages.default.nativeBuildInputs
              ++ config.packages.default.buildInputs;
          };
        };
    };
}
