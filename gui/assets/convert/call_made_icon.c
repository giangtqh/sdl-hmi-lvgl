#include "lvgl/lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_CALL_MADE_ICON
#define LV_ATTRIBUTE_IMG_CALL_MADE_ICON
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_IMG_CALL_MADE_ICON uint8_t call_made_icon_map[] = {
  0x00, 0x00, 0x00, 0x13, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe8, 
  0x00, 0x00, 0x00, 0x08, 0x70, 0x98, 0x97, 0x94, 0xa4, 0xf0, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x94, 0xf7, 0xf0, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x97, 0xf8, 0x94, 0xa4, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x97, 0xf8, 0x97, 0x0c, 0x97, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x97, 0xf8, 0x97, 0x0f, 0x00, 0x97, 0xff, 
  0x00, 0x00, 0x00, 0x0f, 0x97, 0xf8, 0x97, 0x0f, 0x00, 0x00, 0x97, 0xff, 
  0x00, 0x00, 0x0f, 0x97, 0xf8, 0x97, 0x0f, 0x00, 0x00, 0x00, 0x70, 0xcf, 
  0x00, 0x0f, 0x97, 0xf8, 0x97, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x08, 0x13, 
  0x10, 0x94, 0xf8, 0x97, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x8c, 0xf8, 0x94, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x6f, 0x8c, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const lv_img_dsc_t call_made_icon = {
  .header.always_zero = 0,
  .header.w = 12,
  .header.h = 12,
  .data_size = 144,
  .header.cf = LV_IMG_CF_ALPHA_8BIT,
  .data = call_made_icon_map,
};