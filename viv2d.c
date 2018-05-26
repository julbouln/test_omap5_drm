#include "viv2d.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "xf86drm.h"
#include "etnaviv_drmif.h"
#include "etnaviv_drm.h"

#include "state.xml.h"
#include "state_2d.xml.h"
#include "cmdstream.xml.h"

#ifdef VIV2D_BMP_DUMP
#include "write_bmp.h"
#endif

/*
Notes:
- input color are ABGR
- x,y=0,0 is left,bottom
*/

#define ALIGN(val, align)	(((val) + (align) - 1) & ~((align) - 1))

/*
Raster operation foreground and background codes. Even though ROP is not used in `CLEAR`,
`HOR_FILTER_BLT`, `VER_FILTER_BLT` and alpha-enabled `BIT_BLTs`, ROP code still has to be
programmed, because the engine makes the decision whether source, destination and pattern are
involved in the current operation and the correct decision is essential for the engine to complete
the operation as expected.

ROP builds a lookup table for a logical operation with 2, 3 or 4 inputs (depending on ROP type). So
for a ROP3, for example, the ROP pattern will be 2^3=8 bits.

These are the input bit for the ROPs, per ROP type:

`ROP2_PATTERN` [untested]
    bit 0 destination
    bit 1 pattern

`ROP2_SOURCE` [untested]
    bit 0 destination
    bit 1 source

`ROP3` (uses `ROP_FG` only)
    bit 0 destination
    bit 1 source
    bit 2 pattern

`ROP4` (uses `ROP_FG` and `ROP_BG`)
    bit 0 destination
    bit 1 source
    bit 2 pattern
    bit "3" foreground/background (`ROP_FG` / `ROP_BG`)


ROP3/4 examples:
    10101010  0xaa   destination
    01010101  0x55   !destination
    11001100  0xcc   source
    00110011  0x33   !source
    11110000  0xf0   pattern
    00001111  0x0f   !pattern

*/
#if 0
/* GXclear        */  0x00,		// ROP_BLACK,
/* GXand          */  0xa0,		// ROP_BRUSH_AND_DST,
/* GXandReverse   */  0x50,		// ROP_BRUSH_AND_NOT_DST,
/* GXcopy         */  0xf0,		// ROP_BRUSH,
/* GXandInverted  */  0x0a,		// ROP_NOT_BRUSH_AND_DST,
/* GXnoop         */  0xaa,		// ROP_DST,
/* GXxor          */  0x5a,		// ROP_BRUSH_XOR_DST,
/* GXor           */  0xfa,		// ROP_BRUSH_OR_DST,
/* GXnor          */  0x05,		// ROP_NOT_BRUSH_AND_NOT_DST,
/* GXequiv        */  0xa5,		// ROP_NOT_BRUSH_XOR_DST,
/* GXinvert       */  0x55,		// ROP_NOT_DST,
/* GXorReverse    */  0xf5,		// ROP_BRUSH_OR_NOT_DST,
/* GXcopyInverted */  0x0f,		// ROP_NOT_BRUSH,
/* GXorInverted   */  0xaf,		// ROP_NOT_BRUSH_OR_DST,
/* GXnand         */  0x5f,		// ROP_NOT_BRUSH_OR_NOT_DST,
/* GXset          */  0xff		// ROP_WHITE
#endif

#define ROP_BLACK 					0x00
#define	ROP_NOT_BRUSH_AND_NOT_DST 	0x05
#define ROP_NOT_BRUSH_AND_DST 		0x0a
#define ROP_NOT_BRUSH 				0x0f
#define ROP_NOT_SRC_AND_NOT_DST 	0x11
#define ROP_NOT_SRC_AND_DST 		0x22
#define ROP_NOT_SRC					0x33
#define ROP_SRC_AND_NOT_DST 		0x44
#define ROP_BRUSH_XOR_DST			0x5a
#define ROP_NOT_DST					0x55
#define ROP_DST_XOR_SRC 			0x66
#define ROP_NOT_SRC_OR_NOT_DST		0x77
#define ROP_DST_AND_SRC 			0x88
#define ROP_NOT_SRC_XOR_DST			0x99
#define	ROP_BRUSH_AND_DST			0xa0
#define	ROP_NOT_BRUSH_XOR_DST 		0xa5
#define ROP_DST 					0xaa
#define	ROP_NOT_BRUSH_OR_DST 		0xaf
#define ROP_NOT_SRC_OR_DST			0xbb
#define ROP_SRC 					0xcc
#define ROP_SRC_OR_NOT_DST			0xdd
#define ROP_DST_OR_SRC 				0xee
#define ROP_BRUSH    				0xf0
#define ROP_BRUSH_OR_NOT_DST 		0xf5
#define ROP_WHITE 					0xff

// etna utils

static inline void etna_emit_load_state(struct etna_cmd_stream *stream,
                                        const uint16_t offset, const uint16_t count)
{
	uint32_t v;

	v = 	(VIV_FE_LOAD_STATE_HEADER_OP_LOAD_STATE | VIV_FE_LOAD_STATE_HEADER_OFFSET(offset) |
	         (VIV_FE_LOAD_STATE_HEADER_COUNT(count) & VIV_FE_LOAD_STATE_HEADER_COUNT__MASK));

//	printf("etna_emit_load_state: offset:0x%x count:%d\n",offset,count);
	etna_cmd_stream_emit(stream, v);
}

static inline void etna_set_state(struct etna_cmd_stream *stream, uint32_t address, uint32_t value)
{
	etna_cmd_stream_reserve(stream, 2);
	etna_emit_load_state(stream, address >> 2, 1);
	etna_cmd_stream_emit(stream, value);
}


static inline void etna_set_state_from_bo(struct etna_cmd_stream *stream,
        uint32_t address, struct etna_bo *bo)
{
	etna_cmd_stream_reserve(stream, 2);
	etna_emit_load_state(stream, address >> 2, 1);

	etna_cmd_stream_reloc(stream, &(struct etna_reloc) {
		.bo = bo,
		 .flags = ETNA_RELOC_READ,
		  .offset = 0,
	});
}


static const viv2d_format
viv2d_color_formats[] = {
	{viv2d_a8r8g8b8, 32, 32, DE_FORMAT_A8R8G8B8, DE_SWIZZLE_ARGB},
	{viv2d_x8r8g8b8, 32, 24, DE_FORMAT_X8R8G8B8, DE_SWIZZLE_ARGB},
	{viv2d_a8b8g8r8, 32, 32, DE_FORMAT_A8R8G8B8, DE_SWIZZLE_ABGR},
	{viv2d_x8b8g8r8, 32, 24, DE_FORMAT_X8R8G8B8,	DE_SWIZZLE_ABGR},
	{viv2d_b8g8r8a8, 32, 32, DE_FORMAT_A8R8G8B8,	DE_SWIZZLE_BGRA},
	{viv2d_b8g8r8x8, 32, 24, DE_FORMAT_X8R8G8B8,	DE_SWIZZLE_BGRA},
	{viv2d_r5g6b5, 16, 15, DE_FORMAT_R5G6B5, DE_SWIZZLE_ARGB},
	{viv2d_b5g6r5, 16, 15, DE_FORMAT_R5G6B5,	DE_SWIZZLE_ABGR},
	{viv2d_a1r5g5b5, 16, 16, DE_FORMAT_A1R5G5B5, DE_SWIZZLE_ARGB},
	{viv2d_x1r5g5b5, 16, 15, DE_FORMAT_X1R5G5B5, DE_SWIZZLE_ARGB},
	{viv2d_a1b5g5r5, 16, 16, DE_FORMAT_A1R5G5B5,	DE_SWIZZLE_ABGR},
	{viv2d_x1b5g5r5, 16, 15, DE_FORMAT_X1R5G5B5, DE_SWIZZLE_ABGR},
	{viv2d_a4r4g4b4, 16, 16, DE_FORMAT_A4R4G4B4, DE_SWIZZLE_ARGB},
	{viv2d_x4r4g4b4, 16, 12, DE_FORMAT_X4R4G4B4, DE_SWIZZLE_ARGB},
	{viv2d_a4b4g4r4, 16, 16, DE_FORMAT_A4R4G4B4, DE_SWIZZLE_ABGR},
	{viv2d_x4b4g4r4, 16, 12, DE_FORMAT_X4R4G4B4, DE_SWIZZLE_ABGR},
	{viv2d_a8, 8, 8, DE_FORMAT_A8, DE_SWIZZLE_ARGB},
	/*END*/
};

static const viv2d_blend viv2d_blend_ops[] = {
	{viv2d_blend_clear,			DE_BLENDMODE_ZERO, 				DE_BLENDMODE_ZERO},
	{viv2d_blend_src,				DE_BLENDMODE_ONE, 				DE_BLENDMODE_ZERO},
	{viv2d_blend_dst,				DE_BLENDMODE_ZERO, 				DE_BLENDMODE_ONE},
	{viv2d_blend_over,			DE_BLENDMODE_ONE,				DE_BLENDMODE_INVERSED},
	{viv2d_blend_over_rev,		DE_BLENDMODE_INVERSED, 			DE_BLENDMODE_ONE},
	{viv2d_blend_in,				DE_BLENDMODE_NORMAL,			DE_BLENDMODE_ZERO},
	{viv2d_blend_in_rev,		DE_BLENDMODE_ZERO,				DE_BLENDMODE_NORMAL},
	{viv2d_blend_out,				DE_BLENDMODE_INVERSED,			DE_BLENDMODE_ZERO},
	{viv2d_blend_out_rev,		DE_BLENDMODE_ZERO,				DE_BLENDMODE_INVERSED},
	{viv2d_blend_atop,			DE_BLENDMODE_NORMAL,			DE_BLENDMODE_INVERSED},
	{viv2d_blend_atop_rev,		DE_BLENDMODE_INVERSED,			DE_BLENDMODE_NORMAL},
	{viv2d_blend_xor,				DE_BLENDMODE_INVERSED,			DE_BLENDMODE_INVERSED},
	{viv2d_blend_add,				DE_BLENDMODE_ONE,				DE_BLENDMODE_ONE},
	{viv2d_blend_saturate,		DE_BLENDMODE_SATURATED_ALPHA,	DE_BLENDMODE_ONE} // ?
};

static inline unsigned int viv2d_pitch(unsigned width, unsigned bpp)
{
//	unsigned pitch = bpp != 4 ? width * ((bpp + 7) / 8) : width / 2;

	/* GC320 and GC600 needs pitch aligned to 16 */
//	return ALIGN(pitch, 16);
//	printf("BPP %d\n",bpp);
	return ALIGN(width * ((bpp + 7) / 8), 32);
//	return width * 4;
}

void viv2d_device_close(viv2d_device *dev) {

	if (dev->stream)
		etna_cmd_stream_del(dev->stream);

	if (dev->pipe)
		etna_pipe_del(dev->pipe);

	if (dev->gpu)
		etna_gpu_del(dev->gpu);

	if (dev->dev)
		etna_device_del(dev->dev);

	if (dev->fd > -1)
		close(dev->fd);

	free(dev);
}

viv2d_device *viv2d_device_open(char *devname) {
	viv2d_device *dev = (viv2d_device *)malloc(sizeof(viv2d_device));

	dev->dev = NULL;
	dev->gpu = NULL;
	dev->pipe = NULL;
	dev->stream = NULL;

	dev->fd = open(devname, O_RDWR);
	if (dev->fd < 0) {
		viv2d_device_close(dev);
		return NULL;
	}

	dev->dev = etna_device_new(dev->fd);
	if (!dev->dev) {
		viv2d_device_close(dev);
		return NULL;
	}

	dev->gpu = etna_gpu_new(dev->dev, 0);
	if (!dev->gpu) {
		viv2d_device_close(dev);
		return NULL;
	}

	dev->pipe = etna_pipe_new(dev->gpu, ETNA_PIPE_2D);
	if (!dev->pipe) {
		viv2d_device_close(dev);
		return NULL;
	}

	dev->stream = etna_cmd_stream_new(dev->pipe, 1024, NULL, NULL);
	if (!dev->stream) {
		viv2d_device_close(dev);
		return NULL;
	}

	return dev;
}

void viv2d_commit(viv2d_device *dev) {
	if (dev->stream) {
		printf("viv2d_commit: %d/%d\n", dev->stream->offset, dev->stream->size);
		etna_cmd_stream_finish(dev->stream);
	}
}

void viv2d_flush(viv2d_device *dev) {
	if (dev->stream) {
		printf("viv2d_flush: %d/%d\n", dev->stream->offset, dev->stream->size);
		etna_cmd_stream_flush(dev->stream);
	}
}

viv2d_surface *viv2d_surface_alloc(unsigned int width, unsigned int height, viv2d_color_format format) {
	viv2d_surface *surf = (viv2d_surface *)malloc(sizeof(viv2d_surface));

	surf->width = width;
	surf->height = height;

	surf->format = viv2d_color_formats[format];

	surf->pitch = viv2d_pitch(width, surf->format.bpp);
	return surf;
}

viv2d_surface *viv2d_surface_new(viv2d_device *dev, unsigned int width, unsigned int height, viv2d_color_format format) {
	viv2d_surface *surf = viv2d_surface_alloc(width, height, format);
	printf("new surface %dx%d %d %d %d\n", width, height, surf->format.bpp, surf->pitch, height * surf->pitch);
	surf->bo = etna_bo_new(dev->dev, surf->pitch * surf->height, ETNA_BO_UNCACHED);
	return surf;
}

#ifdef VIV2D_BMP_DUMP

void viv2d_surface_to_bmp(viv2d_surface *surf, const char *filename) {
	etna_bo_cpu_prep(surf->bo, DRM_ETNA_PREP_READ);
	bmp_dump32(etna_bo_map(surf->bo), surf->width, surf->height, false, filename);
}
#endif

void viv2d_surface_del(viv2d_surface *surf) {
	if (surf->bo)
		etna_bo_del(surf->bo);

	free(surf);
}

viv2d_op *viv2d_op_new(viv2d_op_cmd cmd, viv2d_surface *src, viv2d_surface *dst) {
	if (dst) {
		viv2d_op *op = (viv2d_op *)malloc(sizeof(viv2d_op));
		op->cmd = cmd;
		op->src = src;
		op->dst = dst;
		op->pat = NULL;
		op->num_rects = 0;

		op->clip_rect.x = 0;
		op->clip_rect.y = 0;
		op->clip_rect.width = dst->width;
		op->clip_rect.height = dst->height;
		op->is_blend = 0;
		op->src_alpha = 0xff;
		op->dst_alpha = 0xff;
		op->color = 0xff000000;
		op->rop = 0xcc;
		return op;
	} else
		return NULL;
}

void viv2d_op_add_rect(viv2d_op *op, unsigned int x, unsigned int y, unsigned int width, unsigned int height ) {
	viv2d_rect r;
	r.x = x;
	r.y = y;
	r.width = width;
	r.height = height;
	op->rects[op->num_rects] = r;
	op->num_rects++;
}

void viv2d_op_set_clip_rect(viv2d_op *op, int x, int y, int width, int height ) {
	op->clip_rect.x = x;
	op->clip_rect.y = y;
	op->clip_rect.width = width;
	op->clip_rect.height = height;
}

void viv2d_op_set_color(viv2d_op *op, int col) {
	op->color = col;
}

void viv2d_op_set_blend(viv2d_op *op, viv2d_blend_cmd cmd, bool src_global, uint8_t src_alpha, bool dst_global, uint8_t dst_alpha) {
	op->blend = viv2d_blend_ops[cmd];
	op->src_global = src_global;
	op->src_alpha = src_alpha;
	op->dst_global = dst_global;
	op->dst_alpha = dst_alpha;
	op->is_blend = 1;
}

void viv2d_op_exec(viv2d_device *dev, viv2d_op *op) {
	int i;
	int cmd;

	if (op->num_rects == 0) {
		viv2d_op_add_rect(op, 0, 0, op->dst->width, op->dst->height);
	}

	switch (op->cmd) {
	case viv2d_cmd_clear:
		cmd = VIVS_DE_DEST_CONFIG_COMMAND_CLEAR;
		break;
	case viv2d_cmd_line:
		cmd = VIVS_DE_DEST_CONFIG_COMMAND_LINE;
		break;
	case viv2d_cmd_bitblt:
		cmd = VIVS_DE_DEST_CONFIG_COMMAND_BIT_BLT;
		break;
	case viv2d_cmd_stretchblt:
		cmd = VIVS_DE_DEST_CONFIG_COMMAND_STRETCH_BLT;
		break;
	case viv2d_cmd_multsrcblt:
		cmd = VIVS_DE_DEST_CONFIG_COMMAND_MULTI_SOURCE_BLT;
		break;
	default:
		cmd = VIVS_DE_DEST_CONFIG_COMMAND_CLEAR;
		break;
	}

	etna_set_state_from_bo(dev->stream, VIVS_DE_DEST_ADDRESS, op->dst->bo);
	etna_set_state(dev->stream, VIVS_DE_DEST_STRIDE, op->dst->pitch);
	etna_set_state(dev->stream, VIVS_DE_DEST_ROTATION_CONFIG, 0);
	etna_set_state(dev->stream, VIVS_DE_DEST_CONFIG,
	               VIVS_DE_DEST_CONFIG_FORMAT(op->dst->format.fmt) |
	               VIVS_DE_DEST_CONFIG_SWIZZLE(op->dst->format.swizzle) |
	               cmd |
	               VIVS_DE_DEST_CONFIG_TILED_DISABLE |
	               VIVS_DE_DEST_CONFIG_MINOR_TILED_DISABLE
	              );

	etna_set_state(dev->stream, VIVS_DE_CLIP_TOP_LEFT,
	               VIVS_DE_CLIP_TOP_LEFT_X(op->clip_rect.x) |
	               VIVS_DE_CLIP_TOP_LEFT_Y(op->clip_rect.y)
	              );
	etna_set_state(dev->stream, VIVS_DE_CLIP_BOTTOM_RIGHT,
	               VIVS_DE_CLIP_BOTTOM_RIGHT_X(op->clip_rect.x + op->clip_rect.width) |
	               VIVS_DE_CLIP_BOTTOM_RIGHT_Y(op->clip_rect.y + op->clip_rect.height)
	              );

	if (op->src) {
		if (op->cmd == viv2d_cmd_multsrcblt) {
			// NOTE does not work
#if 1
			/*			etna_set_state(dev->stream, VIVS_DE_SRC_STRIDE, 0);
						etna_set_state(dev->stream, VIVS_DE_SRC_ROTATION_CONFIG, 0);
						etna_set_state(dev->stream, VIVS_DE_SRC_CONFIG, 0);
						etna_set_state(dev->stream, VIVS_DE_SRC_ORIGIN,
						               VIVS_DE_SRC_ORIGIN_X(0) |
						               VIVS_DE_SRC_ORIGIN_Y(0));
						etna_set_state(dev->stream, VIVS_DE_SRC_SIZE,
						               VIVS_DE_SRC_SIZE_X(0) |
						               VIVS_DE_SRC_SIZE_Y(0)
						              );
			*/
			printf("multisrcblit\n");
			// VIVS_DE_BLOCK8_* + (srcIdx << 2)
			for (int src_idx = 0; src_idx < op->src_count; src_idx++) {
				etna_set_state_from_bo(dev->stream, VIVS_DE_BLOCK8_SRC_ADDRESS(src_idx), op->srcs[src_idx]->bo);
				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_STRIDE(src_idx), op->srcs[src_idx]->pitch);
				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_ROTATION_CONFIG(src_idx), op->srcs[src_idx]->width);
				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_CONFIG(src_idx),
				               VIVS_DE_BLOCK8_SRC_CONFIG_SOURCE_FORMAT(op->srcs[src_idx]->format.fmt) |
				               VIVS_DE_BLOCK8_SRC_CONFIG_SWIZZLE(op->srcs[src_idx]->format.swizzle) |
				               VIVS_DE_BLOCK8_SRC_CONFIG_LOCATION_MEMORY |
				               VIVS_DE_BLOCK8_SRC_CONFIG_PE10_SOURCE_FORMAT(op->srcs[src_idx]->format.fmt));

//				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_COLOR_BG(src_idx), 0);
//				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_COLOR_KEY_HIGH(src_idx), 0);

				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_ORIGIN(src_idx),
				               VIVS_DE_BLOCK8_SRC_ORIGIN_X(op->src_x) |
				               VIVS_DE_BLOCK8_SRC_ORIGIN_Y(op->src_y));
				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_SIZE(src_idx),
				               VIVS_DE_BLOCK8_SRC_SIZE_X(op->srcs[src_idx]->width) |
				               VIVS_DE_BLOCK8_SRC_SIZE_Y(op->srcs[src_idx]->height)
				              ); // source size is ignored

				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_ROTATION_HEIGHT(src_idx),
				               op->srcs[src_idx]->height);

				etna_set_state(dev->stream, VIVS_DE_BLOCK8_ROT_ANGLE(src_idx), 0);

				etna_set_state(dev->stream, VIVS_DE_BLOCK8_ROP(src_idx),
				               VIVS_DE_BLOCK8_ROP_ROP_FG(0xcc) | VIVS_DE_BLOCK8_ROP_ROP_BG(0xcc) | VIVS_DE_BLOCK8_ROP_TYPE_ROP4);


				etna_set_state(dev->stream, VIVS_DE_BLOCK8_SRC_EX_CONFIG(src_idx), 0);

				if (op->is_blend) {
					uint32_t alpha_mode = VIVS_DE_BLOCK8_ALPHA_MODES_GLOBAL_SRC_ALPHA_MODE_NORMAL |
					                      VIVS_DE_BLOCK8_ALPHA_MODES_GLOBAL_DST_ALPHA_MODE_NORMAL;

					etna_set_state(dev->stream, VIVS_DE_BLOCK8_ALPHA_CONTROL(src_idx),
					               VIVS_DE_BLOCK8_ALPHA_CONTROL_ENABLE_ON |
					               VIVS_DE_BLOCK8_ALPHA_CONTROL_PE10_GLOBAL_SRC_ALPHA(op->src_alpha) |
					               VIVS_DE_BLOCK8_ALPHA_CONTROL_PE10_GLOBAL_DST_ALPHA(op->dst_alpha));

					etna_set_state(dev->stream, VIVS_DE_BLOCK8_ALPHA_MODES(src_idx),
					               alpha_mode |
					               VIVS_DE_BLOCK8_ALPHA_MODES_SRC_BLENDING_MODE(DE_BLENDMODE_ONE) |
					               VIVS_DE_BLOCK8_ALPHA_MODES_DST_BLENDING_MODE(DE_BLENDMODE_ZERO));

					etna_set_state(dev->stream, VIVS_DE_BLOCK8_GLOBAL_SRC_COLOR(src_idx), 0);
					etna_set_state(dev->stream, VIVS_DE_BLOCK8_GLOBAL_DEST_COLOR(src_idx), 0);


				} else {
					etna_set_state(dev->stream, VIVS_DE_BLOCK8_ALPHA_CONTROL(src_idx), VIVS_DE_BLOCK8_ALPHA_CONTROL_ENABLE_OFF);
					etna_set_state(dev->stream, VIVS_DE_BLOCK8_ALPHA_MODES(src_idx), 0);
					etna_set_state(dev->stream, VIVS_DE_BLOCK8_GLOBAL_SRC_COLOR(src_idx), 0);
					etna_set_state(dev->stream, VIVS_DE_BLOCK8_GLOBAL_DEST_COLOR(src_idx), 0);
					etna_set_state(dev->stream, VIVS_DE_BLOCK8_COLOR_MULTIPLY_MODES(src_idx), 0);

				}
			}

//			etna_set_state(dev->stream, VIVS_DE_DE_MULTI_SOURCE, VIVS_DE_DE_MULTI_SOURCE_MAX_SOURCE(op->src_count - 1) |
//			               VIVS_DE_DE_MULTI_SOURCE_VERTICAL_BLOCK_LINE64 );
			etna_set_state(dev->stream, VIVS_DE_DE_MULTI_SOURCE, VIVS_DE_DE_MULTI_SOURCE_MAX_SOURCE(op->src_count - 1));
#endif
		} else {
			etna_set_state_from_bo(dev->stream, VIVS_DE_SRC_ADDRESS, op->src->bo);
			etna_set_state(dev->stream, VIVS_DE_SRC_STRIDE, op->src->pitch);
			etna_set_state(dev->stream, VIVS_DE_SRC_ROTATION_CONFIG, 0);
			etna_set_state(dev->stream, VIVS_DE_SRC_CONFIG,
			               VIVS_DE_SRC_CONFIG_SOURCE_FORMAT(op->src->format.fmt) |
			               VIVS_DE_SRC_CONFIG_SWIZZLE(op->src->format.swizzle) |
			               VIVS_DE_SRC_CONFIG_LOCATION_MEMORY |
			               VIVS_DE_SRC_CONFIG_PE10_SOURCE_FORMAT(op->src->format.fmt));
			etna_set_state(dev->stream, VIVS_DE_SRC_ORIGIN,
			               VIVS_DE_SRC_ORIGIN_X(op->src_x) |
			               VIVS_DE_SRC_ORIGIN_Y(op->src_y));
			etna_set_state(dev->stream, VIVS_DE_SRC_SIZE,
			               VIVS_DE_SRC_SIZE_X(op->src->width) |
			               VIVS_DE_SRC_SIZE_Y(op->src->height)
			              ); // source size is ignored
		}
	} else {

		if (op->pat) {
//			etna_set_state(dev->stream, VIVS_DE_SRC_ADDRESS, 0);
			etna_set_state(dev->stream, VIVS_DE_SRC_STRIDE, op->pat->pitch);
		}
		else {
			etna_set_state(dev->stream, VIVS_DE_SRC_STRIDE, 0);
		}
		etna_set_state(dev->stream, VIVS_DE_SRC_ROTATION_CONFIG, 0);
		if (op->pat) {
			etna_set_state(dev->stream, VIVS_DE_SRC_CONFIG,
			               VIVS_DE_SRC_CONFIG_UNK16 |
			               VIVS_DE_SRC_CONFIG_SOURCE_FORMAT(DE_FORMAT_MONOCHROME) |
			               VIVS_DE_SRC_CONFIG_LOCATION_MEMORY |
			               VIVS_DE_SRC_CONFIG_PACK_PACKED8 |
			               VIVS_DE_SRC_CONFIG_PE10_SOURCE_FORMAT(DE_FORMAT_MONOCHROME));

		} else {
			etna_set_state(dev->stream, VIVS_DE_SRC_CONFIG, 0);
		}
		etna_set_state(dev->stream, VIVS_DE_SRC_ORIGIN,
		               VIVS_DE_SRC_ORIGIN_X(0) |
		               VIVS_DE_SRC_ORIGIN_Y(0));
		etna_set_state(dev->stream, VIVS_DE_SRC_SIZE,
		               VIVS_DE_SRC_SIZE_X(0) |
		               VIVS_DE_SRC_SIZE_Y(0)
		              );

		if (op->pat) {
			etna_set_state(dev->stream, VIVS_DE_SRC_COLOR_BG, 0xff44ff44);
			etna_set_state(dev->stream, VIVS_DE_SRC_COLOR_FG, 0xff44ff44);
		}
	}

	if (op->cmd == viv2d_cmd_stretchblt) {
		etna_set_state(dev->stream, VIVS_DE_STRETCH_FACTOR_LOW,
		               VIVS_DE_STRETCH_FACTOR_LOW_X(((op->src->width - 1) << 16) / (op->dst->width - 1)));
		etna_set_state(dev->stream, VIVS_DE_STRETCH_FACTOR_HIGH,
		               VIVS_DE_STRETCH_FACTOR_HIGH_Y(((op->src->height - 1) << 16) / (op->dst->height - 1)));
	}

	if (op->cmd != viv2d_cmd_multsrcblt) {
		etna_set_state(dev->stream, VIVS_DE_ROP,
		               VIVS_DE_ROP_ROP_FG(op->rop) | VIVS_DE_ROP_ROP_BG(op->rop) | VIVS_DE_ROP_TYPE_ROP4);
	}

	if (op->cmd == viv2d_cmd_clear ) {
		/* Clear color PE20 */
		etna_set_state(dev->stream, VIVS_DE_CLEAR_PIXEL_VALUE32, op->color);
		/* Clear color PE10 */
		etna_set_state(dev->stream, VIVS_DE_CLEAR_BYTE_MASK, 0xff);
		etna_set_state(dev->stream, VIVS_DE_CLEAR_PIXEL_VALUE_LOW, op->color);
		etna_set_state(dev->stream, VIVS_DE_CLEAR_PIXEL_VALUE_HIGH, op->color);
	}

//	etna_set_state(dev->stream, VIVS_DE_CONFIG, 0); /* TODO */
//	etna_set_state(dev->stream, VIVS_DE_SRC_ORIGIN_FRACTION, 0);
	if (op->cmd != viv2d_cmd_multsrcblt) {
		if (op->is_blend) {
			uint32_t alpha_mode = VIVS_DE_ALPHA_MODES_GLOBAL_SRC_ALPHA_MODE_NORMAL |
			                      VIVS_DE_ALPHA_MODES_GLOBAL_DST_ALPHA_MODE_NORMAL;

			if (op->src_global)
				alpha_mode |= VIVS_DE_ALPHA_MODES_GLOBAL_SRC_ALPHA_MODE_GLOBAL;

			if (op->dst_global)
				alpha_mode |= VIVS_DE_ALPHA_MODES_GLOBAL_DST_ALPHA_MODE_GLOBAL;

			etna_set_state(dev->stream, VIVS_DE_ALPHA_CONTROL,
			               VIVS_DE_ALPHA_CONTROL_ENABLE_ON |
			               VIVS_DE_ALPHA_CONTROL_PE10_GLOBAL_SRC_ALPHA(op->src_alpha) |
			               VIVS_DE_ALPHA_CONTROL_PE10_GLOBAL_DST_ALPHA(op->dst_alpha));

			etna_set_state(dev->stream, VIVS_DE_ALPHA_MODES,
			               alpha_mode |
			               VIVS_DE_ALPHA_MODES_SRC_BLENDING_MODE(op->blend.src_mode) |
			               VIVS_DE_ALPHA_MODES_DST_BLENDING_MODE(op->blend.dst_mode));

			etna_set_state(dev->stream, VIVS_DE_GLOBAL_SRC_COLOR, op->src_alpha << 24);
			etna_set_state(dev->stream, VIVS_DE_GLOBAL_DEST_COLOR, op->dst_alpha << 24);

			etna_set_state(dev->stream, VIVS_DE_COLOR_MULTIPLY_MODES, /* PE20 */
			               VIVS_DE_COLOR_MULTIPLY_MODES_SRC_PREMULTIPLY_DISABLE |
			               VIVS_DE_COLOR_MULTIPLY_MODES_DST_PREMULTIPLY_DISABLE |
			               VIVS_DE_COLOR_MULTIPLY_MODES_SRC_GLOBAL_PREMULTIPLY_DISABLE |
			               VIVS_DE_COLOR_MULTIPLY_MODES_DST_DEMULTIPLY_DISABLE);
		} else {
			etna_set_state(dev->stream, VIVS_DE_ALPHA_CONTROL,
			               VIVS_DE_ALPHA_CONTROL_ENABLE_OFF);
			etna_set_state(dev->stream, VIVS_DE_ALPHA_MODES, 0);
			etna_set_state(dev->stream, VIVS_DE_GLOBAL_SRC_COLOR, 0);
			etna_set_state(dev->stream, VIVS_DE_GLOBAL_DEST_COLOR, 0);
			etna_set_state(dev->stream, VIVS_DE_COLOR_MULTIPLY_MODES, 0);
		}
	}

	if (op->pat) {
		etna_set_state_from_bo(dev->stream, VIVS_DE_PATTERN_ADDRESS, op->pat->bo);
		etna_set_state(dev->stream, VIVS_DE_PATTERN_CONFIG,
		               VIVS_DE_PATTERN_CONFIG_FORMAT(op->pat->format.fmt) |
		               VIVS_DE_PATTERN_CONFIG_TYPE_PATTERN);
		etna_set_state(dev->stream, VIVS_DE_PATTERN_MASK_LOW, 0xffffffff);
		etna_set_state(dev->stream, VIVS_DE_PATTERN_MASK_HIGH, 0xffffffff);


	}
	/* Queue DE command */
	etna_cmd_stream_reserve(dev->stream, op->num_rects * 2 + 2);
	etna_cmd_stream_emit(dev->stream,
	                     VIV_FE_DRAW_2D_HEADER_OP_DRAW_2D | VIV_FE_DRAW_2D_HEADER_COUNT(op->num_rects) /* render one rectangle */
	                    );
	etna_cmd_stream_emit(dev->stream, 0x0); /* rectangles start aligned */

	for (i = 0; i < op->num_rects; i++) {
		viv2d_rect rect = op->rects[i];
		etna_cmd_stream_emit(dev->stream, VIV_FE_DRAW_2D_TOP_LEFT_X(rect.x) | VIV_FE_DRAW_2D_TOP_LEFT_Y(rect.y));
		etna_cmd_stream_emit(dev->stream, VIV_FE_DRAW_2D_BOTTOM_RIGHT_X(rect.x + rect.width) | VIV_FE_DRAW_2D_BOTTOM_RIGHT_Y(rect.y + rect.height));

	}
	/*
		etna_set_state(dev->stream, 1, 0);
		etna_set_state(dev->stream, 1, 0);
		etna_set_state(dev->stream, 1, 0);
	*/
	etna_set_state(dev->stream, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_PE2D);

}

void viv2d_op_del(viv2d_op *op) {
	free(op);
}