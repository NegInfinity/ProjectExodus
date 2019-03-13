using UnityEditor;
using UnityEngine;

namespace SceneExport{
	[System.Serializable]
	public class JsonTerrain: IFastJsonValue{
		public bool castShadows;
		public float detailObjectDensity;
		public float detailObjectDistance;
		public bool drawHeightmap;
		public bool drawTreesAndFoliage;
		
		public bool renderHeightmap;
		public bool renderTrees;
		public bool renderDetails;
		
		public float heightmapPixelError;
		
		public float legacyShininess;
		public Color legacySpecular;
		public int lightmapIndex;
		public Vector4 lightmapScaleOffet;

		public ResId materialTemplateIndex = ResId.invalid;
		public string materialType;		
		
		public Vector3 patchBoundsMultiplier;
		public bool preserveTreePrototypeLayers;
		public int realtimeLightmapIndex;
		public Vector4 realtimeLightmapScaleOffset;
		
		public ResId terrainDataId = ResId.invalid;
		
		public float treeBillboardDistance;
		public float treeCrossFadeLength;
		
		public float treeDistance;
		public float treeLodBiasMultiplier;
		
		public int treeMaximumFullLODCount;
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			
			writer.writeKeyVal("castShadows", castShadows);
			writer.writeKeyVal("detailObjectDensity", detailObjectDensity);
			writer.writeKeyVal("detailObjectDistance", detailObjectDistance);
			writer.writeKeyVal("drawHeightmap", drawHeightmap);
			writer.writeKeyVal("drawTreesAndFoliage", drawTreesAndFoliage);
		
			writer.writeKeyVal("renderHeightmap", renderHeightmap);
			writer.writeKeyVal("renderTrees", renderTrees);
			writer.writeKeyVal("renderDetails", renderDetails);
		
			writer.writeKeyVal("heightmapPixelError", heightmapPixelError);
		
			writer.writeKeyVal("legacyShininess", legacyShininess);
			writer.writeKeyVal("legacySpecular", legacySpecular);
			writer.writeKeyVal("lightmapIndex", lightmapIndex);
			writer.writeKeyVal("lightmapScaleOffet", lightmapScaleOffet);

			writer.writeKeyVal("materialTemplateIndex", materialTemplateIndex);
			writer.writeKeyVal("materialType", materialType);
		
			writer.writeKeyVal("patchBoundsMultiplier", patchBoundsMultiplier);
			writer.writeKeyVal("preserveTreePrototypeLayers", preserveTreePrototypeLayers);
			writer.writeKeyVal("realtimeLightmapIndex", realtimeLightmapIndex);
			writer.writeKeyVal("realtimeLightmapScaleOffset", realtimeLightmapScaleOffset);
		
			writer.writeKeyVal("terrainDataId", terrainDataId);
		
			writer.writeKeyVal("treeBillboardDistance", treeBillboardDistance);
			writer.writeKeyVal("treeCrossFadeLength", treeCrossFadeLength);
		
			writer.writeKeyVal("treeDistance", treeDistance);
			writer.writeKeyVal("treeLodBiasMultiplier", treeLodBiasMultiplier);
		
			writer.writeKeyVal("treeMaximumFullLODCount", treeMaximumFullLODCount);
			
			writer.endObject();
		}
		
		public static void registerLinkedData(Terrain obj, ResourceMapper resMap){
			if (!obj)
				return;
			if (obj.materialType == Terrain.MaterialType.Custom){
				resMap.getMaterialId(obj.materialTemplate);
			}
			resMap.getTerrainId(obj.terrainData);
		}
		
		public JsonTerrain(Terrain srcObj, ResourceMapper resMap){
			if (!srcObj)
				throw new System.ArgumentNullException("srcObj");
				
			castShadows = srcObj.castShadows;
			detailObjectDensity = srcObj.detailObjectDensity;
			detailObjectDistance = srcObj.detailObjectDistance;
			drawHeightmap = srcObj.drawHeightmap;
			drawTreesAndFoliage = srcObj.drawTreesAndFoliage;
		
			renderHeightmap = (srcObj.editorRenderFlags & TerrainRenderFlags.Heightmap) != 0;
			renderTrees = (srcObj.editorRenderFlags & TerrainRenderFlags.Trees) != 0;
			renderDetails = (srcObj.editorRenderFlags & TerrainRenderFlags.Details) != 0;
		
			heightmapPixelError = srcObj.heightmapPixelError;
		
			legacyShininess = srcObj.legacyShininess;
			legacySpecular = srcObj.legacySpecular;
			lightmapIndex = srcObj.lightmapIndex;
			lightmapScaleOffet = srcObj.lightmapScaleOffset;

			materialType = srcObj.materialType.ToString();
		
			patchBoundsMultiplier = srcObj.patchBoundsMultiplier;
			preserveTreePrototypeLayers = srcObj.preserveTreePrototypeLayers;
			realtimeLightmapIndex = srcObj.realtimeLightmapIndex;
			realtimeLightmapScaleOffset = srcObj.realtimeLightmapScaleOffset;
		
			treeBillboardDistance = srcObj.treeBillboardDistance;
			treeCrossFadeLength = srcObj.treeCrossFadeLength;
		
			treeDistance = srcObj.treeDistance;
			treeLodBiasMultiplier = srcObj.treeLODBiasMultiplier;
		
			treeMaximumFullLODCount = srcObj.treeMaximumFullLODCount;
			
			//And once again I doubt whehter the approahc of making a temporary object is justified.
			materialTemplateIndex = ResId.invalid;
			if (srcObj.materialType == Terrain.MaterialType.Custom)
				materialTemplateIndex = resMap.getMaterialId(srcObj.materialTemplate);//-1;
				//material keeps picking up splat texutre :-\
			terrainDataId = resMap.getTerrainId(srcObj.terrainData);
			//public int terrainDataId = -1;
		}
	};
}
