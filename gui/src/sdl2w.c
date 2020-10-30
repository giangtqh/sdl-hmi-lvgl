#include "sdl2w.h"

#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"

#include <time.h>
#include <stdlib.h>
#include <unistd.h> // sleep
/*********************
 *      DEFINES
 *********************/
#define GAUGE_SCALE_ANGLE 280
#define GAUSE_MAX_SPEED 240
#define GAUSE_SCALE_UNIT 10
#define GAUSE_CRITICAL_SPEED 200
#define LV_DEMO_PRINTER_BG_NORMAL (-2 * (LV_VER_RES / 3))
#define LV_DEMO_PRINTER_BG_SMALL (-5 * (LV_VER_RES / 6))
// number of the scale lines
const int GAUGE_SCALE_LINE_NB = (GAUSE_MAX_SPEED / GAUSE_SCALE_UNIT) + 1;
// number of the scale label
const int GAUGE_SCALE_LABEL_NB = (GAUSE_MAX_SPEED / (GAUSE_SCALE_UNIT*2)) + 1;


static void hal_init(void);
static int tick_thread(void *data);
static void memory_monitor(lv_task_t *param);

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics
 * library
 */
static void hal_init(void) {
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();

    /*Create a display buffer*/
    static lv_disp_buf_t disp_buf1;
    static lv_color_t buf1_1[LV_HOR_RES_MAX * 120];
    lv_disp_buf_init(&disp_buf1, buf1_1, NULL, LV_HOR_RES_MAX * 120);

    /*Create a display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.buffer = &disp_buf1;
    disp_drv.flush_cb = monitor_flush;
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse as input device
    * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv.read_cb = mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv);

    /* Tick init.
    * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
    * how much time were elapsed Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);

    /* Optional:
    * Create a memory monitor task which prints the memory usage in
    * periodically.*/
    lv_task_create(memory_monitor, 5000, LV_TASK_PRIO_MID, NULL);
}


static void gauge_anim(lv_obj_t * gauge, lv_anim_value_t value)
{
    lv_gauge_set_value(gauge, 0, value);

    static char buf[64];
    lv_snprintf(buf, sizeof(buf), "%d", value);
    lv_obj_t * label = lv_obj_get_child(gauge, NULL);
    lv_label_set_text(label, buf);
    lv_obj_align(label, gauge, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_y(label));
}

static void gauge2_anim(lv_obj_t * gauge, lv_anim_value_t value)
{
    lv_gauge_set_value(gauge, 0, value);

    static char buf[64];
    lv_snprintf(buf, sizeof(buf), "%d", value);
    lv_obj_t * label = lv_obj_get_child(gauge, NULL);
    lv_label_set_text(label, buf);
    lv_obj_align(label, gauge, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_y(label));
}

/**
 * Calculate the current value of an animation applying linear characteristic
 * @param a pointer to an animation
 * @return the current value to set
 */
static lv_anim_value_t anim_path_random(const lv_anim_path_t * path, const lv_anim_t * a)
{
    /*Calculate the current step*/
    static int i = 0;
    static int cnt = 100;
    if (i < cnt) {
      ++i;
    } else if (i > cnt) {
      i-=3;
    } else {
      cnt = (rand() % (GAUSE_CRITICAL_SPEED)) + 30;
    }

    return (lv_anim_value_t)i;
}

void demo_sdl2w(void) {

        /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();

    // Use current time as
    // seed for random generator
    srand(time(0));

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_value_align(&style, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    // lv_style_set_value_ofs_y(&style, LV_STATE_DEFAULT, - LV_DPX(10));
    // lv_style_set_margin_top(&style, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_bg_opa(&style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_pad_inner(&style, LV_STATE_DEFAULT, 13);

    lv_style_set_text_font(&style, LV_STATE_DEFAULT, &lv_font_montserrat_12);  /*Set a smaller font*/

    lv_style_set_scale_end_color(&style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_line_color(&style, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    // lv_style_set_scale_grad_color(&style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_line_width(&style, LV_STATE_DEFAULT, 1);
    lv_style_set_scale_end_line_width(&style, LV_STATE_DEFAULT, 4);
    lv_style_set_scale_end_border_width(&style, LV_STATE_DEFAULT, 4);

    static lv_style_t gauge_major_style;
    lv_style_init(&gauge_major_style);
    lv_style_set_value_align(&gauge_major_style, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&gauge_major_style, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&gauge_major_style, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_line_width(&gauge_major_style, LV_STATE_DEFAULT, 4);
    lv_style_set_scale_end_line_width(&gauge_major_style, LV_STATE_DEFAULT, 4);
    lv_style_set_scale_end_border_width(&gauge_major_style, LV_STATE_DEFAULT, 4);
    lv_style_set_line_color(&gauge_major_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    // lv_style_set_scale_grad_color(&gauge_major_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

    static lv_style_t needle_style;
    lv_style_init(&needle_style);
    lv_style_set_value_align(&needle_style, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    // make the needle smaller/shorter than the outer radius
    lv_style_set_pad_inner(&needle_style, LV_STATE_DEFAULT, 10);
    lv_style_set_line_width(&needle_style, LV_STATE_DEFAULT, 4);
    lv_style_set_line_color(&needle_style, LV_STATE_DEFAULT, LV_COLOR_RED);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_values(&a, 0, GAUSE_MAX_SPEED);

    lv_anim_path_t path;
    lv_anim_path_init(&path);
    lv_anim_path_set_cb(&path, anim_path_random);

    lv_anim_set_time(&a, 4000);
    lv_anim_set_playback_time(&a, 1000);
    // lv_anim_set_playback_delay(&a, 500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
    /*Gauge has a needle but for simplicity its style is not initialized here*/

    /*Create an object with the new style*/
    lv_obj_t * gauge = lv_gauge_create(lv_scr_act(), NULL);
    lv_obj_set_size(gauge, 200, 200);
    lv_obj_set_pos(gauge, 50, 50);
    lv_obj_add_style(gauge, LV_GAUGE_PART_MAIN, &style);
    lv_obj_add_style(gauge,  LV_GAUGE_PART_MAJOR  , &gauge_major_style);
    // lv_obj_align(gauge, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
    lv_gauge_set_scale(gauge, GAUGE_SCALE_ANGLE, GAUGE_SCALE_LINE_NB, GAUGE_SCALE_LABEL_NB);
    lv_gauge_set_range(gauge, 0, GAUSE_MAX_SPEED);
    lv_gauge_set_critical_value(gauge, GAUSE_CRITICAL_SPEED);

    lv_obj_t * label = lv_label_create(gauge, NULL);
    lv_obj_align(label, gauge, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());

    lv_anim_set_var(&a, gauge);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)gauge_anim);
    lv_anim_start(&a);


    // Create another gauge
    lv_obj_t * gauge2 = lv_gauge_create(lv_scr_act(), gauge);
    lv_obj_set_pos(gauge2, 300, 50);
    lv_obj_add_style(gauge2,   LV_GAUGE_PART_MAIN   , &style);
    lv_obj_add_style(gauge2,  LV_GAUGE_PART_MAJOR  , &gauge_major_style);
    lv_obj_add_style(gauge2,  LV_GAUGE_PART_NEEDLE   , &needle_style);
    // lv_obj_align(gauge2, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    lv_obj_set_size(gauge2, 200, 200);
    lv_gauge_set_scale(gauge2, GAUGE_SCALE_ANGLE, GAUGE_SCALE_LINE_NB, GAUGE_SCALE_LABEL_NB);
    lv_gauge_set_range(gauge2, 0, GAUSE_MAX_SPEED);
    lv_gauge_set_critical_value(gauge2, GAUSE_CRITICAL_SPEED);

    label = lv_label_create(gauge2, NULL);
    lv_obj_align(label, gauge2, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());

    lv_anim_set_var(&a, gauge2);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)gauge2_anim);
    lv_anim_set_path(&a, &path);
    lv_anim_start(&a);

    while (1) {
        /* Periodically call the lv_task handler.
        * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();
        usleep(5 * 1000);
    }
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
    (void)data;

    while (1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(lv_task_t *param) {
    (void)param; /*Unused*/

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n",
            (int)mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct,
            (int)mon.free_biggest_size);
}
