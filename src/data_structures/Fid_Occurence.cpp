#include "Fid_Occurence.h"

bool Fid_Occurence::compare(const Fid_Occurence &obj1, const Fid_Occurence &obj2){
	return obj1.fid < obj2.fid;
}