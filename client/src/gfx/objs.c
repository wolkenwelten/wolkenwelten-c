#include "objs.h"
#include "../tmp/assets.h"
#include "../tmp/meshassets.h"
#include "../gfx/texture.h"
#include "../gfx/mesh.h"
#include "../gfx/glew.h"

mesh *meshPear;
texture *texPear;

mesh *meshHook;
texture *texHook;

mesh *meshGrenade;
mesh *meshBomb;
texture *texGrenade;

mesh *meshAxe;
texture *texAxe;

mesh *meshPickaxe;
texture *texPickaxe;

mesh *meshMasterblaster;
texture *texMasterblaster;

mesh *meshBlaster;
texture *texBlaster;

void initMeshobjs(){
	meshPear          = meshNewRO( pear_verts,                   pear_count );
	meshHook          = meshNewRO( hook_verts,                   hook_count );
	meshGrenade       = meshNewRO( grenade_verts,             grenade_count );
	meshBomb          = meshNewRO( bomb_verts,                   bomb_count );
	meshAxe           = meshNewRO( axe_verts,                     axe_count );
	meshPickaxe       = meshNewRO( pickaxe_verts,             pickaxe_count );
	meshMasterblaster = meshNewRO( masterblaster_verts, masterblaster_count );
	meshBlaster       = meshNewRO( blaster_verts,             blaster_count );


	texPear           = textureNew( gfx_pear_png_data,                   gfx_pear_png_len,"client/gfx/pear.png");
	texHook           = textureNew( gfx_hook_png_data,                   gfx_hook_png_len,"client/gfx/hook.png");
	texGrenade        = textureNew( gfx_grenade_png_data,             gfx_grenade_png_len,"client/gfx/grenade.png");
	texAxe            = textureNew( gfx_axe_png_data,                     gfx_axe_png_len,"client/gfx/axe.png");
	texPickaxe        = textureNew( gfx_pickaxe_png_data,             gfx_pickaxe_png_len,"client/gfx/pickaxe.png");
	texMasterblaster  = textureNew( gfx_masterblaster_png_data, gfx_masterblaster_png_len,"client/gfx/masterblaster.png");
	texBlaster        = textureNew( gfx_blaster_png_data,             gfx_blaster_png_len,"client/gfx/blaster.png");

	meshPear->tex          = texPear;
	meshHook->tex          = texHook;
	meshGrenade->tex       = texGrenade;
	meshBomb->tex          = texGrenade;
	meshAxe->tex           = texAxe;
	meshPickaxe->tex       = texPickaxe;
	meshMasterblaster->tex = texMasterblaster;
	meshBlaster->tex       = texBlaster;

	meshFinish( meshPear,          GL_STATIC_DRAW );
	meshFinish( meshHook,          GL_STATIC_DRAW );
	meshFinish( meshGrenade,       GL_STATIC_DRAW );
	meshFinish( meshBomb,          GL_STATIC_DRAW );
	meshFinish( meshAxe,           GL_STATIC_DRAW );
	meshFinish( meshPickaxe,       GL_STATIC_DRAW );
	meshFinish( meshMasterblaster, GL_STATIC_DRAW );
	meshFinish( meshBlaster,       GL_STATIC_DRAW );
}
