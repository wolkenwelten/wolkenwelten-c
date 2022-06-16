with import <nixpkgs> {};

let
    stdenv11 = overrideCC stdenv gcc11;
in
    stdenv11.mkDerivation rec {
        name = "wolkenwelten-build";
        env = buildEnv {
            name = name;
            paths = buildInputs;
        };
        buildInputs = [
            pkgconfig
            ffmpeg
            SDL2
            SDL2_mixer
        ];
    }