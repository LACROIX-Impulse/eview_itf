/**
 * @file eviewitf-plot.c
 * @brief Communication API between A53 and R7 CPUs for plot functions
 * @author LACROIX Impulse
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "eviewitf-priv.h"

/******************************************************************************************
 * Private variables
 ******************************************************************************************/

/**
 * @brief Font to pixel
 */
static uint8_t font_basic[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0000 (nul)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0001
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0002
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0003
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0004
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0005
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0006
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0007
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0008
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0009
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 000A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 000B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 000C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 000D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 000E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 000F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0010
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0011
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0012
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0013
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0014
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0015
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0016
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0017
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0018
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0019
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 001A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 001B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 001C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 001D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 001E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 001F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0020 (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},  // 0021 (!)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0022 (")
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},  // 0023 (#)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},  // 0024 ($)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},  // 0025 (%)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},  // 0026 (&)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0027 (')
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},  // 0028 (()
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},  // 0029 ())
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},  // 002A (*)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},  // 002B (+)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},  // 002C (,)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},  // 002D (-)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},  // 002E (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},  // 002F (/)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},  // 0030 (0)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},  // 0031 (1)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},  // 0032 (2)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},  // 0033 (3)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},  // 0034 (4)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},  // 0035 (5)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},  // 0036 (6)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},  // 0037 (7)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},  // 0038 (8)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},  // 0039 (9)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},  // 003A (:)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},  // 003B (;)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},  // 003C (<)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},  // 003D (=)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},  // 003E (>)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},  // 003F (?)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},  // 0040 (@)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},  // 0041 (A)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},  // 0042 (B)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},  // 0043 (C)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},  // 0044 (D)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},  // 0045 (E)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},  // 0046 (F)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},  // 0047 (G)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},  // 0048 (H)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // 0049 (I)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},  // 004A (J)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},  // 004B (K)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},  // 004C (L)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},  // 004D (M)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},  // 004E (N)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},  // 004F (O)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},  // 0050 (P)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},  // 0051 (Q)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},  // 0052 (R)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},  // 0053 (S)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // 0054 (T)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},  // 0055 (U)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},  // 0056 (V)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},  // 0057 (W)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},  // 0058 (X)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},  // 0059 (Y)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},  // 005A (Z)
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},  // 005B ([)
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},  // 005C (\)
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},  // 005D (])
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},  // 005E (^)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},  // 005F (_)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0060 (`)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},  // 0061 (a)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},  // 0062 (b)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},  // 0063 (c)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},  // 0064 (d)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},  // 0065 (e)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},  // 0066 (f)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},  // 0067 (g)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},  // 0068 (h)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // 0069 (i)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},  // 006A (j)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},  // 006B (k)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // 006C (l)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},  // 006D (m)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},  // 006E (n)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},  // 006F (o)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},  // 0070 (p)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},  // 0071 (q)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},  // 0072 (r)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},  // 0073 (s)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},  // 0074 (t)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},  // 0075 (u)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},  // 0076 (v)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},  // 0077 (w)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},  // 0078 (x)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},  // 0079 (y)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},  // 007A (z)
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},  // 007B ({)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},  // 007C (|)
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},  // 007D (})
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 007E (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}   // 007F
};

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
/**
 * @brief Number of bytes for RGB definition.
 */
#define NB_COMPONENTS_RGB (3u)

/**
 * @typedef eviewitf_plot_yuv_color_attributes_t
 * @brief Structure to set an YUV color
 *
 * @struct eviewitf_plot_yuv_color_attributes
 * @brief Structure to set an YUV color
 *
 */
typedef struct eviewitf_plot_yuv_color_attributes {
    uint8_t y; /*!< Y channel value */
    uint8_t u; /*!< U channel value */
    uint8_t v; /*!< V channel value */
} eviewitf_plot_yuv_color_attributes_t;

/**
 * @fn uint8_t get_font_value(int i, int j)
 * @brief Gets a (i,j) font value
 *
 * @param i: Font value row position
 * @param j: Font value column position
 *
 * @return font value
 */
static uint8_t get_font_value(int i, int j) { return font_basic[i][j]; }

/**
 * @fn static void rgb_color_to_yuv_color(eviewitf_plot_rgb_color_attributes_t *rgb,
 * eviewitf_plot_yuv_color_attributes_t *yuv)
 * @brief Converts an RGB color into an YUV one (BT.709 Computer RGB to YUV)
 *
 * @param rgb: RGB color to convert
 * @param yuv: YUV converted color
 *
 */
static void rgb_color_to_yuv_color(eviewitf_plot_rgb_color_attributes_t *rgb,
                                   eviewitf_plot_yuv_color_attributes_t *yuv) {
    int r_val = (int)rgb->red;
    int g_val = (int)rgb->green;
    int b_val = (int)rgb->blue;
    int y_val = 16;
    int u_val = 128;
    int v_val = 128;

    y_val += (47 * r_val + 157 * g_val + 16 * b_val) / 256;
    u_val += (-26 * r_val - 87 * g_val + 112 * b_val) / 256;
    v_val += (112 * r_val - 102 * g_val - 10 * b_val) / 256;

    yuv->y = (uint8_t)y_val;
    yuv->u = (uint8_t)u_val;
    yuv->v = (uint8_t)v_val;
}

/* clang-format off */
/**
 * @fn void set_yuv422sp_pixel(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y, eviewitf_plot_yuv_color_attributes_t color)
 * @brief Sets a pixel to the desired YUV color into an YUV422 semi-planar frame
 *
 * @param frame: YUV422 semi-planar frame attributes pointer
 * @param x: Row pixel position
 * @param y: Column pixel position
 * @param color: YUV pixel color attributes
 *
 */
/* clang-format on */
static void set_yuv422sp_pixel(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y,
                               eviewitf_plot_yuv_color_attributes_t color) {
    uint32_t idx = 0;

    if ((x % 2u) != 0u) {
        return;
    }

    if (x > frame->width || y > frame->height) return;

    idx = x + (y * frame->width);

    /* Y values */
    frame->buffer[idx] = color.y;
    frame->buffer[idx + 1u] = color.y;

    idx += frame->width * frame->height;

    /* U value */
    frame->buffer[idx] = color.u;
    /* V value */
    frame->buffer[idx + 1u] = color.v;
}

/* clang-format off */
/**
 * @fn void set_rgb888il_pixel(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y, eviewitf_plot_rgb_color_attributes_t color)
 * @brief Sets a pixel to the desired RGB color into an RGB888 interleave frame
 *
 * @param frame: RGB888 interleave frame attributes pointer
 * @param x: Row pixel position
 * @param y: Column pixel position
 * @param color: RGB pixel color attributes
 *
 */
/* clang-format on */
static void set_rgb888il_pixel(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y,
                               eviewitf_plot_rgb_color_attributes_t color) {
    uint32_t idx = NB_COMPONENTS_RGB * (x + y * frame->width);
    if (x > frame->width || y > frame->height) return;
    /* R value */
    frame->buffer[idx++] = color.red;
    /* G value */
    frame->buffer[idx++] = color.green;
    /* B value */
    frame->buffer[idx++] = color.blue;
}

/* clang-format off */
/**
 * @fn eviewitf_ret_t set_pixel(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y, eviewitf_plot_rgb_color_attributes_t color)
 * @brief Sets a pixel to the desired RGB color into a frame
 *
 * @param frame: Frame attributes pointer
 * @param x: Row pixel position
 * @param y: Column pixel position
 * @param color: RGB pixel color attributes
 *
 */
/* clang-format on */
static eviewitf_ret_t set_pixel(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y,
                                eviewitf_plot_rgb_color_attributes_t color) {
    eviewitf_ret_t ret = EVIEWITF_INVALID_PARAM;
    if (frame->format == EVIEWITF_PLOT_FRAME_FORMAT_YUV422SP) {
        eviewitf_plot_yuv_color_attributes_t yuv_color;
        rgb_color_to_yuv_color(&color, &yuv_color);
        set_yuv422sp_pixel(frame, x, y, yuv_color);
        ret = EVIEWITF_OK;
    } else if (frame->format == EVIEWITF_PLOT_FRAME_FORMAT_RGB888IL) {
        set_rgb888il_pixel(frame, x, y, color);
        ret = EVIEWITF_OK;
    }

    return ret;
}

/* clang-format off */
/**
 * @fn void plot_yuv422sp_h_line(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y, eviewitf_plot_yuv_color_attributes_t color)
 * @brief Plots an horizontal line to the desired YUV color into an YUV422 semi-planar frame
 *
 * @param frame: YUV422 semi-planar frame attributes pointer
 * @param x: Row pixel position
 * @param len: Line length
 * @param y: Column pixel position
 * @param color: YUV pixel color attributes
 *
 */
/* clang-format on */
static void plot_yuv422sp_h_line(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y,
                                 eviewitf_plot_yuv_color_attributes_t color) {
    uint8_t uv_color[2] = {color.u, color.v};
    uint32_t idx = 0;

    if ((x % 2u) != 0u) x++;

    if (x > frame->width || y > frame->height) return;

    idx = x + y * frame->width;

    /* Y values */
    memset(&frame->buffer[idx], color.y, len);

    idx += frame->width * frame->height;

    for (uint32_t xx = 0; xx < len; xx += 2u) {
        /* UV values */
        memcpy(&frame->buffer[idx], uv_color, sizeof(uint16_t));
        idx += 2u;
    }
}

/* clang-format off */
/**
 * @fn void plot_rgb888il_h_line(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y, eviewitf_plot_rgb_color_attributes_t color)
 * @brief Plots an horizontal line to the desired RGB color into an RGB888 interleave frame
 *
 * @param frame: RGB888 interleave frame attributes pointer
 * @param x: Row pixel position
 * @param len: Line length
 * @param y: Column pixel position
 * @param color: RGB pixel color attributes
 *
 */
/* clang-format on */
static void plot_rgb888il_h_line(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y,
                                 eviewitf_plot_rgb_color_attributes_t color) {
    for (uint32_t xx = x; xx < x + len; xx++) {
        set_rgb888il_pixel(frame, xx, y, color);
    }
}

/* clang-format off */
/**
 * @fn uint32_t plot_char(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y, char c, uint32_t sz, eviewitf_plot_rgb_color_attributes_t color, uint8_t disp)
 * @brief Plots a character in the desired RGB color into a frame
 *
 * @param frame: Frame attributes pointer
 * @param x: Row pixel position
 * @param y: Column pixel position
 * @param c: Character to plot
 * @param sz: Character size
 * @param color: RGB pixel color attributes
 * @param disp: Will be plotted if disp is not equal to 0
 *
 * @return The row position after writting the character
 */
/* clang-format on */
static uint32_t plot_char(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t y, char c, uint32_t sz,
                          eviewitf_plot_rgb_color_attributes_t color, uint8_t disp) {
    uint32_t x_min = 5000u;
    uint32_t x_max = 0u;
    uint32_t y_min = 5000u;
    uint32_t y_max = 0u;

    uint32_t max_x = x;
    uint32_t cnt_row = 0;
    for (uint8_t l = 0u; l < 8u; l++) {
        uint8_t f = get_font_value((int)c, l);
        uint32_t cnt_col = 0;
        for (uint8_t i = 0u; i < 8u; i++) {
            uint8_t v = ((((uint8_t)f) >> i) & 1u);
            for (uint32_t incx = 0; incx < sz; incx++) {
                for (uint32_t incy = 0; incy < sz; incy++) {
                    if (v > 0u && disp > 0u) {
                        if (x_min > x + cnt_col + incx) x_min = x + cnt_col + incx;
                        if (x_max < x + cnt_col + incx) x_max = x + cnt_col + incx;
                        if (y_min > y + cnt_row + incy) y_min = y + cnt_row + incy;
                        if (y_max < y + cnt_row + incy) y_max = y + cnt_row + incy;

                        set_pixel(frame, x + cnt_col + incx, y + cnt_row + incy, color);
                    }
                    max_x = x + cnt_col + incx;
                }
            }
            cnt_col += sz;
        }
        cnt_row += sz;
    }

    return max_x;
}

/* clang-format off */
/**
 * @fn eviewitf_ret_t plot_h_line(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y, eviewitf_plot_rgb_color_attributes_t color)
 * @brief Plots an horizontal line to the desired RGB color into a frame
 *
 * @param frame: Frame attributes pointer
 * @param x: Row pixel position
 * @param len: Line length
 * @param y: Column pixel position
 * @param color: RGB pixel color attributes
 *
 */
/* clang-format on */
static eviewitf_ret_t plot_h_line(eviewitf_plot_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y,
                                  eviewitf_plot_rgb_color_attributes_t color) {
    eviewitf_ret_t ret = EVIEWITF_INVALID_PARAM;
    if (frame->format == EVIEWITF_PLOT_FRAME_FORMAT_YUV422SP) {
        eviewitf_plot_yuv_color_attributes_t yuv_color;
        rgb_color_to_yuv_color(&color, &yuv_color);
        plot_yuv422sp_h_line(frame, x, len, y, yuv_color);
        ret = EVIEWITF_OK;
    } else if (frame->format == EVIEWITF_PLOT_FRAME_FORMAT_RGB888IL) {
        plot_rgb888il_h_line(frame, x, len, y, color);
        ret = EVIEWITF_OK;
    }
    return ret;
}

/**
 * @fn get_str_length(eviewitf_plot_text_attributes_t *text)
 * @brief Gets the string length in pixel
 *
 * @param text: Text attributes pointer
 *
 * @return The string length in pixel
 */
static uint32_t get_str_length(eviewitf_plot_text_attributes_t *text) {
    uint32_t ret = 0;
    size_t len = strlen(text->text);

    for (size_t i = 0; i < len; i++) {
        ret = plot_char(NULL, ret, 0, text->text[i], text->size, text->color, 0) + text->size;
    }
    return ret;
}

/* clang-format off */
/**
 * @fn uint32_t plot_str(eviewitf_plot_frame_attributes_t *frame, eviewitf_plot_text_attributes_t *text, eviewitf_plot_rgb_color_attributes_t color)
 * @brief Plots a text in the desired RGB color into a frame
 *
 * @param frame: Frame attributes pointer
 * @param text: Text attributes pointer
 * @param color: RGB text pixel color attributes
 *
 * @return The row position in pixel after writting the string
 */
/* clang-format on */
static uint32_t plot_str(eviewitf_plot_frame_attributes_t *frame, eviewitf_plot_text_attributes_t *text,
                         eviewitf_plot_rgb_color_attributes_t color) {
    uint32_t ret = text->x;
    uint32_t txt_sz = get_str_length(text);
    uint32_t off = 0;
    size_t len = strlen(text->text);

    if (text->alignment == EVIEWITF_PLOT_TEXT_ALIGN_CENTER) {
        off = txt_sz / 2u;
    } else if (text->alignment == EVIEWITF_PLOT_TEXT_ALIGN_RIGHT) {
        off = txt_sz;
    } else {
        off = 0u;
    }
    ret -= off;

    for (size_t i = 0; i < len; i++) {
        ret = plot_char(frame, ret, text->y, text->text[i], text->size, color, 1) + text->size;
    }
    return ret;
}

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/* clang-format off */
/**
 * @fn eviewitf_plot_rectangle(eviewitf_plot_frame_attributes_t *frame, eviewitf_plot_rectangle_attributes_t *rect)
 * @brief Plots a rectangle into a frame
 *
 * @param frame: Frame attributes pointer where to plot the rectangle
 * @param rect: Rectangle attributes pointer to plot
 *
 * @return Return code as specified by the eviewitf_ret_t enumeration.
 */
/* clang-format on */
eviewitf_ret_t eviewitf_plot_rectangle(eviewitf_plot_frame_attributes_t *frame,
                                       eviewitf_plot_rectangle_attributes_t *rect) {
    uint8_t l_width = rect->line_width;
    uint32_t offset = 0;
    eviewitf_ret_t ret = EVIEWITF_OK;

    if ((l_width % 2u) != 0u) {
        l_width++;
    }

    /* Rectangle outline */
    if (l_width > 0u && rect->line_state == EVIEWITF_PLOT_DISPLAY_ENABLED) {
        offset = l_width;
        for (uint32_t y = rect->y; y < rect->y + l_width; y++) {
            /* Top */
            ret = plot_h_line(frame, rect->x, rect->width, y, rect->line_color);
            if (ret != EVIEWITF_OK) goto rect_out;

            /* Bottom */
            ret = plot_h_line(frame, rect->x, rect->width, y + rect->height - l_width, rect->line_color);
            if (ret != EVIEWITF_OK) goto rect_out;
        }

        for (uint32_t y = rect->y + l_width; y < rect->y + rect->height - l_width; y++) {
            /* Left */
            ret = plot_h_line(frame, rect->x, l_width, y, rect->line_color);
            if (ret != EVIEWITF_OK) goto rect_out;

            /* Right */
            ret = plot_h_line(frame, rect->x + rect->width - l_width, l_width, y, rect->line_color);
            if (ret != EVIEWITF_OK) goto rect_out;
        }
    }

    /* Rectangle fill */
    if (rect->fill_state == EVIEWITF_PLOT_DISPLAY_ENABLED) {
        for (uint32_t y = rect->y + offset; y < rect->y + rect->height - offset; y++) {
            ret = plot_h_line(frame, rect->x + offset, rect->width - 2u * offset, y, rect->fill_color);
            if (ret != EVIEWITF_OK) goto rect_out;
        }
    }

rect_out:
    return ret;
}

eviewitf_ret_t eviewitf_plot_text(eviewitf_plot_frame_attributes_t *frame, eviewitf_plot_text_attributes_t *text) {
    (void)plot_str(frame, text, text->color);
    return EVIEWITF_OK;
}
