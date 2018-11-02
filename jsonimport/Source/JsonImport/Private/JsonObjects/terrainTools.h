#pragma once

#include "JsonTypes.h"
#include "DataPlane2D.h"

using FloatPlane2D = DataPlane2D<float>;
namespace JsonTerrainTools{
	//For remapping unity terrain masks onto unreal data layout
	bool scaleSplatMapToHeightMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui = false);

	//uses linear interpolation internally. As this is how unity does it.
	bool rescaleSplatMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui = false);

	//uses linear interpolation internally
	bool rescaleHeightMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui = false);

	//This is broken.

	//https://en.wikipedia.org/wiki/Cubic_Hermite_spline
	inline constexpr float interpolateHermite(float p0, float p1, float m0, float m1, float t){
		/*
		//Bah. C++11 constexpr, so no vars for me.
	
		const auto t3 = t*t*t;
		const auto t2 = t*t;
		const auto h00 = 2.0f*t3 - 3.0f*t2 + 1.0f;
		const auto h10 = t3 - 2.0f*t2 + t;
		const auto h01 = -2.0f*t3 + 3.0f*t2;
		const auto h11 = t3 - t2;
		return h00*p0 + h10*m0 + h01*p1 + h11*m1;
		*/
		return (2.0f*t*t*t - 3.0f*t*t + 1.0f)*p0  //h00
			+ (t*t*t - 2.0f*t*t + t)*m0 //h10
			+ (-2.0f*t*t*t + 3.0f*t*t)*p1 //h01
			+ (t*t*t - t*t)*m1; //h11
	}


	inline constexpr float catmullRomTangent(float pPrev, float pNext, float tPrev, float tNext){
		return (pNext - pPrev)/(tNext - tPrev);
	}

	inline constexpr float catmullRomTangent(float pPrev, float pNext, float tDiff){
		return (pNext - pPrev)/tDiff;
	}
	/*
	interpolates between p1 and p2 using t
	*/
	inline constexpr float interpolateCatmullRom(float p0, float p1, float p2, float p3, float t){
		/*
		const float m1 = catmullRomTangent(p2, p0, 2.0f);
		const float m2 = catmullRomTangent(p3, p1, 2.0f);
		return interpolateHermite(p1, m1, p2, m2, t);
		*/
		return interpolateHermite(
			p1, 
			p2, 
			catmullRomTangent(p0, p2, 2.0f), 
			catmullRomTangent(p1, p3, 2.0f), 
			t
		);
	}

	inline constexpr float interpolateCatmullRom(const float* data, int idx0, int idx1, int idx2, int idx3, float t){
		return interpolateCatmullRom(
			data[idx0],
			data[idx1],
			data[idx2],
			data[idx3],
			t
		);
	}

	/*
	https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline
	This is a simplification of:
		a1 = p0 * (t1 - t)/(t1 - t0) + p1 * (t - t0)/(t1 - t0);
		a2 = p1 * (t2 - t)/(t2 - t1) + p2 * (t - t1)/(t2 - t1);
		a3 = p2 * (t3 - t)/(t3 - t2) + p3 * (t - t2)/(t3 - t2);
		b1 = a1 * (t2 - t)/(t2 - t0) + a2 * (t - t0)/(t2 - t0);
		b2 = a2 * (t3 - t)/(t3 - t1) + a3 * (t - t1)/(t3 - t1);
		c = b1 * (t2 - t)/(t2 - t1) + b2 * (t - t1)/(t2 - t1);
	for evenly spaced t, where t1 = 0.0f, t0 = -1.0f, t2 = 1.0f, t3 = 2.0f, etc.
	*/
	inline constexpr float interpolateCatmullRomUniform(float p0, float p1, float p2, float p3, float t){
		return 0.5f * (
			(2.0f * p1) +
			(-p0 + p2) * t +
			(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
			(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t
		);
	}

	inline constexpr float interpolateCatmullRomUniform(const float* data, int32 idx0, int32 idx1, int32 idx2, int32 idx3, float t){
		return interpolateCatmullRom(
			data[idx0],
			data[idx1],
			data[idx2],
			data[idx3],
			t
		);
	}
}
