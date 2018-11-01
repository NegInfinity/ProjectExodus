#include "JsonImportPrivatePCH.h"
#include "terrainTools.h"
#include "ScopedSlowTask.h"

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

	const FVector2D srcPixelSize(1.0f/(float)src.getWidth(), 1.0f/(float)src.getHeight());
	const FVector2D dstPixelSize(1.0f/(float)dst.getWidth(), 1.0f/(float)dst.getHeight());

	const FVector2D srcPixelScale = FVector2D(1.0f, 1.0f)/srcPixelSize;
	const FVector2D dstPixelScale = FVector2D(1.0f, 1.0f)/dstPixelSize;

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
		auto srcY = splatToPixCoord(dstV, srcPixelSize.Y);

		auto srcYBase = FMath::FloorToInt(srcY);
		auto srcY0 = FMath::Clamp(srcYBase, minSrcY, maxSrcY);
		auto srcY1 = FMath::Clamp(srcYBase + 1, minSrcY, maxSrcY);
		auto yLerp = FMath::Frac(srcY);

		auto srcRowY0 = src.getRow(srcY0);
		auto srcRowY1 = src.getRow(srcY1);

		for(int32 dstX = 0; dstX < dst.getWidth(); dstX++){
			auto dstU = intToSplatCoord(dstX, dstPixelSize.X);
			auto srcX = splatToPixCoord(dstU, srcPixelSize.X);

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

//uses linear interpolation internally
bool JsonTerrainTools::rescaleHeightMap(FloatPlane2D &dst, const FloatPlane2D &src, bool gui){
	if (dst.isEmpty() || src.isEmpty())
		return false;

	const FVector2D srcPixelSize(1.0f/(float)src.getWidth(), 1.0f/(float)src.getHeight());
	const FVector2D dstPixelSize(1.0f/(float)dst.getWidth(), 1.0f/(float)dst.getHeight());

	const FVector2D srcPixelScale = FVector2D(1.0f, 1.0f)/srcPixelSize;
	const FVector2D dstPixelScale = FVector2D(1.0f, 1.0f)/dstPixelSize;

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
		auto srcY = splatToPixCoord(dstV, srcPixelSize.Y);

		auto srcYBase = FMath::FloorToInt(srcY);
		auto yLerp = FMath::Frac(srcY);

		auto srcY0 = FMath::Clamp(srcYBase - 1, minSrcY, maxSrcY);
		auto srcY1 = FMath::Clamp(srcYBase, minSrcY, maxSrcY);
		auto srcY2 = FMath::Clamp(srcYBase + 1, minSrcY, maxSrcY);
		auto srcY3 = FMath::Clamp(srcYBase + 2, minSrcY, maxSrcY);

		auto srcRowY0 = src.getRow(srcY0);
		auto srcRowY1 = src.getRow(srcY1);
		auto srcRowY2 = src.getRow(srcY2);
		auto srcRowY3 = src.getRow(srcY3);

		for(int32 dstX = 0; dstX < dst.getWidth(); dstX++){
			auto dstU = intToSplatCoord(dstX, dstPixelSize.X);
			auto srcX = splatToPixCoord(dstU, srcPixelSize.X);

			auto srcXBase = FMath::FloorToInt(srcX);
			auto xLerp = FMath::Frac(srcX);

			auto srcX0 = FMath::Clamp(srcXBase - 1, minSrcX, maxSrcX);
			auto srcX1 = FMath::Clamp(srcXBase, minSrcX, maxSrcX);
			auto srcX2 = FMath::Clamp(srcXBase + 2, minSrcX, maxSrcX);
			auto srcX3 = FMath::Clamp(srcXBase + 3, minSrcX, maxSrcX);

			auto vy0 = interpolateCatmullRom(srcRowY0, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy1 = interpolateCatmullRom(srcRowY1, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy2 = interpolateCatmullRom(srcRowY2, srcX0, srcX1, srcX2, srcX3, xLerp);
			auto vy3 = interpolateCatmullRom(srcRowY3, srcX0, srcX1, srcX2, srcX3, xLerp);

			auto val = interpolateCatmullRom(
				vy0, vy1, vy2, vy3, yLerp
			);

			dstRowData[dstX] = val;//1.0f;
		}

		if (gui){
			guiTask->EnterProgressFrame(1.0f);
		}
	}

	return true;
}
