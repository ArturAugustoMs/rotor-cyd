"""Gera imagens fiéis às telas do CYD a partir das constantes do hmi.cpp.

Requisito: pip install pillow

Saídas em docs/:
    tela-principal.png     320x240 (nativa)
    tela-principal@2x.png  640x480 (para o README)
    tela-configuracoes@2x.png
    tela-wifi-ap@2x.png
"""
from __future__ import annotations

import math
import re
import struct
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / "docs"
S = 3  # fator de superamostragem


# --- Cores RGB565 exatamente como chegam ao painel -------------------------
def c565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def un565(c: int) -> tuple[int, int, int]:
    return (((c >> 11) & 0x1F) << 3, ((c >> 5) & 0x3F) << 2, (c & 0x1F) << 3)


def blend(a: int, b: int, t: int) -> int:
    ar, ag, ab = (a >> 11) & 0x1F, (a >> 5) & 0x3F, a & 0x1F
    br, bg, bb = (b >> 11) & 0x1F, (b >> 5) & 0x3F, b & 0x1F
    return (((ar + (((br - ar) * t) >> 8)) << 11)
            | ((ag + (((bg - ag) * t) >> 8)) << 5)
            | (ab + (((bb - ab) * t) >> 8)))


def rgb565(r: int, g: int, b: int) -> tuple[int, int, int]:
    return un565(c565(r, g, b))


BG = rgb565(4, 10, 28)
PANEL = rgb565(8, 16, 38)
CYAN = rgb565(0, 200, 255)
TEAL = rgb565(45, 212, 191)
TEXT = rgb565(235, 240, 250)
MUTED = rgb565(120, 140, 170)
GREEN = rgb565(60, 220, 110)
RED = rgb565(255, 70, 70)


def rgb_blend(a: tuple[int, int, int], b: tuple[int, int, int], t: int):
    return un565(blend(c565(*a), c565(*b), t))


# --- Geometria (espelha src/ui/hmi.cpp) ------------------------------------
CX, CY = 160, 109
ROSA_R = 64
COMPASS_SC = ROSA_R + 14
BTN_Y, BTN_W, BTN_H = 188, 104, 42
BTN_CCW_X, BTN_CW_X = 34, 182
CFG_X, CFG_Y, CFG_W, CFG_H = 292, 4, 24, 22

FONTS = Path("C:/Windows/Fonts")


def font(px: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    name = "arialbd.ttf" if bold else "arial.ttf"
    return ImageFont.truetype(str(FONTS / name), int(px * S * 1.1))


def vlw_advances(size: int) -> dict[str, int]:
    """xAdvance de cada glifo, lido diretamente do VLW embarcado no firmware."""
    blob = (ROOT / "data" / f"font_{size}.vlw").read_bytes()
    count = struct.unpack(">I", blob[:4])[0]
    return {
        chr(struct.unpack(">I", blob[o:o + 4])[0]):
            struct.unpack(">I", blob[o + 12:o + 16])[0]
        for o in (24 + i * 28 for i in range(count))
    }


_L_ADVANCES = vlw_advances("l")
# O Arial é mais largo que o VLW; escala o texto desenhado para bater com a
# largura real medida pelo firmware (tft.textWidth com uiFont(3) == font_l).
_HEADER_SCALE = sum(_L_ADVANCES[c] for c in "PY2ROTOR") / (
    font(22).getlength("PY2ROTOR") / S)


def text_width(text: str, px: int, bold: bool = False) -> float:
    """Largura em pixels lógicos, equivalente a tft.textWidth()."""
    if px == 22:  # uiFont(3) == font_l (22 px)
        return float(sum(_L_ADVANCES.get(ch, 6) for ch in text))
    return font(px, bold).getlength(text) / S


def header_draw_width(text: str) -> float:
    return font(22).getlength(text) / S * _HEADER_SCALE


class Canvas:
    def __init__(self) -> None:
        self.im = Image.new("RGB", (320 * S, 240 * S), BG)
        self.d = ImageDraw.Draw(self.im)

    def box(self, x, y, w, h, r, fill, outline=None, width=1):
        self.d.rounded_rectangle((x * S, y * S, (x + w) * S, (y + h) * S),
                                 radius=r * S, fill=fill,
                                 outline=outline, width=max(1, width * S))

    def rect(self, x, y, w, h, fill):
        self.d.rectangle((x * S, y * S, (x + w) * S, (y + h) * S), fill=fill)

    def circle(self, cx, cy, r, fill=None, outline=None, width=1):
        self.d.ellipse(((cx - r) * S, (cy - r) * S, (cx + r) * S, (cy + r) * S),
                       fill=fill, outline=outline, width=max(1, width * S))

    def line(self, x1, y1, x2, y2, fill, width=1):
        self.d.line((x1 * S, y1 * S, x2 * S, y2 * S), fill=fill,
                    width=max(1, width * S))

    def quad(self, pts, fill):
        self.d.polygon([(x * S, y * S) for x, y in pts], fill=fill)

    def text(self, text, x, y, px, fill, anchor="mm", bold=False):
        self.d.text((x * S, y * S), text, font=font(px, bold), fill=fill,
                    anchor=anchor)

    def save(self, name: str) -> None:
        DOCS.mkdir(exist_ok=True)
        self.im.resize((640, 480), Image.Resampling.LANCZOS).save(
            DOCS / f"{name}@2x.png", optimize=True)
        self.im.resize((320, 240), Image.Resampling.LANCZOS).save(
            DOCS / f"{name}.png", optimize=True)


def draw_header(c: Canvas) -> None:
    c.rect(0, 0, 320, 30, PANEL)
    c.rect(0, 28, 320, 2, CYAN)

    # Mesma truncagem do drawHeader(): o nome encolhe até não tocar o indicativo.
    call = "PY2ROTOR"
    call_right = CFG_X - 6
    name_limit = call_right - text_width(call, 22) - 12
    if name_limit < 24:
        name_limit = 24
    name = "ROTOR ANTENA"
    while len(name) > 1 and text_width(name, 22) > name_limit:
        name = name[:-1]

    hpx = 22 * _HEADER_SCALE
    c.text(name, 8, 3, hpx, TEXT, anchor="lt")
    c.text(call, call_right, 3, hpx, CYAN, anchor="rt")
    gx, gy = CFG_X + CFG_W // 2, CFG_Y + CFG_H // 2
    c.box(CFG_X, CFG_Y, CFG_W, CFG_H, 6,
          rgb_blend(PANEL, TEXT, 26), rgb_blend(PANEL, MUTED, 70))
    for i in (-1, 0, 1):
        c.box(gx - 6, gy + i * 5 - 1, 12, 3, 1, CYAN)


def draw_compass(c: Canvas, angle: int = 137) -> None:
    x0, y0 = CX - COMPASS_SC, CY - COMPASS_SC
    cx, cy = x0 + COMPASS_SC, y0 + COMPASS_SC
    c.rect(x0, y0, 2 * COMPASS_SC, 2 * COMPASS_SC, BG)

    for r, t in ((ROSA_R + 3, 10), (ROSA_R + 2, 16), (ROSA_R + 1, 26)):
        c.circle(cx, cy, r, outline=rgb_blend(BG, TEAL, t))
    c.circle(cx, cy, ROSA_R, fill=PANEL)
    for i in range(4):
        c.circle(cx, cy, ROSA_R - i,
                 outline=rgb_blend(PANEL, TEAL, 64 - i * 13))
    c.circle(cx, cy, ROSA_R - 6, outline=rgb_blend(PANEL, MUTED, 70))

    for a in range(0, 360, 15):
        rad = math.radians(a - 90)
        cardinal, major = a % 90 == 0, a % 45 == 0
        ra = ROSA_R - 15 if cardinal else (ROSA_R - 11 if major else ROSA_R - 7)
        rb = ROSA_R - 4
        color = TEXT if cardinal else (TEAL if major else MUTED)
        c.line(cx + math.cos(rad) * ra, cy + math.sin(rad) * ra,
               cx + math.cos(rad) * rb, cy + math.sin(rad) * rb, color)
        if not cardinal and not major:
            c.circle(round(cx + math.cos(rad) * (rb + 2)),
                     round(cy + math.sin(rad) * (rb + 2)), 1, color)

    for label, x, y in (("N", cx, cy - ROSA_R - 6), ("S", cx, cy + ROSA_R + 6),
                        ("E", cx + ROSA_R + 6, cy), ("W", cx - ROSA_R - 6, cy)):
        c.text(label, x, y, 15, CYAN)

    c.text(f"{angle:03d}", cx, cy, 48, GREEN, bold=True)
    c.text("GRAUS", cx, cy + 36, 11, MUTED)

    rad = math.radians(angle - 90)
    dx, dy = math.cos(rad), math.sin(rad)
    tx, ty = cx + dx * (ROSA_R - 2), cy + dy * (ROSA_R - 2)
    bx, by = cx + dx * (ROSA_R - 13), cy + dy * (ROSA_R - 13)
    px, py = -dy * 5, dx * 5
    c.quad([(tx, ty), (bx + px, by + py), (bx - px, by - py)], RED)


def draw_arrow(c: Canvas, x: int, left: bool) -> None:
    color = RED if left else GREEN
    r = 12
    c.box(x, BTN_Y, BTN_W, BTN_H, r, rgb_blend(PANEL, TEXT, 15), color)
    c.line(x + r, BTN_Y + 1, x + BTN_W - r, BTN_Y + 1,
           rgb_blend(PANEL, TEXT, 42))
    c.line(x + r, BTN_Y + BTN_H - 2, x + BTN_W - r, BTN_Y + BTN_H - 2,
           rgb_blend(BG, color, 40))

    mx, my = x + BTN_W // 2, BTN_Y + BTN_H // 2
    d = -1 if left else 1
    tip, out, end, inn = mx + d * 12, mx - d * 5, mx - d * 11, mx - d * 3
    c.quad([(tip, my), (out, my - 13), (end, my - 10), (inn, my)], color)
    c.quad([(inn, my), (end, my + 10), (out, my + 13), (tip, my)], color)
    c.line(tip, my - 1, out, my - 13, rgb_blend(color, TEXT, 40))


def render_main() -> None:
    c = Canvas()
    draw_header(c)
    draw_compass(c)
    draw_arrow(c, BTN_CCW_X, True)
    draw_arrow(c, BTN_CW_X, False)
    c.save("tela-principal")


def draw_menu_item(c: Canvas, y: int, label: str, color, value: str | None = None):
    if value is None:
        c.box(48, y, 224, 26, 7, color)
        c.text(label, 160, y + 13, 15, BG)
    else:
        bg = rgb_blend(PANEL, color, 22)
        c.box(48, y, 224, 26, 7, bg, color)
        c.text(label, 58, y + 13, 15, color, anchor="lm")
        c.text(value, 262, y + 13, 15, TEXT, anchor="rm")


def render_config() -> None:
    c = Canvas()
    c.box(28, 32, 264, 206, 12, PANEL, CYAN)
    c.text("CONFIGURACOES", 160, 57, 22, CYAN)
    draw_menu_item(c, 72, "CALIBRAR", CYAN)
    draw_menu_item(c, 99, "RESETAR REDE", RED)
    draw_menu_item(c, 126, "IDIOMA", TEAL, "PT-BR")
    draw_menu_item(c, 153, "BRILHO", TEAL, "100%")
    draw_menu_item(c, 180, "SOBRE", TEAL)
    draw_menu_item(c, 207, "VOLTAR", MUTED)
    c.save("tela-configuracoes")


def read_qr_bits() -> tuple[int, int, list[int]]:
    src = (ROOT / "src" / "hardware" / "ap_qr.cpp").read_text(encoding="utf-8")
    hdr = (ROOT / "include" / "hardware" / "ap_qr.h").read_text(encoding="utf-8")
    w = int(re.search(r"kApQrWidth\s*=\s*(\d+)", hdr).group(1))
    h = int(re.search(r"kApQrHeight\s*=\s*(\d+)", hdr).group(1))
    body = src.split("{", 1)[1].split("}", 1)[0]
    return w, h, [int(v, 16) for v in re.findall(r"0x([0-9A-Fa-f]{2})", body)]


def render_ap() -> None:
    c = Canvas()
    c.box(8, 4, 304, 232, 14, PANEL, CYAN)
    c.text("CONFIGURAR WIFI", 160, 22, 22, CYAN)

    w, h, bits = read_qr_bits()
    qx, qy = 160 - w // 2, 42
    c.rect(qx, qy, w, h, TEXT)
    for y in range(h):
        for x in range(0, w, 8):
            byte = bits[y * ((w + 7) // 8) + x // 8]
            for b in range(8):
                if byte & (1 << (7 - b)):
                    c.rect(qx + x + b, qy + y, 1, 1, BG)

    c.text("Conecte-se ao AP:", 160, 158, 11, MUTED)
    c.text("Rotor-Setup", 160, 178, 22, CYAN)
    c.text("192.168.4.1", 160, 198, 13, MUTED)
    draw_menu_item(c, 210, "AVANCAR", TEAL)
    c.save("tela-wifi-ap")


if __name__ == "__main__":
    render_main()
    render_config()
    render_ap()
    print("Imagens geradas em docs/")
