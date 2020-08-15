/* This file exists so we do not get undefined references when we link in code
 * meant for the client side.
 */

#include <stdio.h>

struct item;
typedef struct item item;
struct mesh;
typedef struct mesh mesh;
struct character;
typedef struct character character;


mesh *meshPear;
mesh *meshHook;
mesh *meshGrenade;
mesh *meshBomb;
mesh *meshAxe;
mesh *meshPickaxe;
mesh *meshMasterblaster;;

void recipeAdd1I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID, unsigned char nIngredAmount){
	(void)nResultID;
	(void)nResultAmount;
	(void)nIngredID;
	(void)nIngredAmount;

	fprintf(stderr,"recipeAdd1I got called on the serverside\n");
}
void recipeAdd2I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID1, unsigned char nIngredAmount1, unsigned short nIngredID2, unsigned char nIngredAmount2){
	(void)nResultID;
	(void)nResultAmount;
	(void)nIngredID1;
	(void)nIngredAmount1;
	(void)nIngredID2;
	(void)nIngredAmount2;

	fprintf(stderr,"recipeAdd2I got called on the serverside\n");
}

void ingredientSubstituteAdd(unsigned short ingredient, unsigned short substitute){
	(void)ingredient;
	(void)substitute;

	fprintf(stderr,"ingredientSubstituteAdd got called on the serverside\n");
}
