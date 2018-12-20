#include "JsonImportPrivatePCH.h"
#include "MeshBuilderUtils.h"
#include "UnrealUtilities.h"

using namespace UnrealUtilities;

void MeshBuilderUtils::processTangent(int originalIndex, const FloatArray &normFloats, const FloatArray &tangentFloats, bool hasNormals, bool hasTangents,
		std::function<void(const FVector&)> normCallback, std::function<void(const FVector&, const FVector&)> tanCallback){
	if (!hasNormals)
		return;
	auto unityNorm = getIdxVector3(normFloats, originalIndex);
	auto normUnreal = unityVecToUe(unityNorm);
	if (normCallback)
		normCallback(normUnreal);
	//newRawMesh.WedgeTangentZ.Add(normUnreal);
	if (!hasTangents)
		return;

	auto unityTangent = getIdxVector4(tangentFloats, originalIndex);
	auto uTanUnity = FVector(unityTangent.X, unityTangent.Y, unityTangent.Z);

	auto uTanUnreal = unityVecToUe(uTanUnity);
	//auto vTanUnreal = FVector::CrossProduct(uTanUnreal, normUnreal) * unityTangent.W;
	auto vTanUnreal = FVector::CrossProduct(normUnreal, uTanUnreal) * unityTangent.W;
	/* 
		I suspect unity gets normals wrong on at least SOME geometry, but can't really prove it.
	*/
	uTanUnreal.Normalize();
	vTanUnreal.Normalize();

	if (tanCallback)
		tanCallback(uTanUnreal, vTanUnreal);

	//newRawMesh.WedgeTangentX.Add(uTanUnreal);
	//newRawMesh.WedgeTangentY.Add(vTanUnreal);
}

