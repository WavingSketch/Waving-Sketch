#include "Sketch.h"
#include "FAGMS.h"
#include "JoinSketch_ver3.h"
#include "AGMS.h"
#include "SkimSketch.h"
#include "CM_Sketch.h"
#include "WavingSketch.h"
int outerseed;
Sketch* Choose_Sketch(uint32_t w, uint32_t d, uint32_t hash_seed = 1000,int id=0){
	switch (id){
		// JoinSketch
		case 0:return new Classifier(w,d,hash_seed);
		// FAGMS
		case 1:return new C_Sketch(w,d,hash_seed);
		// SkimSketch
		case 2:return new SkimSketch(w,d,hash_seed);
		// CM_Sketch
		case 3:return new CM_Sketch(w,d,hash_seed);
		// Waving_Sketch
		case 4:return new WavingSketch<16, 16>(w, d, hash_seed);
		// Waving_Sketch
		case 5:return new WavingSketch<32, 32>(w, d, hash_seed);
		// Waving_Sketch
		case 6:return new WavingSketch<64, 64>(w, d, hash_seed);
	}
	return NULL;
}
