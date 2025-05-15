#include <iostream>
#include <fstream>
#include <stdfloat>
#include <string>

#include "kd_tree.h"
#include "idw.h"
#include "json.hpp"
#include "settings.h"


std::string path_to_json = "config.json";
std::string path_to_target_points;
std::string path_to_basis_points;
std::string path_to_result_points;
uint64_t interpolate_by = interpolation_points;

int main(int argc, char** argv){
	if(argc == 1){
	}else if(argc == 2){
		path_to_json = argv[1];
	}else if(argc == 3){
		path_to_json = argv[1];
		interpolate_by = atoll(argv[2]);
	}else{
		printf("Invalid arguments!\n");
		exit(0);
	}


	std::ifstream json_ifstream(path_to_json);
	nlohmann::json_abi_v3_12_0::json data = nlohmann::json_abi_v3_12_0::json::parse(json_ifstream);
	json_ifstream.close();

	path_to_target_points = data["targetPoints"];
	path_to_basis_points = data["basisPoints"];
	path_to_result_points = data["resultedPoints"];


	kd_tree<numbertype, dimensions> tree;


	std::ifstream basis_points_file(path_to_basis_points);
	std::string str;
	uint64_t dim_cnt = 0;
	std::array<numbertype, dimensions> coordinates;
	numbertype value;
	while(basis_points_file >> str){
		if(dim_cnt < dimensions){
			std::stringstream(str) >> coordinates[dim_cnt];
			dim_cnt++;
		}else{
			std::stringstream(str) >> value;
			tree.insert(coordinates, value);
			dim_cnt = 0;
		}
	}
	basis_points_file.close();
	tree.print_recursive(0, 0);

	std::ifstream target_points_file(path_to_target_points);
	dim_cnt = 0;
	std::vector<std::array<numbertype, dimensions>> target_points;
	while(target_points_file >> str){		
		std::stringstream(str) >> coordinates[dim_cnt];
		dim_cnt++;
		if(dim_cnt == dimensions){
			target_points.push_back(coordinates);
			dim_cnt = 0;
		}
	}
	target_points_file.close();

	std::vector<numbertype> results;

	for(uint64_t i = 0; i < target_points.size(); i++){
		answer_nearest<numbertype, dimensions>* answ = tree.find_n_nearest(target_points[i], interpolate_by);

		numbertype resulted_value = idw(answ, target_points[i].data());
		std::cout << "for point #" << i << ", coordinates: (";
		for(uint64_t j = 0; j < dimensions; j++){
			std::cout << target_points[i][j];
			if(j != dimensions-1){
				std::cout << ";";
			}
		}
		std::cout << "), value is: " << resulted_value << std::endl;
		results.push_back(resulted_value);
		delete answ;
	}

	std::ofstream result_points_file(path_to_result_points);
	for(uint64_t i = 0; i < target_points.size(); i++){
		for(uint64_t j = 0; j < dimensions; j++){
			result_points_file << target_points[i][j] << " ";
		}
		result_points_file << results[i] << std::endl;
	}
	result_points_file.close();


	return 0;
}
