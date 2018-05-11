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
	              *surf6, *surf7, *surf8, *surf9, *surf10;
	viv2d_op *op;

	viv2d_rect r;
#if 1

	surf = viv2d_surface_new(dev, 1, 2, viv2d_a8r8g8b8);

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


//	op13 = viv2d_op_new(viv2d_cmd_line, NULL, surf10);
	op = viv2d_op_new(viv2d_cmd_bitblt, NULL, surf10);
//	viv2d_op_set_blend(op, viv2d_blend_xor, 0xff, 0xff);
//	viv2d_op_set_color(op, 0xff4040ff); // ABGR
	op->pat = surf1;
//	op13->rop = 0xcc;
	op->rop = 0xf0;
//	viv2d_op_add_rect(op13, 64, 64, 128, 128);
//	viv2d_op_add_rect(op13, 0, 0, 0, 0);
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
	viv2d_surface_to_bmp(surf10, "out/pat.bmp");

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
	viv2d_surface *src, *mask, *tmp, *dest, *out;
	viv2d_op *op;

	src = viv2d_surface_new(dev, 128, 128, viv2d_x8r8g8b8);
	mask = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	tmp = viv2d_surface_new(dev, 128, 128, viv2d_a8r8g8b8);
	dest = viv2d_surface_new(dev, 256, 256, viv2d_x8r8g8b8);
	out = viv2d_surface_new(dev, 256, 256, viv2d_a8r8g8b8);

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

	viv2d_surface_to_bmp(src, "out/blend_src.bmp");
	viv2d_surface_to_bmp(mask, "out/blend_mask.bmp");
	viv2d_surface_to_bmp(tmp, "out/blend_tmp.bmp");
	viv2d_surface_to_bmp(dest, "out/blend_dest1.bmp");
	viv2d_surface_to_bmp(out, "out/blend_dest.bmp");

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

int main(int argc, char *argv[])
{
	viv2d_device *dev;

	dev = viv2d_device_open(argv[1]);

	printf("TEST START\n");
	test1(dev);
	test_blend1(dev);
	test_blend2(dev);
	test_colorconversion(dev);
//	test_usermem(dev);
	printf("TEST END\n");

	viv2d_device_close(dev);
	return 0;
}
