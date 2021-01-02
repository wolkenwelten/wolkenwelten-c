#pragma once

#ifndef __APPLE__

#ifdef __aarch64__
	#define WW_ASM_PARTICLE_POS_UPDATE
	#define WW_ASM_SPARTICLE_POS_UPDATE
	#define WW_ASM_RAIN_POS_UPDATE
#endif

#ifdef __x86_64__
	#define WW_ASM_PARTICLE_POS_UPDATE
	#define WW_ASM_SPARTICLE_POS_UPDATE
	#define WW_ASM_RAIN_POS_UPDATE
#endif

#endif
