#pragma once
#include "JsonTypes.h"
#include <functional>

namespace MeshBuilderUtils{
	void processTangent(int originalIndex, const FloatArray &normFloats, const FloatArray &tangentFloats, bool hasNormals, bool hasTangents,
		std::function<void(const FVector&)> normCallback, //Receives normal
		std::function<void(const FVector&, const FVector&)> tanCallback //Receives U and V tangents. U, V. In this order.
	);
}

