/**
 * @file demo_theme.h
 *
 */

#ifndef LV_DEMO_PRINTER_THEME_H
#define LV_DEMO_PRINTER_THEME_H
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
// #include "lv_demo_printer.h"

#if LV_USE_DEMO_PRINTER
/*********************
 *      DEFINES
 *********************/
/*Colors*/
#define LV_DEMO_PRINTER_WHITE           lv_color_hex(0xffffff)
#define LV_DEMO_PRINTER_LIGHT           lv_color_hex(0xf3f8fe)
#define LV_DEMO_PRINTER_GRAY            lv_color_hex(0x8a8a8a)
#define LV_DEMO_PRINTER_LIGHT_GRAY      lv_color_hex(0xc4c4c4)
#define LV_DEMO_PRINTER_BLUE            lv_color_hex(0x2f3243) //006fb6
#define LV_DEMO_PRINTER_GREEN           lv_color_hex(0x4cb242)
#define LV_DEMO_PRINTER_RED             lv_color_hex(0xd51732)

#define LV_DEMO_PRINTER_BG_NONE (-LV_VER_RES)
#define LV_DEMO_PRINTER_BG_FULL 0
#define LV_DEMO_PRINTER_BG_NORMAL (-2 * (LV_VER_RES / 3))
#define LV_DEMO_PRINTER_BG_SMALL (-5 * (LV_VER_RES / 6))

/*Animations*/
#define LV_DEMO_PRINTER_ANIM_Y (LV_VER_RES / 20)
#define LV_DEMO_PRINTER_ANIM_DELAY (40)
#define LV_DEMO_PRINTER_ANIM_TIME  (150)
#define LV_DEMO_PRINTER_ANIM_TIME_BG  (300)

/*Padding*/
#define LV_DEMO_PRINTER_TITLE_PAD 20
/*Sizes*/
#define LV_DEMO_PRINTER_BTN_H   (50)
#define LV_DEMO_PRINTER_BTN_W   (200)

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    LV_DEMO_PRINTER_THEME_TITLE = _LV_THEME_BUILTIN_LAST,
    LV_DEMO_PRINTER_THEME_LABEL_WHITE,
    LV_DEMO_PRINTER_THEME_ICON,
    LV_DEMO_PRINTER_THEME_BTN_BORDER,
    LV_DEMO_PRINTER_THEME_BTN_CIRCLE,
    LV_DEMO_PRINTER_THEME_BOX_BORDER,
    LV_DEMO_PRINTER_THEME_BTN_BACK
}lv_demo_printer_theme_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
// TODO: calculate screen, split into parts
#define LV_TITLE_OVERLAP  60
#define  APP_BOX_WIDTH 720   // MAX 800, 720
#define  APP_BOX_HEIGHT 260   // MAX 480, 260

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t * lv_demo_printer_theme_init(lv_color_t color_primary, lv_color_t color_secondary, uint32_t flags,
                                    const lv_font_t * font_small, const lv_font_t * font_normal, const lv_font_t * font_subtitle,
                                    const lv_font_t * font_title);
/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DEMO_PRINTER*/

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
