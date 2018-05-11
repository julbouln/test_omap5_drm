#ifndef VIV2D_H
#define VIV2D_H

#include <stdint.h>
#include <stdbool.h>

#include "etnaviv_drmif.h"
#include "etnaviv_drm.h"

#define VIV2D_BMP_DUMP 1

#define VIV2D_MAX_RECTS 256

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#define PAGE_MASK       (~(PAGE_SIZE-1))
#define PAGE_ALIGN(addr)        (((addr)+PAGE_SIZE-1)&PAGE_MASK)

typedef struct _viv2d_device {
	int fd;
	struct etna_device *dev;
	struct etna_gpu *gpu;
	struct etna_pipe *pipe;
	struct etna_cmd_stream *stream;

} viv2d_device;

typedef struct _viv2d_rect {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
} viv2d_rect;

typedef enum {
	viv2d_a8r8g8b8 = 0,
	viv2d_x8r8g8b8,
	viv2d_a8b8g8r8,
	viv2d_x8b8g8r8,
	viv2d_b8g8r8a8,
	viv2d_b8g8r8x8,
	viv2d_r5g6b5,
	viv2d_b5g6r5,
	viv2d_a1r5g5b5,
	viv2d_x1r5g5b5,
	viv2d_a1b5g5r5,
	viv2d_x1b5g5r5,
	viv2d_a4r4g4b4,
	viv2d_x4r4g4b4,
	viv2d_a4b4g4r4,
	viv2d_x4b4g4r4,
	viv2d_a8,
} viv2d_color_format;

typedef struct _viv2d_format {
	viv2d_color_format col_fmt;
	int bpp;
	int depth;
	int fmt;
	int swizzle;

} viv2d_format;

typedef struct _viv2d_surface {
	unsigned int width;
	unsigned int height;
	unsigned int pitch;

	viv2d_format format;

	struct etna_bo *bo;
} viv2d_surface;

typedef enum {
	viv2d_cmd_clear = 0,
	viv2d_cmd_line,
	viv2d_cmd_bitblt,
	viv2d_cmd_stretchblt,
	viv2d_cmd_multsrcblt
} viv2d_op_cmd;

typedef enum {
	viv2d_blend_clear,
	viv2d_blend_src,
	viv2d_blend_dst,
	viv2d_blend_over,
	viv2d_blend_over_rev,
	viv2d_blend_in,
	viv2d_blend_in_rev,
	viv2d_blend_out,
	viv2d_blend_out_rev,
	viv2d_blend_atop,
	viv2d_blend_atop_rev,
	viv2d_blend_xor,
	viv2d_blend_add,
	viv2d_blend_saturate
} viv2d_blend_cmd;

typedef struct _viv2d_blend {
	viv2d_blend_cmd cmd;
	int src_mode;
	int dst_mode;
} viv2d_blend;

typedef struct _viv2d_op {
	viv2d_op_cmd cmd;

	viv2d_surface *src;
	viv2d_surface *pat;
	viv2d_surface *dst;

	viv2d_surface *src1;

	int src_x;
	int src_y;
	int rop;
	int color;

	int num_rects;
	viv2d_rect rects[VIV2D_MAX_RECTS];

	viv2d_rect clip_rect;

	bool src_global;
	uint8_t src_alpha;
	bool dst_global;
	uint8_t dst_alpha;
	uint8_t is_blend;
	viv2d_blend blend;

} viv2d_op;

// device
viv2d_device *viv2d_device_open(char *devname);
void viv2d_commit(viv2d_device *dev);
void viv2d_flush(viv2d_device *dev);
void viv2d_device_close(viv2d_device *dev);

// surface
viv2d_surface *viv2d_surface_alloc(unsigned int width, unsigned int height, viv2d_color_format fmt);
viv2d_surface *viv2d_surface_new(viv2d_device *dev, unsigned int width, unsigned int height, viv2d_color_format fmt);
#ifdef VIV2D_BMP_DUMP
void viv2d_surface_to_bmp(viv2d_surface *surf, const char *filename);
#endif
void viv2d_surface_del(viv2d_surface *surf);

// op
viv2d_op *viv2d_op_new(viv2d_op_cmd cmd, viv2d_surface *src, viv2d_surface *dst);
void viv2d_op_add_rect(viv2d_op *op, unsigned int x, unsigned int y, unsigned int width, unsigned int height );
void viv2d_op_set_clip_rect(viv2d_op *op, int x, int y, int width, int height );
void viv2d_op_set_color(viv2d_op *op, int col);
void viv2d_op_set_blend(viv2d_op *op, viv2d_blend_cmd cmd, bool src_global, uint8_t src_alpha, bool dst_global, uint8_t dst_alpha);
void viv2d_op_exec(viv2d_device *dev, viv2d_op *op);
void viv2d_op_del(viv2d_op *op);

struct etna_bo *etna_bo_from_usermem_prot(viv2d_device *v2d, void *memory, size_t size, int flags);

#endif