{ pkgs }: {
    deps = [
        pkgs.gcc
        pkgs.make
        pkgs.glibc
        pkgs.curl
        pkgs.json-c
        pkgs.fftw
        pkgs.websockets
        pkgs.gdb
        pkgs.ccls
        pkgs.pkg-config
    ];
} 