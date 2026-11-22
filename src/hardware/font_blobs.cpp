// VLW fonts embedded by board_build.embed_files (platformio.ini).
#include "hardware/display_font.h"

#define UI_FONT_SYMBOLS(n)                                                        \
  extern const uint8_t _binary_data_font_##n##_vlw_start[] asm(                   \
      "_binary_data_font_" #n "_vlw_start");                                      \
  extern const uint8_t _binary_data_font_##n##_vlw_end[] asm(                     \
      "_binary_data_font_" #n "_vlw_end");

extern "C" {
UI_FONT_SYMBOLS(xs)
UI_FONT_SYMBOLS(s)
UI_FONT_SYMBOLS(m)
UI_FONT_SYMBOLS(l)
UI_FONT_SYMBOLS(xl)
UI_FONT_SYMBOLS(num)
}

const uint8_t* uiFontBlob(int index, size_t* len) {
  const uint8_t* s = nullptr;
  const uint8_t* e = nullptr;
  switch (index) {
    case 0: s = _binary_data_font_xs_vlw_start; e = _binary_data_font_xs_vlw_end; break;
    case 1: s = _binary_data_font_s_vlw_start; e = _binary_data_font_s_vlw_end; break;
    case 2: s = _binary_data_font_m_vlw_start; e = _binary_data_font_m_vlw_end; break;
    case 3: s = _binary_data_font_l_vlw_start; e = _binary_data_font_l_vlw_end; break;
    case 4: s = _binary_data_font_xl_vlw_start; e = _binary_data_font_xl_vlw_end; break;
    case 5: s = _binary_data_font_num_vlw_start; e = _binary_data_font_num_vlw_end; break;
    default: break;
  }
  if (len) *len = (s && e) ? static_cast<size_t>(e - s) : 0;
  return s;
}
