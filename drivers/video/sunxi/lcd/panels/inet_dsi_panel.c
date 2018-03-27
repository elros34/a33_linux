#include "inet_dsi_panel.h"
#include "panels.h"
#include <mach/sys_config.h>

//https://github.com/BPI-SINOVOIP/BPI-M2U-bsp/blob/master/linux-sunxi/drivers/video/sunxi/disp2/disp/lcd/inet_dsi_panel.c

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

#define panel_reset(val) sunxi_lcd_gpio_set_value(sel,    1, val)
#define power_en(val) sunxi_lcd_gpio_set_value(sel, 0, val)


static void LCD_cfg_panel_info(panel_extend_para * info)
{
    u32 i = 0, j=0;
    u32 items;
    u8 lcd_gamma_tbl[][2] =
    {
        //{input value, corrected value}
        {0, 0},
        {15, 15},
        {30, 30},
        {45, 45},
        {60, 60},
        {75, 75},
        {90, 90},
        {105, 105},
        {120, 120},
        {135, 135},
        {150, 150},
        {165, 165},
        {180, 180},
        {195, 195},
        {210, 210},
        {225, 225},
        {240, 240},
        {255, 255},
    };

    u8 lcd_bright_curve_tbl[][2] =
    {
        //{input value, corrected value}
        {0    ,0  },//0
        {15   ,3  },//0
        {30   ,6  },//0
        {45   ,9  },// 1
        {60   ,12  },// 2
        {75   ,16  },// 5
        {90   ,22  },//9
        {105   ,28 }, //15
        {120  ,36 },//23
        {135  ,44 },//33
        {150  ,54 },
        {165  ,67 },
        {180  ,84 },
        {195  ,108},
        {210  ,137},
        {225 ,171},
        {240 ,210},
        {255 ,255},
    };

    u32 lcd_cmap_tbl[2][3][4] = {
    {
        {LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
        {LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
        {LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
        },
        {
        {LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
        {LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
        {LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
        },
    };

    memset(info,0,sizeof(panel_extend_para));

    items = sizeof(lcd_gamma_tbl)/2;
    for(i=0; i<items-1; i++) {
        u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

        for(j=0; j<num; j++) {
            u32 value = 0;

            value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
            info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
        }
    }
    info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

    items = sizeof(lcd_bright_curve_tbl)/2;
    for(i=0; i<items-1; i++) {
        u32 num = lcd_bright_curve_tbl[i+1][0] - lcd_bright_curve_tbl[i][0];

        for(j=0; j<num; j++) {
            u32 value = 0;

            value = lcd_bright_curve_tbl[i][1] + ((lcd_bright_curve_tbl[i+1][1] - lcd_bright_curve_tbl[i][1]) * j)/num;
            info->lcd_bright_curve_tbl[lcd_bright_curve_tbl[i][0] + j] = value;
        }
    }
    info->lcd_bright_curve_tbl[255] = lcd_bright_curve_tbl[items-1][1];

    memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{
    LCD_OPEN_FUNC(sel, LCD_power_on, 20);   //open lcd power, and delay 50ms
    LCD_OPEN_FUNC(sel, LCD_panel_init, 100);   //open lcd power, than delay 200ms
    LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 150);     //open lcd controller, and delay 100ms
    LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

    return 0;
}

static s32 LCD_close_flow(u32 sel)
{
    LCD_CLOSE_FUNC(sel, LCD_bl_close, 200);       //close lcd backlight, and delay 0ms
    LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 10);         //close lcd controller, and delay 0ms
    LCD_CLOSE_FUNC(sel, LCD_panel_exit,	200);   //open lcd power, than delay 200ms
    LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms

    return 0;
}

static void LCD_power_on(u32 sel)
{
    sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power
    sunxi_lcd_delay_ms(5);
    sunxi_lcd_power_enable(sel, 1);//config lcd_power pin to open lcd power1
    sunxi_lcd_delay_ms(5);
//    sunxi_lcd_power_enable(sel, 2);//config lcd_power pin to open lcd power2
//    sunxi_lcd_delay_ms(5);
    power_en(1);
    sunxi_lcd_delay_ms(20);
    panel_reset(1);
    sunxi_lcd_delay_ms(5);
    sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
    sunxi_lcd_pin_cfg(sel, 0);
    power_en(0);
    sunxi_lcd_delay_ms(20);
    panel_reset(0);
//    sunxi_lcd_delay_ms(5);
//    sunxi_lcd_power_disable(sel, 2);//config lcd_power pin to close lcd power2
    sunxi_lcd_delay_ms(5);
    sunxi_lcd_power_disable(sel, 1);//config lcd_power pin to close lcd power1
    sunxi_lcd_delay_ms(5);
    sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power
}

static void LCD_bl_open(u32 sel)
{
    sunxi_lcd_pwm_enable(sel);
    sunxi_lcd_delay_ms(50);
    sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(u32 sel)
{
    sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
    sunxi_lcd_delay_ms(20);
    sunxi_lcd_pwm_disable(sel);
}

#define REGFLAG_DELAY             						0XFE
#define REGFLAG_END_OF_TABLE      						0xFF   // END OF REGISTERS MARKER

struct LCM_setting_table {
    u8 cmd;
    u32 count;
    u8 para_list[64];
};

static const struct LCM_setting_table LCM_LT080B21BA100_setting[] = {
    {0xe0,    1,    {0x00}},
    {0xe1,    1,    {0x93}},
    {0xe2,    1,    {0x65}},
    {0xe3,    1,    {0xf8}},
    {0xe0,    1,    {0x00}},
    {0x70,    1,    {0x02}},
    {0x71,    1,    {0x23}},
    {0x72,    1,    {0x06}},
    {0xe0,    1,    {0x01}},
    {0x00,    1,    {0x00}},
    {0x01,    1,    {0xa0}},
    {0x03,    1,    {0x00}},
    {0x04,    1,    {0xa0}},
    {0x17,    1,    {0x00}},
    {0x18,    1,    {0xb1}},
    {0x19,    1,    {0x00}},
    {0x1a,    1,    {0x00}},
    {0x1b,    1,    {0xb1}},
    {0x1c,    1,    {0x00}},
    {0x1f,    1,    {0x48}},
    {0x20,    1,    {0x23}},
    {0x21,    1,    {0x23}},
    {0x22,    1,    {0x0e}},
    {0x23,    1,    {0x00}},
    {0x24,    1,    {0x38}},
    {0x26,    1,    {0xd3}},
    {0x37,    1,    {0x59}},
    {0x38,    1,    {0x05}},
    {0x39,    1,    {0x08}},
    {0x3a,    1,    {0x12}},
    {0x3c,    1,    {0x78}},
    {0x3e,    1,    {0x80}},
    {0x3f,    1,    {0x80}},
    {0x40,    1,    {0x06}},
    {0x41,    1,    {0xa0}},
    {0x55,    1,    {0x0f}},
    {0x56,    1,    {0x01}},
    {0x57,    1,    {0x85}},
    {0x58,    1,    {0x0a}},
    {0x59,    1,    {0x0a}},
    {0x5a,    1,    {0x32}},
    {0x5b,    1,    {0x0f}},
    {0x5d,    1,    {0x7c}},
    {0x5e,    1,    {0x62}},
    {0x5f,    1,    {0x50}},
    {0x60,    1,    {0x42}},
    {0x61,    1,    {0x3d}},
    {0x62,    1,    {0x2d}},
    {0x63,    1,    {0x30}},
    {0x64,    1,    {0x19}},
    {0x65,    1,    {0x30}},
    {0x66,    1,    {0x2e}},
    {0x67,    1,    {0x2d}},
    {0x68,    1,    {0x4a}},
    {0x69,    1,    {0x3a}},
    {0x6a,    1,    {0x43}},
    {0x6b,    1,    {0x37}},
    {0x6c,    1,    {0x37}},
    {0x6d,    1,    {0x2d}},
    {0x6e,    1,    {0x1f}},
    {0x6f,    1,    {0x00}},
    {0x70,    1,    {0x7c}},
    {0x71,    1,    {0x62}},
    {0x72,    1,    {0x50}},
    {0x73,    1,    {0x42}},
    {0x74,    1,    {0x3d}},
    {0x75,    1,    {0x2d}},
    {0x76,    1,    {0x30}},
    {0x77,    1,    {0x19}},
    {0x78,    1,    {0x30}},
    {0x79,    1,    {0x2e}},
    {0x7a,    1,    {0x2d}},
    {0x7b,    1,    {0x4a}},
    {0x7c,    1,    {0x3a}},
    {0x7d,    1,    {0x43}},
    {0x7e,    1,    {0x37}},
    {0x7f,    1,    {0x37}},
    {0x80,    1,    {0x2d}},
    {0x81,    1,    {0x1f}},
    {0x82,    1,    {0x00}},
    {0xe0,    1,    {0x02}},
    {0x00,    1,    {0x1f}},
    {0x01,    1,    {0x1f}},
    {0x02,    1,    {0x13}},
    {0x03,    1,    {0x11}},
    {0x04,    1,    {0x0b}},
    {0x05,    1,    {0x0b}},
    {0x06,    1,    {0x09}},
    {0x07,    1,    {0x09}},
    {0x08,    1,    {0x07}},
    {0x09,    1,    {0x1f}},
    {0x0a,    1,    {0x1f}},
    {0x0b,    1,    {0x1f}},
    {0x0c,    1,    {0x1f}},
    {0x0d,    1,    {0x1f}},
    {0x0e,    1,    {0x1f}},
    {0x0f,    1,    {0x07}},
    {0x10,    1,    {0x05}},
    {0x11,    1,    {0x05}},
    {0x12,    1,    {0x01}},
    {0x13,    1,    {0x03}},
    {0x14,    1,    {0x1f}},
    {0x15,    1,    {0x1f}},
    {0x16,    1,    {0x1f}},
    {0x17,    1,    {0x1f}},
    {0x18,    1,    {0x12}},
    {0x19,    1,    {0x10}},
    {0x1a,    1,    {0x0a}},
    {0x1b,    1,    {0x0a}},
    {0x1c,    1,    {0x08}},
    {0x1d,    1,    {0x08}},
    {0x1e,    1,    {0x06}},
    {0x1f,    1,    {0x1f}},
    {0x20,    1,    {0x1f}},
    {0x21,    1,    {0x1f}},
    {0x22,    1,    {0x1f}},
    {0x23,    1,    {0x1f}},
    {0x24,    1,    {0x1f}},
    {0x25,    1,    {0x06}},
    {0x26,    1,    {0x04}},
    {0x27,    1,    {0x04}},
    {0x28,    1,    {0x00}},
    {0x29,    1,    {0x02}},
    {0x2a,    1,    {0x1f}},
    {0x2b,    1,    {0x1f}},
    {0x2c,    1,    {0x1f}},
    {0x2d,    1,    {0x1f}},
    {0x2e,    1,    {0x00}},
    {0x2f,    1,    {0x02}},
    {0x30,    1,    {0x08}},
    {0x31,    1,    {0x08}},
    {0x32,    1,    {0x0a}},
    {0x33,    1,    {0x0a}},
    {0x34,    1,    {0x04}},
    {0x35,    1,    {0x1f}},
    {0x36,    1,    {0x1f}},
    {0x37,    1,    {0x1f}},
    {0x38,    1,    {0x1f}},
    {0x39,    1,    {0x1f}},
    {0x3a,    1,    {0x1f}},
    {0x3b,    1,    {0x04}},
    {0x3c,    1,    {0x06}},
    {0x3d,    1,    {0x06}},
    {0x3e,    1,    {0x12}},
    {0x3f,    1,    {0x10}},
    {0x40,    1,    {0x1f}},
    {0x41,    1,    {0x1f}},
    {0x42,    1,    {0x1f}},
    {0x43,    1,    {0x1f}},
    {0x44,    1,    {0x01}},
    {0x45,    1,    {0x03}},
    {0x46,    1,    {0x09}},
    {0x47,    1,    {0x09}},
    {0x48,    1,    {0x0b}},
    {0x49,    1,    {0x0b}},
    {0x4a,    1,    {0x05}},
    {0x4b,    1,    {0x1f}},
    {0x4c,    1,    {0x1f}},
    {0x4d,    1,    {0x1f}},
    {0x4e,    1,    {0x1f}},
    {0x4f,    1,    {0x1f}},
    {0x50,    1,    {0x1f}},
    {0x51,    1,    {0x05}},
    {0x52,    1,    {0x07}},
    {0x53,    1,    {0x07}},
    {0x54,    1,    {0x13}},
    {0x55,    1,    {0x11}},
    {0x56,    1,    {0x1f}},
    {0x57,    1,    {0x1f}},
    {0x58,    1,    {0x40}},
    {0x59,    1,    {0x00}},
    {0x5a,    1,    {0x00}},
    {0x5b,    1,    {0x30}},
    {0x5c,    1,    {0x02}},
    {0x5d,    1,    {0x30}},
    {0x5e,    1,    {0x01}},
    {0x5f,    1,    {0x02}},
    {0x60,    1,    {0x30}},
    {0x61,    1,    {0x01}},
    {0x62,    1,    {0x02}},
    {0x63,    1,    {0x03}},
    {0x64,    1,    {0x6b}},
    {0x65,    1,    {0x75}},
    {0x66,    1,    {0x08}},
    {0x67,    1,    {0x73}},
    {0x68,    1,    {0x02}},
    {0x69,    1,    {0x03}},
    {0x6a,    1,    {0x6b}},
    {0x6b,    1,    {0x07}},
    {0x6c,    1,    {0x00}},
    {0x6d,    1,    {0x04}},
    {0x6e,    1,    {0x04}},
    {0x6f,    1,    {0x88}},
    {0x70,    1,    {0x00}},
    {0x71,    1,    {0x00}},
    {0x72,    1,    {0x06}},
    {0x73,    1,    {0x7b}},
    {0x74,    1,    {0x00}},
    {0x75,    1,    {0x3c}},
    {0x76,    1,    {0x00}},
    {0x77,    1,    {0x0d}},
    {0x78,    1,    {0x2c}},
    {0x79,    1,    {0x00}},
    {0x7a,    1,    {0x00}},
    {0x7b,    1,    {0x00}},
    {0x7c,    1,    {0x00}},
    {0x7d,    1,    {0x03}},
    {0x7e,    1,    {0x7b}},
    {0xe0,    1,    {0x01}},
    {0x0e,    1,    {0x01}},
    {0xe0,    1,    {0x03}},
    {0x98,    1,    {0x2f}},
#ifdef LT080B21BA94
    {0xE0,    1,    {0x04}},//esd
    {0x2B,    1,    {0x2B}},//esd
    {0x2E,    1,    {0x44}},//esd
#endif
    {0xe0,    1,     {0x00}},
    {0x11,   0,     {0x00}},
    {REGFLAG_DELAY, REGFLAG_DELAY, {200}},
    {0x29,   0,     {0x00}},
    {REGFLAG_DELAY, REGFLAG_DELAY, {5}},
    {REGFLAG_END_OF_TABLE, REGFLAG_END_OF_TABLE,    {0x00}},
};

static void LCD_panel_init(u32 sel)
{
    u32 i;
    script_item_u val;
    script_item_value_type_e type;

    type = script_get_item("lcd0_para", "lcd_model_name", &val);

    if (type != SCIRPT_ITEM_VALUE_TYPE_STR) {
        pr_info("fetch lcd_model_name from sys_config failed\n");
    } else {
        pr_info("lcd_model_name: %s\n", val.str);

        sunxi_lcd_dsi_clk_enable(sel);
        sunxi_lcd_delay_ms(20);
        dsi_dcs_wr_0para(sel, DSI_DCS_SOFT_RESET);
        sunxi_lcd_delay_ms(10);

        if (strcmp("LT080B21BA100", val.str) != 0) {
            pr_info("No suitable config found for %s\n", val.str);
            return;
        }

        for(i=0;;i++)
        {
            if(LCM_LT080B21BA100_setting[i].count == REGFLAG_END_OF_TABLE)
                break;
            else if (LCM_LT080B21BA100_setting[i].count == REGFLAG_DELAY)
                sunxi_lcd_delay_ms(LCM_LT080B21BA100_setting[i].para_list[0]);
            else
                dsi_dcs_wr(sel,
                           LCM_LT080B21BA100_setting[i].cmd,
                           LCM_LT080B21BA100_setting[i].para_list,
                           LCM_LT080B21BA100_setting[i].count);
        }
    }
}

static void LCD_panel_exit(u32 sel)
{

    dsi_dcs_wr_0para(sel, DSI_DCS_SET_DISPLAY_OFF);
    sunxi_lcd_delay_ms(20);
    dsi_dcs_wr_0para(sel, DSI_DCS_ENTER_SLEEP_MODE);
    sunxi_lcd_delay_ms(80);

    return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
    return 0;
}

__lcd_panel_t inet_dsi_panel = {
    /* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
    .name = "inet_dsi_panel",
    .func = {
        .cfg_panel_info = LCD_cfg_panel_info,
        .cfg_open_flow = LCD_open_flow,
        .cfg_close_flow = LCD_close_flow,
        .lcd_user_defined_func = LCD_user_defined_func,
    },
};
