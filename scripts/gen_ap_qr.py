"""Gera o bitmap 1bpp do QR Code do Access Point usado na tela do CYD.

O QR codifica uma string WiFi padrao (WIFI:T:nopass;S:<SSID>;;), para que um
celular conecte no AP de configuracao bastando escanear o codigo.

Uso:
    pip install "qrcode[pil]"
    python scripts/gen_ap_qr.py

Se você alterar kPortalApName em include/config.h, rode este script novamente
e recompile o firmware.
"""
from pathlib import Path

import qrcode

ROOT = Path(__file__).resolve().parents[1]
SSID = "Rotor-Setup"          # mantenha igual a config::kPortalApName
CONTEUDO = f"WIFI:T:nopass;S:{SSID};;"
CAMINHO_H = ROOT / "include" / "hardware" / "ap_qr.h"
CAMINHO_CPP = ROOT / "src" / "hardware" / "ap_qr.cpp"

qr = qrcode.QRCode(
    version=None,
    error_correction=qrcode.constants.ERROR_CORRECT_M,
    box_size=3,
    border=2,
)
qr.add_data(CONTEUDO)
qr.make(fit=True)

img = qr.make_image(fill_color="black", back_color="white").convert("L")
w, h = img.size
px = img.load()

# 1 bit por pixel, MSB primeiro (compatível com LovyanGFX drawBitmap).
out = bytearray()
for y in range(h):
    for x in range(0, w, 8):
        byte = 0
        for b in range(8):
            if x + b < w and px[x + b, y] < 128:
                byte |= 1 << (7 - b)
        out.append(byte)

CAMINHO_H.write_text(
    "#pragma once\n\n"
    "#include <cstdint>\n\n"
    f"// QR do AP \"{SSID}\" ({CONTEUDO}).\n"
    "// Gerado por scripts/gen_ap_qr.py; 1 bit por pixel, MSB primeiro.\n"
    f"constexpr int kApQrWidth = {w};\n"
    f"constexpr int kApQrHeight = {h};\n"
    "extern const uint8_t kApQrBits[];\n",
    encoding="utf-8",
)

with CAMINHO_CPP.open("w", encoding="utf-8") as f:
    f.write('#include "hardware/ap_qr.h"\n\n')
    f.write("const uint8_t kApQrBits[] = {\n")
    for i in range(0, len(out), 16):
        f.write("  " + ", ".join(f"0x{b:02X}" for b in out[i:i + 16]) + ",\n")
    f.write("};\n")

print(f"SSID: {SSID}")
print(f"Conteudo: {CONTEUDO}")
print(f"Bitmap: {w}x{h} px, {len(out)} bytes")
