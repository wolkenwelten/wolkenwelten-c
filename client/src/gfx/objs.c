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
texture *texGrenade;

mesh *meshBomb;
texture *texBomb;

mesh *meshAxe;
texture *texAxe;

mesh *meshPickaxe;
texture *texPickaxe;

mesh *meshMasterblaster;
texture *texMasterblaster;

mesh *meshBlaster;
texture *texBlaster;

mesh *meshCrystalbullet;
texture *texCrystalbullet;

mesh *meshAssaultblaster;
texture *texAssaultblaster;

mesh *meshShotgunblaster;
texture *texShotgunblaster;

mesh *meshSunglasses;
texture *texSunglasses;

mesh *meshGlider;
texture *texGlider;

void initMeshobjs(){
	meshPear           = meshNewRO( pear_verts,                     pear_count );
	meshHook           = meshNewRO( hook_verts,                     hook_count );
	meshGrenade        = meshNewRO( grenade_verts,               grenade_count );
	meshBomb           = meshNewRO( bomb_verts,                     bomb_count );
	meshAxe            = meshNewRO( axe_verts,                       axe_count );
	meshPickaxe        = meshNewRO( pickaxe_verts,               pickaxe_count );
	meshMasterblaster  = meshNewRO( masterblaster_verts,   masterblaster_count );
	meshBlaster        = meshNewRO( blaster_verts,               blaster_count );
	meshCrystalbullet  = meshNewRO( crystalbullet_verts,   crystalbullet_count );
	meshAssaultblaster = meshNewRO( assaultblaster_verts, assaultblaster_count );
	meshShotgunblaster = meshNewRO( shotgunblaster_verts, shotgunblaster_count );
	meshSunglasses     = meshNewRO( sunglasses_verts,         sunglasses_count );
	meshGlider         = meshNewRO( glider_verts,                 glider_count );

	texPear           = textureNew( gfx_pear_png_data,                   gfx_pear_png_len,"client/gfx/pear.png");
	texHook           = textureNew( gfx_hook_png_data,                   gfx_hook_png_len,"client/gfx/hook.png");
	texBomb           = textureNew( gfx_bomb_png_data,             gfx_bomb_png_len,"client/gfx/bomb.png");
	texGrenade        = textureNew( gfx_grenade_png_data,             gfx_grenade_png_len,"client/gfx/grenade.png");
	texAxe            = textureNew( gfx_axe_png_data,                     gfx_axe_png_len,"client/gfx/axe.png");
	texPickaxe        = textureNew( gfx_pickaxe_png_data,             gfx_pickaxe_png_len,"client/gfx/pickaxe.png");
	texMasterblaster  = textureNew( gfx_masterblaster_png_data, gfx_masterblaster_png_len,"client/gfx/masterblaster.png");
	texBlaster        = textureNew( gfx_blaster_png_data,             gfx_blaster_png_len,"client/gfx/blaster.png");
	texCrystalbullet  = textureNew( gfx_crystalbullet_png_data, gfx_crystalbullet_png_len,"client/gfx/crystalbullet.png");
	texAssaultblaster = textureNew( gfx_assaultblaster_png_data, gfx_assaultblaster_png_len,"client/gfx/assaultblaster.png");
	texShotgunblaster = textureNew( gfx_shotgunblaster_png_data, gfx_shotgunblaster_png_len,"client/gfx/shotgunblaster.png");
	texSunglasses     = textureNew( gfx_sunglasses_png_data, gfx_sunglasses_png_len,"client/gfx/sunglasses.png");
	texGlider         = textureNew( gfx_glider_png_data, gfx_glider_png_len,"client/gfx/glider.png");

	meshPear->tex           = texPear;
	meshHook->tex           = texHook;
	meshGrenade->tex        = texGrenade;
	meshBomb->tex           = texBomb;
	meshAxe->tex            = texAxe;
	meshPickaxe->tex        = texPickaxe;
	meshMasterblaster->tex  = texMasterblaster;
	meshBlaster->tex        = texBlaster;
	meshCrystalbullet->tex  = texCrystalbullet;
	meshAssaultblaster->tex = texAssaultblaster;
	meshShotgunblaster->tex = texShotgunblaster;
	meshSunglasses->tex     = texSunglasses;
	meshGlider->tex         = texGlider;

	meshFinish( meshPear,           GL_STATIC_DRAW );
	meshFinish( meshHook,           GL_STATIC_DRAW );
	meshFinish( meshGrenade,        GL_STATIC_DRAW );
	meshFinish( meshBomb,           GL_STATIC_DRAW );
	meshFinish( meshAxe,            GL_STATIC_DRAW );
	meshFinish( meshPickaxe,        GL_STATIC_DRAW );
	meshFinish( meshMasterblaster,  GL_STATIC_DRAW );
	meshFinish( meshBlaster,        GL_STATIC_DRAW );
	meshFinish( meshCrystalbullet,  GL_STATIC_DRAW );
	meshFinish( meshAssaultblaster, GL_STATIC_DRAW );
	meshFinish( meshShotgunblaster, GL_STATIC_DRAW );
	meshFinish( meshSunglasses,     GL_STATIC_DRAW );
	meshFinish( meshGlider,         GL_STATIC_DRAW );
}
