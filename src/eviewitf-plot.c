/**
 * \file
 * \brief Communication API between A53 and R7 CPUs for plot functions
 * \author LACROIX Impulse
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

/* Font to pixel */
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
 * \enum eviewitf_text_align
 * \brief eViewItf bounding boxes text alignment supported.
 */
typedef enum {
    EVIEWITF_TEXT_ALIGN_LEFT,
    EVIEWITF_TEXT_ALIGN_CENTER,
    EVIEWITF_TEXT_ALIGN_RIGHT,
} eviewitf_text_align;

/**
 * \struct eviewitf_yuv_color_attributes_t
 * \brief Structure to set an YUV color
 *
 */
typedef struct {
    uint8_t y; /*!< Y channel value */
    uint8_t u; /*!< U channel value */
    uint8_t v; /*!< V channel value */
} eviewitf_yuv_color_attributes_t;

/**
 * \fn uint8_t get_font_value(int i, int j)
 * \brief Gets a (i,j) font value
 *
 * \param i: Font value row position
 * \param j: Font value column position
 *
 * \return font value
 */
static uint8_t get_font_value(int i, int j) { return font_basic[i][j]; }

/**
 * \fn void rgb_color_to_yuv_color(eviewitf_rgb_color_attributes_t *rgb, eviewitf_yuv_color_attributes_t *yuv)
 * \brief Converts an RGB color into an YUV one
 *
 * \param rgb: RGB color to convert
 * \param yuv: YUV converted color
 *
 */
static void rgb_color_to_yuv_color(eviewitf_rgb_color_attributes_t *rgb, eviewitf_yuv_color_attributes_t *yuv) {
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

/**
 * \fn void set_yuv422sp_pixel(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t y,
 * eviewitf_yuv_color_attributes_t color) \brief Sets a pixel to the desired YUV color into an YUV422sp frame
 *
 * \param frame: YUV422sp frame
 * \param x: Row pixel position
 * \param y: Column pixel position
 * \param color: YUV pixel color
 *
 */
static void set_yuv422sp_pixel(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t y,
                               eviewitf_yuv_color_attributes_t color) {
    if ((x % 2u) != 0u) {
        return;
    }

    /* Y value */
    frame->buffer[x + (y * frame->width) + 0u] = color.y;
    frame->buffer[x + (y * frame->width) + 1u] = color.y;

    /* U value */
    frame->buffer[x + ((y + frame->height) * frame->width) + 0u] = color.u;
    /* V value */
    frame->buffer[x + ((y + frame->height) * frame->width) + 1u] = color.v;
}

/**
 * \fn void draw_yuv422sp_h_line(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y,
 * eviewitf_yuv_color_attributes_t color) \brief Draws an horizontal line to the desired YUV color into an YUV422sp
 * frame
 *
 * \param frame: YUV422sp frame
 * \param x: Row pixel position
 * \param len: Line length
 * \param y: Column pixel position
 * \param color: YUV pixel color
 *
 */
static void draw_yuv422sp_h_line(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t len, uint32_t y,
                                 eviewitf_yuv_color_attributes_t color) {
    if ((x % 2u) != 0u) x++;

    /* Y value */
    memset(&frame->buffer[x + (y * frame->width)], color.y, len);

    uint32_t xval = x + ((y + frame->height) * frame->width);
    uint8_t uv_color[2] = {color.u, color.v};
    for (uint32_t xx = 0; xx < len; xx += 2u) {
        /* UV values */
        memcpy(&frame->buffer[xval], uv_color, sizeof(uint16_t));
        xval += 2u;
    }
}

/**
 * \fn uint32_t plot_yuv422sp_char(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t y, char c, uint32_t sz,
 * eviewitf_yuv_color_attributes_t color, uint8_t disp) \brief Draws a character in the desired YUV color into an
 * YUV422sp frame
 *
 * \param frame: YUV422sp frame
 * \param x: Row pixel position
 * \param y: Column pixel position
 * \param c: Character to draw
 * \param sz: Character size
 * \param color: YUV pixel color
 * \param disp: Will be plotted if disp is not equal to 0
 *
 * \return The row position after writting the character
 */
static uint32_t plot_yuv422sp_char(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t y, char c, uint32_t sz,
                                   eviewitf_yuv_color_attributes_t color, uint8_t disp) {
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
                        set_yuv422sp_pixel(frame, x + cnt_col + incx, y + cnt_row + incy, color);
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

/**
 * \fn get_yuv422sp_str_length(char *str, uint32_t sz)
 * \brief Gets the string length in pixel
 *
 * \param str: string
 * \param sz: Text size
 *
 * \return The string length in pixel
 */
static uint32_t get_yuv422sp_str_length(char *str, uint32_t sz) {
    eviewitf_yuv_color_attributes_t color;
    uint32_t ret = 0;
    for (size_t i = 0; i < strlen(str); i++) {
        ret = plot_yuv422sp_char(NULL, ret, 0, str[i], sz, color, 0) + sz;
    }
    return ret;
}

/**
 * \fn uint32_t plot_yuv422sp_str(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t y, char *str, uint32_t sz,
 * eviewitf_text_align align, eviewitf_yuv_color_attributes_t color) \brief Plots a string in the desired YUV color into
 * an YUV422sp frame
 *
 * \param frame: YUV422sp frame
 * \param x: Row pixel position
 * \param y: Column pixel position
 * \param str: String to draw
 * \param sz: Character size
 * \param color: YUV pixel color
 *
 * \return The row position after writting the string
 */
static uint32_t plot_yuv422sp_str(eviewitf_frame_attributes_t *frame, uint32_t x, uint32_t y, char *str, uint32_t sz,
                                  eviewitf_text_align align, eviewitf_yuv_color_attributes_t color) {
    uint32_t txt_sz = get_yuv422sp_str_length(str, sz);
    uint32_t off = 0;
    if (align == EVIEWITF_TEXT_ALIGN_CENTER) {
        off = txt_sz / 2u;
    } else if (align == EVIEWITF_TEXT_ALIGN_RIGHT) {
        off = txt_sz;
    } else {
        off = 0u;
    }
    uint32_t ret = x - off;
    for (size_t i = 0; i < strlen(str); i++) {
        ret = plot_yuv422sp_char(frame, ret, y, str[i], sz, color, 1) + sz;
    }
    return ret;
}

/**
 * \fn uint32_t char *label_to_str(eviewitf_bounding_box_label label)
 * \brief Converts a label into the string associated
 *
 * \param label: label definition value
 *
 * \return The string associated to the label definition value
 */
static char *label_to_str(eviewitf_bounding_box_label label) {
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_PERSON) return "Person";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_BICYCLE) return "Bicycle";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_CAR) return "Car";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_MOTORCYCLE) return "Motorcycle";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_AIRPLANE) return "Airplane";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_BUS) return "Bus";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_TRAIN) return "Train";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_TRUCK) return "Truck";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_BOAT) return "Boat";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_TRAFFIC_LIGHT) return "Traffic Light";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_FIRE_HYDRANT) return "Fire Hydrant";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_STOP_SIGN) return "Stop Sign";
    if (label == EVIEWITF_BOUNDING_BOX_LABEL_PARKING_METER) return "Parking Meter";
    return "Unlabeled";
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

/**
 * \fn eviewitf_plot_bounding_box(eviewitf_frame_attributes_t *frame, eviewitf_bounding_box_attributes_t *bounding_box)
 * \brief Plots a bouding box into a frame
 *
 * \param frame: frame pointer where to draw the bounding box
 * \param bounding_box: bounding box pointer to draw
 *
 * \return Return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_plot_bounding_box(eviewitf_frame_attributes_t *frame, eviewitf_bounding_box_attributes_t *bounding_box) {
    if (frame->format == EVIEWITF_FRAME_FORMAT_YUV422SP) {
        eviewitf_yuv_color_attributes_t yuv_line_color;
        eviewitf_yuv_color_attributes_t yuv_text_color;
        uint32_t x_off = bounding_box->x_offset;
        uint32_t y_off = bounding_box->y_offset;
        uint32_t width = bounding_box->width;
        uint32_t height = bounding_box->height;
        uint8_t l_width = bounding_box->line_width;
        uint8_t t_size = bounding_box->text_size;
        uint8_t score = bounding_box->score;
        uint32_t x_txt = 0;
        uint32_t y_txt = 0;
        uint32_t l_txt = 0;
        eviewitf_bounding_box_label label = bounding_box->label;

        if ((l_width % 2u) != 0u) {
            l_width++;
        }

        rgb_color_to_yuv_color(&bounding_box->line_color, &yuv_line_color);
        rgb_color_to_yuv_color(&bounding_box->text_color, &yuv_text_color);

        for (uint32_t y = y_off; y < y_off + l_width; y++) {
            /* Top */
            draw_yuv422sp_h_line(frame, x_off, width, y, yuv_line_color);
            /* Bottom */
            draw_yuv422sp_h_line(frame, x_off, width, y + height - l_width, yuv_line_color);
        }

        for (uint32_t y = y_off + l_width; y < y_off + height - l_width; y++) {
            /* Left */
            draw_yuv422sp_h_line(frame, x_off, l_width, y, yuv_line_color);
            /* Right */
            draw_yuv422sp_h_line(frame, x_off + width - l_width, l_width, y, yuv_line_color);
        }

        if (bounding_box->label_state == EVIEWITF_BOUNDING_BOX_DISPLAY_ENABLED) {
            /* Label */
            x_txt = x_off + l_width;
            y_txt = y_off + l_width;
            l_txt = get_yuv422sp_str_length(label_to_str(label), t_size);

            for (uint32_t y = y_txt; y < y_txt + t_size * 8u * 2u; y++) {
                draw_yuv422sp_h_line(frame, x_txt, l_txt + t_size * 4u, y, yuv_line_color);
            }

            x_txt += t_size * 2u;
            y_txt += t_size * 2u;

            plot_yuv422sp_str(frame, x_txt, y_txt, label_to_str(label), t_size, EVIEWITF_TEXT_ALIGN_LEFT,
                              yuv_text_color);
        }

        if (bounding_box->score_state == EVIEWITF_BOUNDING_BOX_DISPLAY_ENABLED) {
            /* Score */
            char buf[10];
            (void)sprintf(buf, "%d%%", score);

            l_txt = get_yuv422sp_str_length(buf, t_size);
            x_txt = x_off + width - l_width - l_txt - t_size * 2u;
            y_txt = y_off + l_width;

            for (uint32_t y = y_txt; y < y_txt + t_size * 8u * 2u; y++) {
                draw_yuv422sp_h_line(frame, x_txt, x_off + width - l_width - x_txt, y, yuv_line_color);
            }

            x_txt += l_txt + t_size * 2u;
            y_txt += t_size * 2u;

            plot_yuv422sp_str(frame, x_txt, y_txt, buf, t_size, EVIEWITF_TEXT_ALIGN_RIGHT, yuv_text_color);
        }
        return EVIEWITF_OK;
    }

    return EVIEWITF_FAIL;
}
