#include "V_Occurence.h"

bool V_Occurence::compare(const V_Occurence &obj1, const V_Occurence &obj2){
	return obj1.vid < obj2.vid;
}