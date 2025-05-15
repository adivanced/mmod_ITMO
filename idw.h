#ifndef __IDW_H__
#define __IDW_H__

#include "kd_tree.h"
#include "settings.h"


#if USE_ASM == 0

template<typename T, size_t K>
T idw(answer_nearest<T, K>*& answ, T* pointi){
	T result = 0.0;
	T tmp = 0.0;
	for(size_t i = 0; i < answ->N; i++){
		T dist = distance(&(answ->coords[i*K]), pointi, K);
		dist *= dist; // squared
		result += answ->values[i] / dist;
		tmp += 1.0 / dist;
		//printf("I:%ld dst:%f res+:%f tmp+:%f\n", i, dist, answ->values[i]/dist, 1.0/dist);
	}
	return result/tmp;
}

#else

extern "C" float idw(answer_nearest<float, dimensions>* answ, float* pointi);
#define numbertype float

#endif



#endif
