import os
import subprocess
import sys

BUILD_DIR = "build"
CONFIG = "Release"

if len(sys.argv) > 1 and sys.argv[1].lower() in ("debug", "release"):
    CONFIG = sys.argv[1].capitalize()

VCPKG_TOOLCHAIN = "vcpkg-local/scripts/buildsystems/vcpkg.cmake"
cmake_configure = [
    "cmake", "-S", ".", "-B", BUILD_DIR,
    f"-DCMAKE_TOOLCHAIN_FILE={VCPKG_TOOLCHAIN}",
    "-G", "Visual Studio 17 2022"
]
cmake_build = ["cmake", "--build", BUILD_DIR, "--config", CONFIG]

print(f"[CBA BUILD] Configurazione: {CONFIG}")

# Aggiungi C:\ninja all'inizio del PATH per assicurarti che Ninja sia trovato
os.environ["PATH"] = r"C:\ninja;" + os.environ.get("PATH", "")

try:
    print("[CBA BUILD] Configurazione CMake...")
    subprocess.check_call(cmake_configure)
    print("[CBA BUILD] Compilazione...")
    subprocess.check_call(cmake_build)
    print(f"[CBA BUILD] Build completata con successo in '{BUILD_DIR}' [{CONFIG}]")
except subprocess.CalledProcessError as e:
    print(f"[CBA BUILD] Errore durante la build: {e}")
    sys.exit(1) 