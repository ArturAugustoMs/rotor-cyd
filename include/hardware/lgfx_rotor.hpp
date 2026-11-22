#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "config.h"

/**
 * CYD 2USB (ESP32-2432S028R, ST7789) display + XPT2046 touch.
 * Both buses are handled by LovyanGFX, so touch coordinates already follow
 * setRotation() and no separate touch library is needed.
 */
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Light_PWM _light;
  lgfx::Touch_XPT2046 _touch;

 public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;  // HSPI
      cfg.spi_mode = 0;
      cfg.freq_write = config::kTftSpiWriteHz;
      cfg.freq_read = config::kTftSpiReadHz;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = config::kPinTftSclk;
      cfg.pin_mosi = config::kPinTftMosi;
      cfg.pin_miso = config::kPinTftMiso;
      cfg.pin_dc = config::kPinTftDc;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = config::kPinTftCs;
      cfg.pin_rst = config::kPinTftRst;
      cfg.pin_busy = -1;
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.readable = false;
      cfg.bus_shared = false;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dummy_read_pixel = 16;
      cfg.dummy_read_bits = 1;
      _panel.config(cfg);
    }
    {
      auto cfg = _light.config();
      cfg.pin_bl = config::kPinTftBacklight;
      cfg.invert = false;
      cfg.freq = 12000;
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }
    {
      auto cfg = _touch.config();
      // Raw XPT2046 limits (before LovyanGFX applies the landscape rotation).
      cfg.x_min = 240;
      cfg.x_max = 3800;
      cfg.y_min = 200;
      cfg.y_max = 3700;
      cfg.pin_int = -1;  // IRQ flutuante no CYD gera toque fantasma; usar pressão
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
      cfg.spi_host = SPI3_HOST;  // VSPI
      cfg.freq = 1000000;
      cfg.pin_sclk = config::kPinTouchSclk;
      cfg.pin_mosi = config::kPinTouchMosi;
      cfg.pin_miso = config::kPinTouchMiso;
      cfg.pin_cs = config::kPinTouchCs;
      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
    setPanel(&_panel);
  }
};
