#ifndef __KD_TREE_H__
#define __KD_TREE_H__

#include <stdint.h>
#include <array>
#include <vector>
#include <iostream>
#include <math.h>
#include <string.h>
#include <queue>


template<typename T, size_t K>
struct point{
	T value;
	std::array<T, K> coords;

	point();
	point(const std::array<T, K>& coordsi, T valuei);
};

template<typename T, size_t K>
point<T, K>::point(){
	coords = {0};
	value = 0;
}

template<typename T, size_t K>
point<T, K>::point(const std::array<T, K>& coordsi, T valuei){
	coords = std::move(coordsi);
	value = std::move(valuei);
}


template<typename T, size_t K>
T distance(const point<T, K>& A, const point<T, K>& B){
	T result = 0.0;
	for(size_t i = 0; i < K; i++){
		result += (A.coords[i] - B.coords[i]) * (A.coords[i] - B.coords[i]);
	}
	return result;
}

template<typename T, size_t K>
T distance(const point<T, K>& A, std::array<T, K> B){
	T result = 0.0;
	for(size_t i = 0; i < K; i++){
		result += (A.coords[i] - B[i]) * (A.coords[i] - B[i]);
	}
	return result;
}

template<typename T>
T distance(T* A, T* B, uint64_t dims){
	T result = 0.0;
	for(size_t i = 0; i < dims; i++){
		result += (A[i] - B[i]) * (A[i] - B[i]);
	}
	return result;
}


template<typename T, size_t K>
struct kd_tree_node{
	struct point<T, K> node_point;
	uint64_t left = -1;
	uint64_t right = -1;

	kd_tree_node(point<T, K> pointi);
	kd_tree_node(const std::array<T, K>& coordsi, T valuei);
};

template<typename T, size_t K>
kd_tree_node<T, K>::kd_tree_node(point<T, K> pointi){
	node_point = std::move(pointi);
}

template<typename T, size_t K>
kd_tree_node<T, K>::kd_tree_node(const std::array<T, K>& coordsi, T valuei){
	node_point = point(coordsi, valuei);
}

template<typename T, size_t K>
struct answer_nearest{
	uint64_t dims = K;
	uint64_t N; 

	T* coords;
	T* values;

	answer_nearest(uint64_t Ni, uint64_t dimsi){
		dims = dimsi;
		N = Ni;

		values = new T[Ni];

		coords = (T*)(new char[Ni*K*sizeof(T) + 32 - (Ni*K*sizeof(T))%32]);  // K horizontal, Ni vertical, make it 256-bit aligned for possible AVX2 processing

		memset((void*)coords, 0, Ni*K*sizeof(T) + 32 - (Ni*K*sizeof(T))%32);
	}

	~answer_nearest(){
		delete [] values;
		delete [] coords;
	}
}__attribute__((packed));

template<typename T, size_t K>
struct my_cmp{
	constexpr bool operator()(std::pair<T, point<T, K>> const& A, std::pair<T, point<T, K>> const& B) const noexcept{
		return A.first < B.first;
	}
};

template<typename T, size_t K>
struct kd_tree{
private:
	std::vector<kd_tree_node<T, K>> nodes;

	inline uint64_t insert_recursive(uint64_t curnode, const std::array<T, K>& coordsi, T valuei, uint64_t depth);
	inline uint64_t get_index_recursive(uint64_t curnode, const std::array<T, K>& coordsi, uint64_t depth) const;
	void nearest_neighbour_recursive(uint64_t curnode, const std::array<T, K>& coordsi, std::priority_queue<std::pair<T, point<T, K>>, std::vector<std::pair<T, point<T, K>>>, my_cmp<T, K>>& pqueue, uint64_t depth, uint64_t n) const;		

public:
	kd_tree_node<T, K>& operator[](uint64_t index) const;

	uint64_t insert(const std::array<T, K>& coordsi, T valuei);

	uint64_t get_index(const std::array<T, K>& coordsi) const;

	void print_recursive(uint64_t curnode, uint64_t depth) const;

	answer_nearest<T, K>* find_n_nearest(std::array<T, K>& coordsi, uint64_t n);
};

template<typename T, size_t K>
kd_tree_node<T, K>& kd_tree<T, K>::operator[](uint64_t index) const{
	return nodes[index];
}

template<typename T, size_t K>
inline uint64_t kd_tree<T, K>::insert_recursive(uint64_t curnode, const std::array<T, K>& coordsi, T valuei, uint64_t depth){
	if(!nodes.size()){
		nodes.push_back(kd_tree_node(point(coordsi, valuei)));
		return 0;
	}

	if(curnode == -1){
		nodes.push_back(kd_tree_node(point(coordsi, valuei)));
		return nodes.size()-1;
	}

	uint64_t cd = depth % K;

	if(coordsi[cd] < nodes[curnode].node_point.coords[cd]){
		uint64_t tmp = insert_recursive(nodes[curnode].left, coordsi, valuei, depth+1);
		if(nodes[curnode].left == -1){
			nodes[curnode].left = tmp;
		}
		return nodes[curnode].left;
	}else{
		uint64_t tmp = insert_recursive(nodes[curnode].right, coordsi, valuei, depth+1);
		if(nodes[curnode].right == -1){
			nodes[curnode].right = tmp;
		}
		return nodes[curnode].right;
	}
}

template<typename T, size_t K>
uint64_t kd_tree<T, K>::insert(const std::array<T, K>& coordsi, T valuei){
	return insert_recursive(0, coordsi, valuei, 0);
}

template<typename T, size_t K>
inline uint64_t kd_tree<T, K>::get_index_recursive(uint64_t curnode, const std::array<T, K>& coordsi, uint64_t depth) const{
	if(!nodes.size() or curnode == -1){return -1;}

	if(nodes[curnode].node_point.coords == coordsi){return curnode;}

	uint64_t cd = depth % K;

	if(coordsi[cd] < nodes[curnode].node_point.coords[cd]){
		return get_index_recursive(nodes[curnode].left, coordsi, depth+1);
	}else{
		return get_index_recursive(nodes[curnode].right, coordsi, depth+1);
	}
}

template<typename T, size_t K>
uint64_t kd_tree<T, K>::get_index(const std::array<T, K>& coordsi) const{
	return get_index_recursive(0, coordsi, 0);
}


template<typename T, size_t K>
void kd_tree<T, K>::print_recursive(uint64_t curnode, uint64_t depth) const{
	if(!nodes.size()){
		printf("Empty tree\n");
		return;
	}

	if(curnode == -1){return;}

	for(uint64_t i = 0; i < depth; i++){printf("  ");}
	putchar('(');
	for(size_t i = 0; i < K; i++){
		std::cout << nodes[curnode].node_point.coords[i];
		printf(", ");
		if(i == K-1){
			printf("VAL:");
			std::cout << nodes[curnode].node_point.value;
		}
	}
	printf(")\n");

	print_recursive(nodes[curnode].left, depth+1);
	print_recursive(nodes[curnode].right, depth+1);
}


template<typename T, size_t K>
answer_nearest<T, K>* kd_tree<T, K>::find_n_nearest(std::array<T, K>& coordsi, uint64_t n){
	if(nodes.size() < n){return nullptr;}

	std::priority_queue<std::pair<T, point<T, K>>, std::vector<std::pair<T, point<T, K>>>, my_cmp<T, K> > bpq;
	nearest_neighbour_recursive(0, coordsi, bpq, 0, n);

	answer_nearest<T, K>* result = new answer_nearest<T, K>(bpq.size(), K);
	uint64_t j = 0;
	for(; !bpq.empty(); bpq.pop()){
		result->values[j] = bpq.top().second.value;
		for(uint64_t k = 0; k < K; k++){
			result->coords[j*K+k] = bpq.top().second.coords[k];
		}
		j++;		
	}
	return result;
}

template<typename T, size_t K>
void kd_tree<T, K>::nearest_neighbour_recursive(uint64_t curnode, const std::array<T, K>& coordsi, std::priority_queue<std::pair<T, point<T, K>>, std::vector<std::pair<T, point<T, K>>>, my_cmp<T, K> >& pqueue, uint64_t depth, uint64_t n) const{
	if(curnode == -1){return;}

	pqueue.push(std::make_pair(distance(nodes[curnode].node_point, coordsi), nodes[curnode].node_point));
	if(pqueue.size() > n){
		pqueue.pop();
	}

	bool is_left = false;
	uint64_t cd = depth % K;
	if(coordsi[cd] < nodes[curnode].node_point.coords[cd]){
		nearest_neighbour_recursive(nodes[curnode].left, coordsi, pqueue, depth+1, n);
		is_left = true;
	}else{
		nearest_neighbour_recursive(nodes[curnode].right, coordsi, pqueue, depth+1, n);
	}

	if(pqueue.size() < n or fabs(coordsi[cd] - nodes[curnode].node_point.coords[cd]) < pqueue.top().second.coords[cd]){
		if(is_left){
			nearest_neighbour_recursive(nodes[curnode].right, coordsi, pqueue, depth+1, n);
		}else{
			nearest_neighbour_recursive(nodes[curnode].left, coordsi, pqueue, depth+1, n);
		}
	}
}


#endif
