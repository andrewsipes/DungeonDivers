#pragma once
#include "./load_object_oriented.h"
class uiPanel {

	std::vector <Model> objects; //holds the models in the panel;
	bool render;				//turns the panel on or off


public:

	uiPanel() {

		render = false;

	}

	//scales a model's vertices
	void scaleObject(Model &object, float scale) {

		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			object.cpuModel.vertices[i].pos.x *= scale;
			object.cpuModel.vertices[i].pos.z *= scale;
			object.cpuModel.vertices[i].pos.y *= scale;

		}
	}

	//translate's a model
	void translateObject(Model &object, GW::MATH::GVECTORF translate) {

		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			object.cpuModel.vertices[i].pos.x += translate.x;
			object.cpuModel.vertices[i].pos.z += translate.y;
			object.cpuModel.vertices[i].pos.y += translate.z;

		}
	}

	//rotates around the y
	void rotateObjectYAxis(Model& object, float degrees) {
		float cosTheta = cos(toRad(degrees));
		float sinTheta = sin(toRad(degrees));
		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			float x = object.cpuModel.vertices[i].pos.x;
			float z = object.cpuModel.vertices[i].pos.z;
			object.cpuModel.vertices[i].pos.x = x * cosTheta - z * sinTheta;
			object.cpuModel.vertices[i].pos.z = x * sinTheta + z * cosTheta;
		}
	}

	//rotates around the x
	void rotateObjectXAxis(Model& object, float degrees) {
		float cosTheta = cos(toRad(degrees));
		float sinTheta = sin(toRad(degrees));
		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			float y = object.cpuModel.vertices[i].pos.y;
			float z = object.cpuModel.vertices[i].pos.z;
			object.cpuModel.vertices[i].pos.y = y * cosTheta - z * sinTheta;
			object.cpuModel.vertices[i].pos.z = y * sinTheta + z * cosTheta;
		}
	}
};
