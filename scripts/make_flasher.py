"""Monta o site do flasher web a partir do build do PlatformIO.

Entradas:
    .pio/build/cyd/{bootloader,partitions,firmware}.bin
    boot_app0.bin do pacote framework-arduinoespressif32
    flasher/index.html
    docs/tela-principal@2x.png
    include/config.h (kFirmwareVersion)

Saída (pasta `site/`, usada pelo GitHub Actions/Pages):
    site/index.html
    site/tela-principal@2x.png
    site/firmware/manifest.json
    site/firmware/{bootloader,partitions,boot_app0,firmware}.bin
"""
from __future__ import annotations

import json
import os
import re
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".pio" / "build" / "cyd"
SITE = ROOT / "site"
FIRMWARE = SITE / "firmware"


def find_boot_app0() -> Path:
    packages = Path.home() / ".platformio" / "packages"
    candidates = sorted(packages.glob("framework-arduinoespressif32*/tools/partitions/boot_app0.bin"))
    if not candidates:
        raise SystemExit("boot_app0.bin não encontrado em ~/.platformio/packages")
    return candidates[-1]


def firmware_version() -> str:
    header = (ROOT / "include" / "config.h").read_text(encoding="utf-8")
    match = re.search(r'kFirmwareVersion\[\]\s*=\s*"([^"]+)"', header)
    if not match:
        raise SystemExit("kFirmwareVersion não encontrado em include/config.h")
    base = match.group(1)

    # Em Release, o nome da tag é a versão. Em push normal, inclui o commit.
    tag = os.environ.get("GITHUB_REF_NAME", "")
    sha = os.environ.get("GITHUB_SHA", "")
    if tag.startswith("v"):
        return tag
    if sha:
        return f"{base}+{sha[:7]}"
    return base


def main() -> None:
    for path in (BUILD / "bootloader.bin", BUILD / "partitions.bin",
                 BUILD / "firmware.bin", ROOT / "flasher" / "index.html",
                 ROOT / "docs" / "tela-principal@2x.png"):
        if not path.exists():
            raise SystemExit(f"Arquivo ausente: {path}")

    if SITE.exists():
        shutil.rmtree(SITE)
    FIRMWARE.mkdir(parents=True)

    shutil.copy2(ROOT / "flasher" / "index.html", SITE / "index.html")
    shutil.copy2(ROOT / "docs" / "tela-principal@2x.png",
                 SITE / "tela-principal@2x.png")

    files = (
        ("bootloader.bin", BUILD / "bootloader.bin", 0x1000),
        ("partitions.bin", BUILD / "partitions.bin", 0x8000),
        ("boot_app0.bin", find_boot_app0(), 0xE000),
        ("firmware.bin", BUILD / "firmware.bin", 0x10000),
    )
    for name, source, _ in files:
        shutil.copy2(source, FIRMWARE / name)

    manifest = {
        "name": "Rotor CYD",
        "version": firmware_version(),
        "builds": [
            {
                "chipFamily": "ESP32",
                "improv": False,
                "parts": [
                    {"path": name, "offset": offset}
                    for name, _, offset in files
                ],
            }
        ],
    }
    (FIRMWARE / "manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8")

    print(f"Site gerado em {SITE} — versão {manifest['version']}")
    for name, _, offset in files:
        print(f"  {name:16s} 0x{offset:05X}  {(FIRMWARE / name).stat().st_size} bytes")


if __name__ == "__main__":
    main()
