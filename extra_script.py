# -*- coding: utf-8 -*-
"""
Cible PlatformIO "merged" : génère un seul fichier firmware.merged.bin
à partir de bootloader.bin, partitions.bin et firmware.bin (même contenu
que le flash avec les 3 fichiers, pour esptool-js à l'adresse 0x0).
"""
Import("env")
import os

build_dir = env.subst("$BUILD_DIR")
# Slashes normaux pour éviter \f, \p etc. mal interprétés sous Windows
def path_safe(p):
    return os.path.normpath(p).replace("\\", "/")
out = path_safe(os.path.join(build_dir, "firmware.merged.bin"))
bootloader = path_safe(os.path.join(build_dir, "bootloader.bin"))
partitions = path_safe(os.path.join(build_dir, "partitions.bin"))
firmware = path_safe(os.path.join(build_dir, "firmware.bin"))

cmd = (
    'esptool.py --chip esp32s3 merge_bin -o "%s" --flash_mode qio --flash_size 16MB '
    '0x0 "%s" 0x8000 "%s" 0x10000 "%s"'
    % (out, bootloader, partitions, firmware)
)

# Générer firmware.merged.bin automatiquement à chaque build
env.AddPostAction("$BUILD_DIR/firmware.bin", cmd)

# Cible manuelle "merged" si on veut le faire sans recompiler
env.AddCustomTarget(
    name="merged",
    dependencies=["$BUILD_DIR/firmware.bin"],
    actions=[cmd],
    title="Générer firmware.merged.bin (un seul fichier pour flash à 0x0)",
)
