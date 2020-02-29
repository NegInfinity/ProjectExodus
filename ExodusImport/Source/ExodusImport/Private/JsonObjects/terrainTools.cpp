#include "JsonImportPrivatePCH.h"
#include "terrainTools.h"
#include "Misc/ScopedSlowTask.h"

using namespace JsonTerrainTools;

constexpr float intToSplatCoord(int x, float pixelSize){
	return ((float)x + 0.5f) * pixelSize;
}

constexpr float intToVertCoord(int x, float stripSize){
	return ((float)x) * stripSize;
}

FVector2D intToSplatCoord(const FIntPoint xy, const FVector2D &pixelSize){
	return FVector2D(
		intToSplatCoord(xy.X, pixelSize.X),
		intToSplatCoord(xy.Y, pixelSize.Y)
	);
}

FVector2D intToVertCoord(const FIntPoint &xy, const FVector2D &stripSize){
	return FVector2D(
		intToVertCoord(xy.X, stripSize.X),
		intToVertCoord(xy.Y, stripSize.Y)
	);
}

constexpr float splatToPixCoord(float x, float pixelScale){
	return x * pixelScale - 0.5f;
}

constexpr float vertToPixCoord(float x, float pixelScale){
	return x * pixelScale;
}

FVector2D splatToPixCoord(const FVector2D &xy, const FVector2D &pixelScale){
	return FVector2D(
		splatToPixCoord(xy.X, pixelScale.X),
		splatToPixCoord(xy.Y, pixelScale.Y)
	);
}

FVector2D vertToPixCoord(const FVector2D &xy, const FVector2D &pixelScale){
	return FVector2D(
		vertToPixCoord(xy.X, pixelScale.X),
		vertToPixCoord(xy.Y, pixelScale.Y)
	);
}

//uses linear interpolation internally. As this is how unity does it.
bool JsonTerrainTools::rescaleSplatMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui){
	if (dst.isEmpty() || src.isEmpty())
		return false;

	const FVector2D srcPixelScale = FVector2D((float)src.getWidth(), (float)src.getHeight());
	const FVector2D dstPixelScale = FVector2D((float)dst.getWidth(), (float)dst.getHeight());

	const FVector2D srcPixelSize = FVector2D(1.0f, 1.0f)/srcPixelScale;
	const FVector2D dstPixelSize = FVector2D(1.0f, 1.0f)/dstPixelScale;

	const int32 minSrcY = 0;
	const int32 maxSrcY = src.getHeight() - 1;
	const int32 minSrcX = 0;
	const int32 maxSrcX = src.getWidth() - 1;

	TUniquePtr<FScopedSlowTask> guiTask;
	if (gui){
		guiTask = MakeUnique<FScopedSlowTask>(dst.getHeight());
	}

	for(int32 dstY = 0; dstY < dst.getHeight(); dstY++){
		float* dstRowData = dst.getRow(dstY);

		auto dstV = intToSplatCoord(dstY, dstPixelSize.Y);
		auto srcY = splatToPixCoord(dstV, srcPixelScale.Y);

		auto srcYBase = FMath::FloorToInt(srcY);
		auto srcY0 = FMath::Clamp(srcYBase, minSrcY, maxSrcY);
		auto srcY1 = FMath::Clamp(srcYBase + 1, minSrcY, maxSrcY);
		auto yLerp = FMath::Frac(srcY);

		auto srcRowY0 = src.getRow(srcY0);
		auto srcRowY1 = src.getRow(srcY1);

		for(int32 dstX = 0; dstX < dst.getWidth(); dstX++){
			auto dstU = intToSplatCoord(dstX, dstPixelSize.X);
			auto srcX = splatToPixCoord(dstU, srcPixelScale.X);

			auto srcXBase = FMath::FloorToInt(srcX);
			auto srcX0 = FMath::Clamp(srcXBase, minSrcX, maxSrcX);
			auto srcX1 = FMath::Clamp(srcXBase + 1, minSrcX, maxSrcX);

			auto xLerp = FMath::Frac(srcX);

			auto val = FMath::Lerp(
				FMath::Lerp(srcRowY0[srcX0], srcRowY0[srcX1], xLerp),
				FMath::Lerp(srcRowY1[srcX0], srcRowY1[srcX1], xLerp),
				yLerp
			);

			dstRowData[dstX] = val;//1.0f;
		}
		if (gui){
			guiTask->EnterProgressFrame(1.0f);
		}
	}

	return true;
}

//uses linear interpolation internally. As this is how unity does it.
bool JsonTerrainTools::scaleSplatMapToHeightMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui){
	if (dst.isEmpty() || src.isEmpty())
		return false;

	const FVector2D srcPixelScale = FVector2D((float)src.getWidth(), (float)src.getHeight());
	const FVector2D dstPixelScale = FVector2D((float)(dst.getWidth() - 1), (float)(dst.getHeight() - 1));

	const FVector2D srcPixelSize = FVector2D(1.0f, 1.0f)/srcPixelScale;
	const FVector2D dstPixelSize = FVector2D(1.0f, 1.0f)/dstPixelScale;

	const int32 minSrcY = 0;
	const int32 maxSrcY = src.getHeight() - 1;
	const int32 minSrcX = 0;
	const int32 maxSrcX = src.getWidth() - 1;

	TUniquePtr<FScopedSlowTask> guiTask;
	if (gui){
		guiTask = MakeUnique<FScopedSlowTask>(dst.getHeight());
	}

	for(int32 dstY = 0; dstY < dst.getHeight(); dstY++){
		float* dstRowData = dst.getRow(dstY);

		//This is extremely error-prone and should be probably moved into DataPlane class
		auto dstV = intToVertCoord(dstY, dstPixelSize.Y);
		auto srcY = splatToPixCoord(dstV, srcPixelScale.Y);

		auto srcYBase = FMath::FloorToInt(srcY);
		auto srcY0 = FMath::Clamp(srcYBase, minSrcY, maxSrcY);
		auto srcY1 = FMath::Clamp(srcYBase + 1, minSrcY, maxSrcY);
		auto yLerp = FMath::Frac(srcY);

		auto srcRowY0 = src.getRow(srcY0);
		auto srcRowY1 = src.getRow(srcY1);

		for(int32 dstX = 0; dstX < dst.getWidth(); dstX++){
			auto dstU = intToVertCoord(dstX, dstPixelSize.X);
			auto srcX = splatToPixCoord(dstU, srcPixelScale.X);

			auto srcXBase = FMath::FloorToInt(srcX);
			auto srcX0 = FMath::Clamp(srcXBase, minSrcX, maxSrcX);
			auto srcX1 = FMath::Clamp(srcXBase + 1, minSrcX, maxSrcX);

			auto xLerp = FMath::Frac(srcX);

			auto val = FMath::Lerp(
				FMath::Lerp(srcRowY0[srcX0], srcRowY0[srcX1], xLerp),
				FMath::Lerp(srcRowY1[srcX0], srcRowY1[srcX1], xLerp),
				yLerp
			);

			dstRowData[dstX] = val;//1.0f;
		}
		if (gui){
			guiTask->EnterProgressFrame(1.0f);
		}
	}

	return true;
}

float filterNearest(float a, float b, float t){
	return (t < 0.5f) ? a: b;
}

float filterLinear(float a, float b, float t){
	return FMath::Lerp(a, b, t);
}

bool JsonTerrainTools::rescaleHeightMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui){
	if (dst.isEmpty() || src.isEmpty())
		return false;

	const FVector2D srcPixelScale = FVector2D((float)(src.getWidth() - 1), (float)(src.getHeight() - 1));
	const FVector2D dstPixelScale = FVector2D((float)(dst.getWidth() - 1), (float)(dst.getHeight() - 1));

	const FVector2D srcPixelSize = FVector2D(1.0f, 1.0f)/srcPixelScale;
	const FVector2D dstPixelSize = FVector2D(1.0f, 1.0f)/dstPixelScale;


	const int32 minSrcY = 0;
	const int32 maxSrcY = src.getHeight() - 1;
	const int32 minSrcX = 0;
	const int32 maxSrcX = src.getWidth() - 1;

	TUniquePtr<FScopedSlowTask> guiTask;
	if (gui){
		guiTask = MakeUnique<FScopedSlowTask>(dst.getHeight());
	}

	for(int32 dstY = 0; dstY < dst.getHeight(); dstY++){
		float* dstRowData = dst.getRow(dstY);

		auto dstV = intToVertCoord(dstY, dstPixelSize.Y);
		auto srcY = vertToPixCoord(dstV, srcPixelScale.Y);

		auto srcYBase = FMath::FloorToInt(srcY);
		auto yLerp = FMath::Frac(srcY);

		auto srcY0 = FMath::Clamp(srcYBase + 0, minSrcY, maxSrcY);
		auto srcY1 = FMath::Clamp(srcYBase + 1, minSrcY, maxSrcY);

		auto srcRowY0 = src.getRow(srcY0);
		auto srcRowY1 = src.getRow(srcY1);

		for(int32 dstX = 0; dstX < dst.getWidth(); dstX++){
			auto dstU = intToVertCoord(dstX, dstPixelSize.X);
			auto srcX = vertToPixCoord(dstU, srcPixelScale.X);

			auto srcXBase = FMath::FloorToInt(srcX);
			auto xLerp = FMath::Clamp(FMath::Frac(srcX), 0.0f, 1.0f);

			auto srcX0 = FMath::Clamp(srcXBase + 0, minSrcX, maxSrcX);
			auto srcX1 = FMath::Clamp(srcXBase + 1, minSrcX, maxSrcX);

			auto vy0 = filterLinear(srcRowY0[srcX0], srcRowY0[srcX1], xLerp);
			auto vy1 = filterLinear(srcRowY1[srcX0], srcRowY1[srcX1], xLerp);
			auto val = filterLinear(vy0, vy1, yLerp);
			//I tried more advanced interpolation, but it, sadly, causes overshoot at ridges.

			dstRowData[dstX] = val;//1.0f;
		}

		if (gui){
			guiTask->EnterProgressFrame(1.0f);
		}
	}

	return true;
}
#if 0
//uses linear interpolation internally
bool JsonTerrainTools::rescaleHeightMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui){
	if (dst.isEmpty() || src.isEmpty())
		return false;

	const FVector2D srcPixelScale = FVector2D((float)(src.getWidth() - 1), (float)(src.getHeight() - 1));
	const FVector2D dstPixelScale = FVector2D((float)(dst.getWidth() - 1), (float)(dst.getHeight() - 1));

	const FVector2D srcPixelSize = FVector2D(1.0f, 1.0f)/srcPixelScale;
	const FVector2D dstPixelSize = FVector2D(1.0f, 1.0f)/dstPixelScale;


	const int32 minSrcY = 0;
	const int32 maxSrcY = src.getHeight() - 1;
	const int32 minSrcX = 0;
	const int32 maxSrcX = src.getWidth() - 1;

	TUniquePtr<FScopedSlowTask> guiTask;
	if (gui){
		guiTask = MakeUnique<FScopedSlowTask>(dst.getHeight());
	}

	for(int32 dstY = 0; dstY < dst.getHeight(); dstY++){
		float* dstRowData = dst.getRow(dstY);

		auto dstV = intToVertCoord(dstY, dstPixelSize.Y);
		auto srcY = vertToPixCoord(dstV, srcPixelScale.Y);

		auto srcYBase = FMath::FloorToInt(srcY);
		auto yLerp = FMath::Frac(srcY);

		auto srcY0 = FMath::Clamp(srcYBase - 1, minSrcY, maxSrcY);
		auto srcY1 = FMath::Clamp(srcYBase + 0, minSrcY, maxSrcY);
		auto srcY2 = FMath::Clamp(srcYBase + 1, minSrcY, maxSrcY);
		auto srcY3 = FMath::Clamp(srcYBase + 2, minSrcY, maxSrcY);

		auto srcRowY0 = src.getRow(srcY0);
		auto srcRowY1 = src.getRow(srcY1);
		auto srcRowY2 = src.getRow(srcY2);
		auto srcRowY3 = src.getRow(srcY3);

		for(int32 dstX = 0; dstX < dst.getWidth(); dstX++){
			auto dstU = intToVertCoord(dstX, dstPixelSize.X);
			auto srcX = vertToPixCoord(dstU, srcPixelScale.X);

			auto srcXBase = FMath::FloorToInt(srcX);
			auto xLerp = FMath::Clamp(FMath::Frac(srcX), 0.0f, 1.0f);

			auto srcX0 = FMath::Clamp(srcXBase - 1, minSrcX, maxSrcX);
			auto srcX1 = FMath::Clamp(srcXBase + 0, minSrcX, maxSrcX);
			auto srcX2 = FMath::Clamp(srcXBase + 1, minSrcX, maxSrcX);
			auto srcX3 = FMath::Clamp(srcXBase + 2, minSrcX, maxSrcX);

			/*
			auto vy0 = interpolateCatmullRom(srcRowY0, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy1 = interpolateCatmullRom(srcRowY1, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy2 = interpolateCatmullRom(srcRowY2, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy3 = interpolateCatmullRom(srcRowY3, srcX0, srcX1, srcX2, srcX3, xLerp);

			auto val = interpolateCatmullRom(
				vy0, vy1, vy2, vy3, yLerp
			);
			*/
			/*
			auto vy1 = filterNearest(srcRowY1[srcX1], srcRowY1[srcX2], xLerp);
			auto vy2 = filterNearest(srcRowY2[srcX1], srcRowY2[srcX2], xLerp);
			auto val = filterNearest(vy1, vy2, yLerp);
			*/

			/*
			auto vy1 = filterLinear(srcRowY1[srcX1], srcRowY1[srcX2], xLerp);
			auto vy2 = filterLinear(srcRowY2[srcX1], srcRowY2[srcX2], xLerp);
			auto val = filterLinear(vy1, vy2, yLerp);
			*/

			auto vy0 = interpolateCatmullRom(srcRowY0, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy1 = interpolateCatmullRom(srcRowY1, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy2 = interpolateCatmullRom(srcRowY2, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy3 = interpolateCatmullRom(srcRowY3, srcX0, srcX1, srcX2, srcX3, xLerp);

			auto val = interpolateCatmullRom(
				vy0, vy1, vy2, vy3, yLerp
			);
			/*
			auto vy0 = interpolateCatmullRomUniform(srcRowY0, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy1 = interpolateCatmullRomUniform(srcRowY1, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy2 = interpolateCatmullRomUniform(srcRowY2, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy3 = interpolateCatmullRomUniform(srcRowY3, srcX0, srcX1, srcX2, srcX3, xLerp);

			auto val = interpolateCatmullRomUniform(
				vy0, vy1, vy2, vy3, yLerp
			);
			*/


			dstRowData[dstX] = val;//1.0f;
		}

		if (gui){
			guiTask->EnterProgressFrame(1.0f);
		}
	}

	return true;
}
#endif