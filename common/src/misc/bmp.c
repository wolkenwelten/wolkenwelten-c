/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "bmp.h"
#include "colors.h"
#include "misc.h"

#include <stdlib.h>
#include <string.h>

/* Writes val to b as an unsigned 16-bit little-endian integer */
static void *WriteLEu16(void *buf, u16 val){
        u8 *b = buf;
        *b++ = (val     ) & 0xFF;
        *b++ = (val >> 8) & 0xFF;
        return b;
}

/* Writes val to buf as an unsigned 24-bit little-endian integer */
static void *WriteLEu24(void *buf, u32 val){
        u8 *b = buf;
        *b++ = (val      ) & 0xFF;
        *b++ = (val >>  8) & 0xFF;
        *b++ = (val >> 16) & 0xFF;
        return b;
}

/* Writes val to buf as an unsigned 32-bit little-endian integer */
static void *WriteLEu32(void *buf,u32 val){
        u8 *b = buf;
        *b++ = (val      ) & 0xFF;
        *b++ = (val >>  8) & 0xFF;
        *b++ = (val >> 16) & 0xFF;
        *b++ = (val >> 24) & 0xFF; return b;
}

/* Calculates rowPadding since each row has to be DWORD aligned. */
static int BMPCalcRowPadding(int width,int pixelLength){
        const int ret = 4 - ((width * pixelLength) & 0x3);
        return ret == 4 ? 0 : ret;
}

/* Reads 24bpp BMP data and puts the pixels in *ret. */
static void BMPWritePixels24(u8 *out, int width, int height, const u32 *data){
        int cy,cx,rowPadding;
        const u32 *src;

        rowPadding = BMPCalcRowPadding(width,3);
        for(cy = 0; cy < height;cy++){
                src = data + (cy * width);
                for(cx = width;cx > 0;cx--){
                        out = WriteLEu24(out, RgbaToBgra(*src++));
                }
                out += rowPadding; /* Row has to be DWORD aligned. */
        }
}

/* Write a BMP Image File into filename, pixels has to be 32-bit RGBA
 * Data.
 */
void saveBMP(const char *filename, int width, int height, const u32 *data){
        int imagesize,filesize;
        u8 *buffer,*buf,*pixels;
        if(data == NULL){return;}

	imagesize = (width+BMPCalcRowPadding(width,3))*height*3;
        filesize  = imagesize + 14 + 40;
        buffer    = calloc(1,filesize);
        buf       = buffer;
        pixels    = buffer + 14 + 40;

        *buf++ = 'B';
        *buf++ = 'M';
        buf = WriteLEu32(buf, filesize);
        buf = WriteLEu16(buf, 0);           /* Reserved 1                   */
        buf = WriteLEu16(buf, 0);           /* Reserved 2                   */
        buf = WriteLEu32(buf, 14 + 40);     /* Pixel Offset                 */
        buf = WriteLEu32(buf, 40);          /* BITMAPINFOHEADER             */
        buf = WriteLEu32(buf, width);
        buf = WriteLEu32(buf, height);
        buf = WriteLEu16(buf, 1);           /* Planes                       */
        buf = WriteLEu16(buf, 24);          /* BPP                          */
        buf = WriteLEu32(buf, 0);           /* Compression method           */
        buf = WriteLEu32(buf, imagesize);
        buf = WriteLEu32(buf, 8192);        /* h. resolution, pixels/meter. */
        buf = WriteLEu32(buf, 8192);        /* v. resolution, pixels/meter. */
        buf = WriteLEu32(buf, 0);           /* Palette size, 0 for 2^n.     */
        buf = WriteLEu32(buf, 0);           /* Important colors, 0 for all. */

	BMPWritePixels24(pixels, width, height, data);

	saveFile(filename, buffer, filesize);
	free(buffer);
}
