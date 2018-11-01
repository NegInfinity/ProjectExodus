#pragma once

#include "JsonTypes.h"
#include "DataPlane2D.h"

using FloatPlane2D = DataPlane2D<float>;
namespace JsonTerrainTools{
	//uses linear interpolation internally. As this is how unity does it.
	bool rescaleSplatMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui = false);

	//uses linear interpolation internally
	bool rescaleHeightMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui = false);

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
			catmullRomTangent(p2, p0, 2.0f), 
			p2, 
			catmullRomTangent(p3, p1, 2.0f), 
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
}
