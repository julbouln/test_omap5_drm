#include <stdlib.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "write_bmp.h"
#include "xf86drm.h"
#include "etnaviv_drmif.h"
#include "etnaviv_drm.h"

#include "viv2d.h"

#include "state.xml.h"
#include "state_2d.xml.h"
#include "cmdstream.xml.h"

void test1(viv2d_device *dev) {
	viv2d_surface *surf;

	viv2d_surface *surf1, *surf2, *surf3, *surf4, *surf5,
	              *surf6, *surf7, *surf8, *surf9, *surf10, *surfpat;
	viv2d_op *op;

	viv2d_rect r;
#if 1

	surf = viv2d_surface_new(dev, 1, 2, viv2d_a8r8g8b8);

	surfpat = viv2d_surface_new(dev, 8, 8, viv2d_a8r8g8b8);

	surf1 = viv2d_surface_new(dev, 64, 64, viv2d_a8r8g8b8);
	surf2 = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	surf3 = viv2d_surface_new(dev, 256, 192, viv2d_a8r8g8b8);
	surf4 = viv2d_surface_new(dev, 256, 192, viv2d_a8r8g8b8);
	surf5 = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);
	surf6 = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);
	surf7 = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);

	surf8 = viv2d_surface_new(dev, 1, 1, viv2d_a8r8g8b8);
	surf9 = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);
	surf10 = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);

	// clear
	op = viv2d_op_new(viv2d_cmd_clear, NULL, surfpat);
	viv2d_op_set_color(op, 0xff40ff40);// green/ABGR
	viv2d_op_add_rect(op, 0, 0, 4, 4);
	viv2d_op_add_rect(op, 4, 4, 4, 4);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, surfpat);
	viv2d_op_set_color(op, 0xff4040ff); // red/ABGR
	viv2d_op_add_rect(op, 0, 4, 4, 4);
	viv2d_op_add_rect(op, 4, 0, 4, 4);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	// clear
	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf1);
	viv2d_op_set_color(op, 0xff40ff40);// green/ABGR
	viv2d_op_add_rect(op, 0, 0, 32, 32);
	viv2d_op_add_rect(op, 32, 32, 32, 32);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf1);
	viv2d_op_set_color(op, 0xff4040ff); // red/ABGR
	viv2d_op_add_rect(op, 0, 32, 32, 32);
	viv2d_op_add_rect(op, 32, 0, 32, 32);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("clear\n");

	// bitblt
	op = viv2d_op_new(viv2d_cmd_bitblt, surf1, surf2);
	viv2d_op_add_rect(op, 0, 0, 64, 64);
	viv2d_op_add_rect(op, 0, 64, 64, 64);
	viv2d_op_add_rect(op, 64, 0, 64, 64);
	viv2d_op_add_rect(op, 64, 64, 64, 64);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("bitblt\n");

	// stretchblt
	op = viv2d_op_new(viv2d_cmd_stretchblt, surf2, surf3);
	viv2d_op_add_rect(op, 32, 32, 128, 128);
	viv2d_op_add_rect(op, 0, 0, 64, 64);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("stretchblt\n");

	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf4);
	viv2d_op_set_color(op, 0xff7f22ff); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("clear\n");

	op = viv2d_op_new(viv2d_cmd_line, NULL, surf4);
	viv2d_op_set_color(op, 0xffff4040); // ABGR
	viv2d_op_add_rect(op, 0, 0, 128, 128);
	viv2d_op_add_rect(op, 0, 64, 128, 64);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("line\n");

	/*
		op6 = viv2d_op_new(viv2d_cmd_bitblt, surf4, surf3);
		viv2d_op_set_blend(op6, viv2d_blend_in, 0x00, 0x00);
		viv2d_op_add_rect(op6, 64, 64, 128, 128);
		viv2d_op_exec(dev, op6);
		viv2d_commit(dev);
	*/

	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf5);
	viv2d_op_add_rect(op, 64, 64, 128, 128);
	viv2d_op_set_color(op, 0xff4040ff); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("clear\n");

	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf6);
	viv2d_op_add_rect(op, 128, 128, 128, 128);
	viv2d_op_set_color(op, 0xaa40ff40); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("clear\n");

	op = viv2d_op_new(viv2d_cmd_bitblt, surf5, surf6);
	viv2d_op_set_blend(op, viv2d_blend_xor, false, 0xff, false, 0xff);
//	viv2d_op_add_rect(op9, 64, 64, 128, 128);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("bitblt\n");

	/*
		op10 = viv2d_op_new(viv2d_cmd_multsrcblt, surf3, surf7);
		viv2d_op_set_blend(op12, viv2d_blend_src, false, 0xff, false, 0xff);
		op10->src1 = surf6;
	//	viv2d_op_add_rect(op10, 64, 64, 128, 128);
		viv2d_op_exec(dev, op10);
		viv2d_commit(dev);

		printf("mutlsrcblt\n");
	*/
	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf8);
	viv2d_op_set_color(op, 0xff4040ff); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	printf("clear\n");


	op = viv2d_op_new(viv2d_cmd_stretchblt, surf8, surf9);
	viv2d_op_set_blend(op, viv2d_blend_xor, false, 0xff, false, 0xff);
//	viv2d_op_add_rect(op, 32, 32, 1, 128);
//	viv2d_op_add_rect(op, 32, 32, 128, 1);
//	viv2d_op_add_rect(op, 64, 64, 128, 128);
//	viv2d_op_add_rect(op, 0, 0, 32, 32);
//	viv2d_op_add_rect(op, 0, 0, 0, 0);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);


	viv2d_commit(dev);

#if 0
//	op13 = viv2d_op_new(viv2d_cmd_line, NULL, surf10);
	op = viv2d_op_new(viv2d_cmd_bitblt, NULL, surf10);
//	viv2d_op_set_blend(op, viv2d_blend_xor, 0xff, 0xff);
//	viv2d_op_set_color(op, 0xff4040ff); // ABGR
	op->pat = surfpat;
//	op13->rop = 0xcc;
	op->rop = 0xf0;
//	viv2d_op_add_rect(op13, 64, 64, 128, 128);
//	viv2d_op_add_rect(op13, 0, 0, 0, 0);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);
#endif
	op = viv2d_op_new(viv2d_cmd_bitblt, NULL, surf10);
	op->pat_fill = 1;
	viv2d_op_set_color(op, 0xff4040ff);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	viv2d_commit(dev);

	viv2d_surface_to_bmp(surf1, "out/checkbrd_pat.bmp");
	viv2d_surface_to_bmp(surf2, "out/checkbrd.bmp");
	viv2d_surface_to_bmp(surf3, "out/checkbrd_stretch.bmp");
	viv2d_surface_to_bmp(surf4, "out/line.bmp");
	viv2d_surface_to_bmp(surf6, "out/blend.bmp");
	viv2d_surface_to_bmp(surf7, "out/multisrc.bmp");
	viv2d_surface_to_bmp(surf9, "out/stretch.bmp");
	viv2d_surface_to_bmp(surfpat, "out/pat.bmp");
	viv2d_surface_to_bmp(surf10, "out/out_pat.bmp");

	viv2d_surface_del(surf1);
	viv2d_surface_del(surf2);
	viv2d_surface_del(surf3);
	viv2d_surface_del(surf4);
	viv2d_surface_del(surf5);
	viv2d_surface_del(surf6);
	viv2d_surface_del(surf7);
	viv2d_surface_del(surf8);
	viv2d_surface_del(surf9);
	viv2d_surface_del(surf10);

#endif
}

void test_blend1(viv2d_device *dev) {
	viv2d_surface *src, *mask, *tmp, *dest;
	viv2d_op *op;

	src = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);
	mask = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);
	tmp = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);
	dest = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, src);
	viv2d_op_set_color(op, 0xff40ff40); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, mask);
	viv2d_op_add_rect(op, 96, 96, 64, 64);
	viv2d_op_set_color(op, 0xff000000); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, src, tmp);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, mask, tmp);
	viv2d_op_set_blend(op, viv2d_blend_in_rev, false, 0xff, false, 0xff);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, tmp, dest);
	viv2d_op_set_blend(op, viv2d_blend_over, false, 0xff, false, 0xff);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	viv2d_commit(dev);

	viv2d_surface_to_bmp(src, "out/blend1_src.bmp");
	viv2d_surface_to_bmp(mask, "out/blend1_mask.bmp");
	viv2d_surface_to_bmp(tmp, "out/blend1_tmp.bmp");
	viv2d_surface_to_bmp(dest, "out/blend1_dest.bmp");

	viv2d_surface_del(src);
	viv2d_surface_del(mask);
	viv2d_surface_del(tmp);
	viv2d_surface_del(dest);

}


void test_blend2(viv2d_device *dev) {
	viv2d_surface *src, *mask, *tmp, *dest, *dest_mul, *out, *out_mul;
	viv2d_op *op;

	src = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	mask = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	tmp = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	dest = viv2d_surface_new(dev, 256, 256, viv2d_x8r8g8b8);
	out = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);

	dest_mul = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	out_mul = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, src);
	viv2d_op_add_rect(op, 0, 0, 64, 64);
	viv2d_op_add_rect(op, 64, 64, 64, 64);
	viv2d_op_set_color(op, 0xff4040ff); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, dest);
	viv2d_op_set_color(op, 0xffffffff); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, mask);
	viv2d_op_add_rect(op, 64, 64, 32, 32);
	viv2d_op_add_rect(op, 32, 32, 32, 32);
//	viv2d_op_add_rect(op, 32, 4, 64, 6);

	viv2d_op_set_color(op, 0xaa000000); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, tmp);
//	viv2d_op_add_rect(op, 64, 64, 128, 128);
	viv2d_op_set_color(op, 0xff40ff40); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, mask, tmp);
	viv2d_op_set_blend(op, viv2d_blend_in_rev, false, 0x00, false, 0x00);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, tmp, dest);
	viv2d_op_add_rect(op, 64, 64, 128, 128);
	viv2d_op_set_blend(op, viv2d_blend_over, false, 0x00, false, 0x00);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, dest, out);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	viv2d_commit(dev);

	/*
		op = viv2d_op_new(viv2d_cmd_clear, NULL, dest_mul);
		viv2d_op_set_color(op, 0xff40ff40); // ABGR
		viv2d_op_exec(dev, op);
		viv2d_op_del(op);
	*/
	op = viv2d_op_new(viv2d_cmd_multsrcblt, NULL, dest_mul);
	viv2d_op_set_blend(op, viv2d_blend_add, false, 0x00, false, 0x00);
	op->srcs[0] = src;
	op->srcs[1] = mask;
	op->srcs[2] = src;
	op->srcs[3] = mask;
	op->src_count = 4;
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, dest_mul, out_mul);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	viv2d_commit(dev);

	viv2d_surface_to_bmp(src, "out/blend_src.bmp");
	viv2d_surface_to_bmp(mask, "out/blend_mask.bmp");
	viv2d_surface_to_bmp(tmp, "out/blend_tmp.bmp");
	viv2d_surface_to_bmp(dest, "out/blend_dest.bmp");
	viv2d_surface_to_bmp(dest_mul, "out/blend_dest_mul.bmp");
	viv2d_surface_to_bmp(out, "out/blend_out.bmp");
	viv2d_surface_to_bmp(out_mul, "out/blend_out_mul.bmp");

	viv2d_surface_del(src);
	viv2d_surface_del(mask);
	viv2d_surface_del(tmp);
	viv2d_surface_del(dest);
	viv2d_surface_del(out);

}

void test_usermem(viv2d_device *dev) {
	uint8_t *buf;
	viv2d_surface *surf, *out;
	viv2d_op *op;
	surf = viv2d_surface_alloc(1024 * 2, 1024, viv2d_a8r8g8b8);
	size_t size = PAGE_ALIGN(surf->height * surf->pitch);
	buf = (uint8_t*)aligned_alloc(PAGE_SIZE, size);
	memset(buf, 0, size);

	surf->bo = etna_bo_from_usermem_prot(dev->dev, buf, size, (ETNA_USERPTR_READ | ETNA_USERPTR_WRITE));

	printf("usermem bo:%p buf:%p map:%p handle:%d size:%d pitch:%d\n", surf->bo, buf, etna_bo_map(surf->bo), etna_bo_handle(surf->bo), size, surf->pitch);

	out = viv2d_surface_new(dev, 64, 64, viv2d_a8r8g8b8);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf);
	viv2d_op_set_color(op, 0xff40ff40);// green/ABGR
	viv2d_op_add_rect(op, 0, 0, 32, 32);
	viv2d_op_add_rect(op, 32, 32, 32, 32);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, surf);
	viv2d_op_set_color(op, 0xff4040ff); // red/ABGR
	viv2d_op_add_rect(op, 0, 32, 32, 32);
	viv2d_op_add_rect(op, 32, 0, 32, 32);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, surf, out);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	viv2d_commit(dev);

	viv2d_surface_to_bmp(out, "out/usermem.bmp");

//	etna_bo_wait(dev->dev, surf->bo);
	viv2d_surface_del(surf);
	free(buf);
}

void test_colorconversion(viv2d_device *dev) {
	viv2d_surface *src, *mask, *tmp, *dest, *destalpha;
	viv2d_op *op;

	src = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	mask = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	tmp = viv2d_surface_new(dev, 128, 128, viv2d_r5g6b5);
	dest = viv2d_surface_new(dev, 128, 128, viv2d_x8r8g8b8);
	destalpha = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, src);
	viv2d_op_set_color(op, 0xff40ff40); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, mask);
	viv2d_op_add_rect(op, 32, 32, 32, 32);
	viv2d_op_add_rect(op, 32, 4, 64, 6);
	viv2d_op_set_color(op, 0xaa000000); // ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, src);
	viv2d_op_set_color(op, 0xff4040ff);// green/ABGR
	viv2d_op_add_rect(op, 0, 0, 32, 32);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, src, tmp);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_clear, NULL, tmp);
	viv2d_op_set_color(op, 0xff4040ff);// green/ABGR
	viv2d_op_add_rect(op, 32, 32, 32, 32);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, tmp, dest);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, mask, dest);
	viv2d_op_set_blend(op, viv2d_blend_in_rev, false, 0x00, false, 0x00);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, tmp, destalpha);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	op = viv2d_op_new(viv2d_cmd_bitblt, mask, destalpha);
	viv2d_op_set_blend(op, viv2d_blend_in_rev, false, 0x00, false, 0x00);
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	viv2d_commit(dev);

	viv2d_surface_to_bmp(src, "out/col_conv_src.bmp");
	viv2d_surface_to_bmp(mask, "out/col_conv_mask.bmp");
	viv2d_surface_to_bmp(dest, "out/col_conv_dest.bmp");
	viv2d_surface_to_bmp(destalpha, "out/col_conv_destalpha.bmp");

}

int test_monochrome(viv2d_device *dev) {

	/* Text data to expand, packed per 8x4 bits in 32 bit words */
	uint32_t text_width = 128;
	uint32_t text_height = 8;
	uint32_t text_data[] = {
		0xa9aada89, 0x00898a88, 0xc20528c8, 0x00c82825, 0x00008080, 0x00808000,
		0x724a49f0, 0x00f24a4b, 0x27284887, 0x002728e0, 0x0808881c, 0x001c8888,
		0x80804830, 0x00304880, 0x08080000, 0x00020508, 0xa29c0000, 0x001c20be,
		0xcab10000, 0x00838083, 0x02e60002, 0x00c722c2, 0x221c0000, 0x001c2222,
		0xc8b00000, 0x00888888, 0x03000807, 0x00070800, 0x00808000, 0x00189880,
		0xa8988870, 0x007088c8
	};
	uint32_t text_size = sizeof(text_data) / 4;

	printf("mono blit %d\n",text_size);

	viv2d_surface *dest;
	dest = viv2d_surface_new(dev, 256, 192, viv2d_a8r8g8b8);

	viv2d_op *op;
	op = viv2d_op_new(viv2d_cmd_mono_blit, NULL, dest);
	viv2d_op_add_rect(op, 4, 4, text_width, text_height);

	op->data = text_data;
	op->data_pitch = 4;
	op->data_size = text_size;
	viv2d_op_set_color(op, 0xff40ff40);// green/ABGR
	viv2d_op_exec(dev, op);
	viv2d_op_del(op);

	viv2d_commit(dev);

	viv2d_surface_to_bmp(dest, "out/mono_dest.bmp");

#if 0
	etna_set_state(dev->stream, VIVS_DE_SRC_ADDRESS, 0); /* source address is ignored for monochrome blits */
	//etna_set_state(ctx, VIVS_DE_SRC_ADDRESS, etna_bo_gpu_address(src));
	etna_set_state(dev->stream, VIVS_DE_SRC_STRIDE, 4);
	etna_set_state(dev->stream, VIVS_DE_SRC_ROTATION_CONFIG, 0);
	etna_set_state(dev->stream, VIVS_DE_SRC_CONFIG,
	               VIVS_DE_SRC_CONFIG_UNK16 |
	               VIVS_DE_SRC_CONFIG_SOURCE_FORMAT(DE_FORMAT_MONOCHROME) |
	               VIVS_DE_SRC_CONFIG_LOCATION_STREAM |
	               VIVS_DE_SRC_CONFIG_PACK_PACKED8 | /* 8 by 4 blocks */
	               VIVS_DE_SRC_CONFIG_PE10_SOURCE_FORMAT(DE_FORMAT_MONOCHROME));
	etna_set_state(dev->stream, VIVS_DE_SRC_ORIGIN, 0);
	etna_set_state(dev->stream, VIVS_DE_SRC_SIZE, 0); // source size is ignored
	etna_set_state(dev->stream, VIVS_DE_SRC_COLOR_BG, 0xff2020ff);
	etna_set_state(dev->stream, VIVS_DE_SRC_COLOR_FG, 0xffffffff);
	etna_set_state(dev->stream, VIVS_DE_STRETCH_FACTOR_LOW, 0);
	etna_set_state(dev->stream, VIVS_DE_STRETCH_FACTOR_HIGH, 0);
	etna_set_state(dev->stream, VIVS_DE_DEST_ADDRESS, etna_bo_gpu_address(bmp));
	etna_set_state(dev->stream, VIVS_DE_DEST_STRIDE, width * 4);
	etna_set_state(dev->stream, VIVS_DE_DEST_ROTATION_CONFIG, 0);
	etna_set_state(dev->stream, VIVS_DE_DEST_CONFIG,
	               VIVS_DE_DEST_CONFIG_FORMAT(DE_FORMAT_A8R8G8B8) |
	               VIVS_DE_DEST_CONFIG_COMMAND_BIT_BLT |
	               VIVS_DE_DEST_CONFIG_SWIZZLE(DE_SWIZZLE_ARGB) |
	               VIVS_DE_DEST_CONFIG_TILED_DISABLE |
	               VIVS_DE_DEST_CONFIG_MINOR_TILED_DISABLE
	              );

	etna_set_state(dev->stream, VIVS_DE_ROP,
	               VIVS_DE_ROP_ROP_FG(0xcc) | VIVS_DE_ROP_ROP_BG(0xcc) | VIVS_DE_ROP_TYPE_ROP4);
	etna_set_state(dev->stream, VIVS_DE_CLIP_TOP_LEFT,
	               VIVS_DE_CLIP_TOP_LEFT_X(0) |
	               VIVS_DE_CLIP_TOP_LEFT_Y(0)
	              );
	etna_set_state(dev->stream, VIVS_DE_CLIP_BOTTOM_RIGHT,
	               VIVS_DE_CLIP_BOTTOM_RIGHT_X(width) |
	               VIVS_DE_CLIP_BOTTOM_RIGHT_Y(height)
	              );
	etna_set_state(dev->stream, VIVS_DE_CONFIG, 0); /* TODO */
	etna_set_state(dev->stream, VIVS_DE_SRC_ORIGIN_FRACTION, 0);
	etna_set_state(dev->stream, VIVS_DE_ALPHA_CONTROL, 0);
	etna_set_state(dev->stream, VIVS_DE_ALPHA_MODES, 0);
	etna_set_state(dev->stream, VIVS_DE_DEST_ROTATION_HEIGHT, 0);
	etna_set_state(dev->stream, VIVS_DE_SRC_ROTATION_HEIGHT, 0);
	etna_set_state(dev->stream, VIVS_DE_ROT_ANGLE, 0);

	/* Clear color PE20 */
	etna_set_state(dev->stream, VIVS_DE_CLEAR_PIXEL_VALUE32, 0xff40ff40);
	/* Clear color PE10 */
	etna_set_state(dev->stream, VIVS_DE_CLEAR_BYTE_MASK, 0xff);
	etna_set_state(dev->stream, VIVS_DE_CLEAR_PIXEL_VALUE_LOW, 0xff40ff40);
	etna_set_state(dev->stream, VIVS_DE_CLEAR_PIXEL_VALUE_HIGH, 0xff40ff40);

	etna_set_state(dev->stream, VIVS_DE_DEST_COLOR_KEY, 0);
	etna_set_state(dev->stream, VIVS_DE_GLOBAL_SRC_COLOR, 0);
	etna_set_state(dev->stream, VIVS_DE_GLOBAL_DEST_COLOR, 0);
	etna_set_state(dev->stream, VIVS_DE_COLOR_MULTIPLY_MODES, 0);
	etna_set_state(dev->stream, VIVS_DE_PE_TRANSPARENCY, 0);
	etna_set_state(dev->stream, VIVS_DE_PE_CONTROL, 0);
	etna_set_state(dev->stream, VIVS_DE_PE_DITHER_LOW, 0xffffffff);
	etna_set_state(dev->stream, VIVS_DE_PE_DITHER_HIGH, 0xffffffff);

	/* Queue DE command */
	etna_reserve(dev->stream, 256 * 2 + 2 + text_size);
	(ctx)->buf[(dev->stream)->offset++] = VIV_FE_DRAW_2D_HEADER_OP_DRAW_2D |
	                                      VIV_FE_DRAW_2D_HEADER_COUNT(NUM_RECTS) |
	                                      VIV_FE_DRAW_2D_HEADER_DATA_COUNT(text_size);
	(ctx)->offset++; /* rectangles start aligned */
	for (int rec = 0; rec < NUM_RECTS; ++rec)
	{
		int x1 = 4 + 16 * rec;
		int y1 = 4;
		int x2 = x1 + text_width;
		int y2 = y1 + text_height;
		(dev->stream)->buf[(dev->stream)->offset++] = VIV_FE_DRAW_2D_TOP_LEFT_X(x1) |
		        VIV_FE_DRAW_2D_TOP_LEFT_Y(y1);
		(dev->stream)->buf[(dev->stream)->offset++] = VIV_FE_DRAW_2D_BOTTOM_RIGHT_X(x2) |
		        VIV_FE_DRAW_2D_BOTTOM_RIGHT_Y(y2);
	}
	/* data in-stream */
	ETNA_ALIGN(dev->stream);
	for (int ptr = 0; ptr < text_size; ++ptr)
		(dev->stream)->buf[(ctx)->offset++] = text_data[ptr];
	ETNA_ALIGN(ctx);
	etna_set_state(dev->stream, 1, 0);
	etna_set_state(dev->stream, 1, 0);
	etna_set_state(dev->stream, 1, 0);

	etna_set_state(dev->stream, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_PE2D);
	etna_finish(dev->stream);
#endif
}

int main(int argc, char *argv[])
{
	viv2d_device *dev;

	dev = viv2d_device_open(argv[1]);

	viv2d_surface *surf = viv2d_surface_new(dev, 64, 64, viv2d_a8r8g8b8);
	printf("FIRST bo %p\n", etna_bo_map(surf->bo));
	viv2d_surface_del(surf);
	surf = viv2d_surface_new(dev, 64, 64, viv2d_a8r8g8b8);
	printf("FIRST bo %p\n", etna_bo_map(surf->bo));

	printf("TEST START\n");
	test1(dev);
	test_blend1(dev);
	test_blend2(dev);
	test_colorconversion(dev);
	test_monochrome(dev);
//	test_usermem(dev);
	printf("TEST END\n");

	viv2d_device_close(dev);
	return 0;
}
