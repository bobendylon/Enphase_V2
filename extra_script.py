# -*- coding: utf-8 -*-
Import("env")
import os
import subprocess

def merge_bin(source, target, env):
    build_dir = env.subst("$BUILD_DIR")

    out        = os.path.join(build_dir, "firmware.merged.bin")
    bootloader = os.path.join(build_dir, "bootloader.bin")
    partitions = os.path.join(build_dir, "partitions.bin")
    firmware   = os.path.join(build_dir, "firmware.bin")

    for f in [bootloader, partitions, firmware]:
        if not os.path.isfile(f):
            print(f"[merge_bin] ERREUR : fichier manquant → {f}")
            return

    esptool_script = os.path.join(
        env.subst("$PROJECT_PACKAGES_DIR"),
        "tool-esptoolpy", "esptool.py"
    )
    python = env.subst("$PYTHONEXE")

    cmd = [
        python, esptool_script,
        "--chip", "esp32s3",
        "merge_bin",
        "-o", out,
        "--flash_mode", "qio",
        "--flash_size", "16MB",
        "0x0",     bootloader,
        "0x8000",  partitions,
        "0x10000", firmware,
    ]

    print("[merge_bin] Génération de firmware.merged.bin...")
    print("[merge_bin] esptool utilisé :", esptool_script)

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode == 0:
        size = os.path.getsize(out) / 1024
        print(f"[merge_bin] ✓ firmware.merged.bin généré ({size:.1f} Ko)")
    else:
        print("[merge_bin] ERREUR esptool :")
        print(result.stderr)

env.AddPostAction("$BUILD_DIR/firmware.bin", merge_bin)

env.AddCustomTarget(
    name="merged",
    dependencies=["$BUILD_DIR/firmware.bin"],
    actions=[merge_bin],
    title="Générer firmware.merged.bin (flash à 0x0)",
)