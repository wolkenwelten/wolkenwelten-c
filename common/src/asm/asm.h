#pragma once

#ifndef WW_NO_ASM

	#ifdef __aarch64__
		#define WW_ASM_PARTICLE_UPDATE
	#endif

	#ifdef __x86_64__
		#define WW_ASM_PARTICLE_UPDATE
	#endif

#endif
