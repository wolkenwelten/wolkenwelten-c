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

void initMeshobjs(){
	meshPear          = meshNewRO( pear_verts,                   pear_count );
	meshHook          = meshNewRO( hook_verts,                   hook_count );
	meshGrenade       = meshNewRO( grenade_verts,             grenade_count );
	meshBomb          = meshNewRO( bomb_verts,                   bomb_count );
	meshAxe           = meshNewRO( axe_verts,                     axe_count );
	meshPickaxe       = meshNewRO( pickaxe_verts,             pickaxe_count );
	meshMasterblaster = meshNewRO( masterblaster_verts, masterblaster_count );

	texPear           = textureNew( gfx_pear_png_data,                   gfx_pear_png_len );
	texHook           = textureNew( gfx_hook_png_data,                   gfx_hook_png_len );
	texGrenade        = textureNew( gfx_grenade_png_data,             gfx_grenade_png_len );
	texAxe            = textureNew( gfx_axe_png_data,                     gfx_axe_png_len );
	texPickaxe        = textureNew( gfx_pickaxe_png_data,             gfx_pickaxe_png_len );
	texMasterblaster  = textureNew( gfx_masterblaster_png_data, gfx_masterblaster_png_len );

	meshPear->tex          = texPear;
	meshHook->tex          = texHook;
	meshGrenade->tex       = texGrenade;
	meshBomb->tex          = texGrenade;
	meshAxe->tex           = texAxe;
	meshPickaxe->tex       = texPickaxe;
	meshMasterblaster->tex = texMasterblaster;

	meshFinish( meshPear,          GL_STATIC_DRAW );
	meshFinish( meshHook,          GL_STATIC_DRAW );
	meshFinish( meshGrenade,       GL_STATIC_DRAW );
	meshFinish( meshBomb,          GL_STATIC_DRAW );
	meshFinish( meshAxe,           GL_STATIC_DRAW );
	meshFinish( meshPickaxe,       GL_STATIC_DRAW );
	meshFinish( meshMasterblaster, GL_STATIC_DRAW );
}
