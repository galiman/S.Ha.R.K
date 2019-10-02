# The Frame Buffer Device

ifndef BASE
BASE=../../..
endif

include $(BASE)/config/config.mk

LIBRARY       = osmesa

OBJS_PATH     = $(BASE)/ports/mesa/src/

OSMESA = accum.o api_arrayelt.o api_eval.o api_loopback.o api_noop.o api_validate.o\
		attrib.o blend.o buffers.o clip.o colortab.o context.o convolve.o\
		debug.o depth.o dispatch.o dlist.o drawpix.o enable.o enums.o eval.o\
		extensions.o feedback.o fog.o get.o glapi.o glthread.o hash.o hint.o\
		histogram.o image.o light.o lines.o matrix.o mmath.o pixel.o\
		points.o polygon.o rastpos.o state.o stencil.o texcompress.o texformat.o\
		teximage.o texobj.o texstate.o texstore.o texutil.o varray.o vpexec.o\
		vpparse.o vpstate.o vtxfmt.o ./array_cache/ac_context.o\
		./array_cache/ac_import.o ./swrast/s_aaline.o ./swrast/s_aatriangle.o\
		./swrast/s_accum.o ./swrast/s_alphabuf.o ./swrast/s_alpha.o ./swrast/s_bitmap.o\
		./swrast/s_blend.o ./swrast/s_buffers.o ./swrast/s_context.o ./swrast/s_copypix.o\
		./swrast/s_depth.o ./swrast/s_drawpix.o ./swrast/s_feedback.o ./swrast/s_fog.o\
		./swrast/s_histogram.o ./swrast/s_imaging.o ./swrast/s_lines.o ./swrast/s_logic.o\
		./swrast/s_masking.o ./swrast/s_pixeltex.o ./swrast/s_points.o ./swrast/s_readpix.o\
		./swrast/s_span.o ./swrast/s_stencil.o ./swrast/s_texstore.o ./swrast/s_texture.o\
		./swrast/s_triangle.o ./swrast/s_zoom.o ./swrastst/ss_context.o\
		./swrastst/ss_triangle.o ./swrastst/ss_vb.o ./tnl/t_array_api.o\
		./tnl/t_import_array.o ./tnl/t_context.o ./tnl/t_eval_api.o ./tnl/t_imm_alloc.o\
		./tnl/t_imm_api.o ./tnl/t_imm_debug.o ./tnl/t_imm_dlist.o ./tnl/t_imm_elt.o\
		./tnl/t_imm_eval.o ./tnl/t_imm_exec.o ./tnl/t_imm_fixup.o ./tnl/t_pipeline.o\
		./tnl/t_vb_fog.o ./tnl/t_vb_light.o\
		./tnl/t_vb_normals.o ./tnl/t_vb_points.o ./tnl/t_vb_program.o ./tnl/t_vb_render.o\
		./tnl/t_vb_gentex.o ./tnl/t_vb_texmat.o ./tnl/t_vb_vertex.o\
		./math/m_clip_debug.o ./math/m_norm_debug.o\
		./math/m_xform_debug.o ./math/m_eval.o ./math/m_matrix.o ./math/m_translate.o\
		./math/m_vector.o ./math/m_xform.o\
		./x86/asm_common_x86.o ./x86/common_x86.o ./x86/glapi_x86.o\
		./x86/mmx_blend.o ./x86/x86.o ./x86/x86_cliptest.o ./x86/xform2_x86.o\
		./x86/xform3_x86.o ./x86/xform4_x86.o ./osmesa/osmesa.o imports.o \
 
OBJS = $(OSMESA)

C_OPT += -I../../../drivers/linuxc26/include -I../include -I. -I..

ASM_OPT += -I../../../drivers/linuxc26/include/asm -I../include -I. -I..

include $(BASE)/config/lib.mk

