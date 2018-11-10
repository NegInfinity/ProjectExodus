#include "JsonImportPrivatePCH.h"
#include "MaterialExpressionBuilder.h"
#include "MaterialBuilder.h"

auto makeMaterialFactoryNew(){
	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();
	return TSharedPtr<UMaterialFactoryNew>(
		matFactory, 
		[](UMaterialFactoryNew* p){
			if (p) 
				p->RemoveFromRoot();
		}
	);
}

void MaterialBuilder::createBillboardTransformNodes(UMaterial *material){
	MaterialExpressionBuilder b(material);

	auto xAxis = b.constVec3(1.0f, 0.0f, 0.0f, TEXT("xAxis"));
	auto yAxis = b.constVec3(0.0f, 1.0f, 0.0f, TEXT("yAxis"));
	auto zAxis = b.constVec3(0.0f, 0.0f, 1.0f, TEXT("zAxis"));
	auto zero = b.constVec3(0.0f, 0.0f, 0.0f, TEXT("zero"));

	auto worldPos = b.worldPos();

	auto locPos = b.transformPos(worldPos, TRANSFORMPOSSOURCE_World, TRANSFORMPOSSOURCE_Local, TEXT("locPos"));

	auto worldOrigin = b.transformPos(zero, TRANSFORMPOSSOURCE_Local, TRANSFORMPOSSOURCE_World, TEXT("worldOrigin"));
	auto xScale = b.distance(b.transformPos(xAxis, TRANSFORMPOSSOURCE_Local, TRANSFORMPOSSOURCE_World), worldOrigin, TEXT("xScale"));
	auto yScale = b.distance(b.transformPos(yAxis, TRANSFORMPOSSOURCE_Local, TRANSFORMPOSSOURCE_World), worldOrigin, TEXT("yScale"));
	auto zScale = b.distance(b.transformPos(zAxis, TRANSFORMPOSSOURCE_Local, TRANSFORMPOSSOURCE_World), worldOrigin, TEXT("zScale"));

	auto locX = b.mask(locPos, true, false, false, false, TEXT("locX"));
	auto locY = b.mask(locPos, false, true, false, false, TEXT("locY"));
	auto locZ = b.mask(locPos, false, false, true, false, TEXT("locZ"));

	auto scaledX = b.mul(xScale, locX);
	auto scaledY = b.mul(yScale, locY);
	auto scaledZ = b.mul(zScale, locZ);

	auto camX = b.transform(xAxis, TRANSFORMSOURCE_Camera, TRANSFORM_World, TEXT("camX"));
	auto camY = b.transform(yAxis, TRANSFORMSOURCE_Camera, TRANSFORM_World, TEXT("camY"));
	auto camZ = b.transform(zAxis, TRANSFORMSOURCE_Camera, TRANSFORM_World, TEXT("camZ"));

	auto adjustedPos = b.add(worldOrigin,
		b.add(
			b.add(
				b.mul(camX, scaledY), 
				b.mul(camY, scaledZ)
			),
			b.mul(camZ, scaledX)
		), TEXT("Adjusted pos")
	);

	auto worldOffset = b.sub(adjustedPos, worldPos);

	material->WorldPositionOffset.Expression = worldOffset;
	//auto xScale = 
}

void MaterialBuilder::fillBillboardMaterial(UMaterial* material, const JsonTerrainDetailPrototype * detailPrototype, 
		int layerIndex, const TerrainBuilder *terrainBuilder){

	check(material);
	check(detailPrototype);
	check(terrainBuilder);

	material->BlendMode = BLEND_Masked;
	auto &opacityTarget = material->OpacityMask;

	auto importer = terrainBuilder->getImporter();
	auto texId = detailPrototype->textureId;
	auto tex = importer->getTexture(texId);

	MaterialExpressionBuilder b(material);


	auto mul = b.mul(b.texSample(tex, nullptr), 
		b.constVec4(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), TEXT("Tint"))
	);

	auto alpha = b.mask(mul, false, false, false ,true); 

	//detailPrototype->healthyColor;
	//detailPrototype->dryColor;

	material->BaseColor.Expression = mul;
	material->OpacityMask.Expression = alpha;

	material->BaseColor.Expression = mul;
	material->OpacityMask.Expression = alpha;

	material->bUsedWithInstancedStaticMeshes = true;

	if (detailPrototype->isBillboard()){
		UE_LOG(JsonLog, Warning, TEXT("Billboard material found on terrain data \"%s\" (%s), detail layer %d. Billboards are not currently supported"),
			*terrainBuilder->terrainData.name, *terrainBuilder->terrainData.exportPath, layerIndex);
		//createBillboardTransformNodes(material);
	}
	//if (detailPrototype->billboard){
	//}

	arrangeNodesTree(material);
}

UMaterial* MaterialBuilder::createBillboardMaterial(const JsonTerrainDetailPrototype * detailPrototype, 
		int layerIndex, const TerrainBuilder *terrainBuilder, const FString &terrainDataPath){
	check(detailPrototype);
	check(terrainBuilder);

	auto baseName = terrainBuilder->terrainData.getGrassLayerName(layerIndex) + TEXT("_Material");

	auto matFactory = makeMaterialFactoryNew();
	auto mat = createAssetObject<UMaterial>(baseName, &terrainDataPath, terrainBuilder->getImporter(), 
		[&](UMaterial* mat){
			fillBillboardMaterial(mat, detailPrototype, layerIndex, terrainBuilder);
			mat->PreEditChange(0);
			mat->PostEditChange();
		},
		[&](UPackage* pkg) -> UMaterial*{
			return Cast<UMaterial>(
				matFactory->FactoryCreateNew(
					UMaterial::StaticClass(), 
					pkg, 
					*sanitizeObjectName(baseName), 
					RF_Standalone|RF_Public,
					0, GWarn
				)
			);
		}, RF_Standalone|RF_Public
	);

	return mat;
}

