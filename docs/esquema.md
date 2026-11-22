# Esquema de ligação — Rotor CYD

ESP32-2432S028R (CYD 2USB, display ST7789). Este documento descreve o
**potenciômetro de azimute** e a **saída dos relés**, que são as duas interfaces
com o mundo externo. O diagrama vetorial está em [`esquema.svg`](esquema.svg).

## 1. Potenciômetro de azimute (ADC)

```
   3V3 ──────┐
             │
          ┌──┴──┐
          │     │  Potenciômetro linear 10 kΩ
          │  ↗  │  (3 terminais)
          └──┬──┘
             │  wiper (pino central)
             └────────── GPIO34  (ADC1_CH6)

   GND ───────────────── GND
```

| Pino do pot | Ligações |
|---|---|
| V+ (uma extremidade) | `3V3` do ESP32 |
| GND (outra extremidade) | `GND` do ESP32 |
| Wiper (central) | `GPIO34` (ADC1, entrada analógica) |

**Leitura no firmware** (`src/main.cpp`, `readAzimuth()`):

- 10 amostras consecutivas, média aritmética.
- `kAdcMin = 100` → **0°**
- `kAdcMax = 3900` → **360°**
- `map()` + `constrain(0, 360)`.

> Use um potenciômetro **linear** (B10K), não logarítmico. Se o valor parar
> antes de 0° ou 360°, ajuste `kAdcMin`/`kAdcMax` em `src/main.cpp` com o valor
> bruto lido nos extremos mecânicos. O ADC do ESP32 é não-linear nas pontas;
> deixe uma pequena folga em relação a 0 e 4095.

## 2. Saída dos relés (motor)

```
   GPIO22 ──────> IN1  ┌──────────┐   COM1 ─┐
   GPIO27 ──────> IN2  │ RL1  CW  │   NO1  ─┤
                       │ RL2  CCW │          ├──> MOTOR
   5V/VIN ──────> VCC  └──────────┘   COM2 ─┤
   GND    ──────> GND                 NO2  ─┘
```

| Pino ESP32 | Função | Observação |
|---|---|---|
| `GPIO22` | `CW` (horário) | **ativo em LOW** |
| `GPIO27` | `CCW` (anti-horário) | **ativo em LOW** |

**Lógica** (`src/hardware/motor.cpp`):

| Comando | GPIO22 | GPIO27 |
|---|---|---|
| Parado | HIGH | HIGH |
| CW | **LOW** | HIGH |
| CCW | HIGH | **LOW** |

- Relés **ativos em nível baixo**: `LOW` Liga, `HIGH` desliga.
- O firmware **nunca** liga os dois relés ao mesmo tempo — as saídas são
  mutuamente exclusivas (`apply()`).
- Watchdog de segurança: após **60 s** de comando contínuo, o motor é desligado
  (`kMaxContinuousRunMs`), inclusive para comandos travados via web.
- Ao ligar, `init()` chama `allStop()`, deixando ambos os pinos em `HIGH`.

## 3. Alimentação e cuidados

- O módulo de relés pode ser alimentado pelo `5V/VIN` do CYD **se a corrente
  das bobinas for baixa**. Para relés de maior consumo, use fonte 5 V externa.
- **GND comum obrigatório** entre ESP32, fonte dos relés e fonte do motor.
- O contato dos relés deve comutar a alimentação do motor por `COM → NO`.
- Coloque um **diodo de roda-livre** (ex.: 1N4007) em paralelo ao motor, com o
  cátodo no positivo, para proteger os contatos e a fonte.
- Use fusível adequado na alimentação do motor.
- O potenciômetro usa apenas 3V3/GND do ESP32 — não ligue ao 5 V, para que a
  leitura do ADC não ultrapasse a faixa calibrada.

## 4. Pinos auxiliares do display/touch (referência)

| Função | GPIO |
|---|---|
| TFT SCLK / MOSI / MISO | 14 / 13 / 12 (HSPI) |
| TFT DC / CS / Backlight | 2 / 15 / 21 |
| Touch SCLK / MOSI / MISO / CS | 25 / 32 / 39 / 33 (VSPI) |
| Touch IRQ (GY-TFT) | 36 — **não usado** (`cfg.pin_int = -1`, evita toque fantasma) |

## 5. Fonte no código

| Informação | Arquivo |
|---|---|
| Definição dos pinos | `include/config.h` |
| Lógica dos relés e watchdog | `src/hardware/motor.cpp` |
| Leitura e calibração do ADC | `src/main.cpp` (`readAzimuth()`, `kAdcMin`, `kAdcMax`) |
| Configuração SPI do display/touch | `include/hardware/lgfx_rotor.hpp` |
