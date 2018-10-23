using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonTerrainDetailPrototype: IFastJsonValue{
		public DetailPrototype data;
		
		public int textureId = -1;
		public int detailPrefabId = -1;
		public int detailPrefabObjectId = -1;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("bendFactor", data.bendFactor);
			writer.writeKeyVal("dryColor", data.dryColor);
			writer.writeKeyVal("minWidth", data.minWidth);
			writer.writeKeyVal("maxWidth", data.maxWidth);
			writer.writeKeyVal("minHeight", data.minHeight);
			writer.writeKeyVal("maxHeight", data.maxHeight);
			writer.writeKeyVal("healthyColor", data.healthyColor);
			writer.writeKeyVal("noiseSpread", data.noiseSpread);
			writer.writeKeyVal("renderMode", data.renderMode.ToString());
			writer.writeKeyVal("usePrototypeMesh", data.usePrototypeMesh);
			
			writer.writeKeyVal("textureId", textureId);
			writer.writeKeyVal("detailPrefabId", detailPrefabId);
			writer.writeKeyVal("detailPrefabObjectId", detailPrefabObjectId);
			
			writer.endObject();
		}		
		
		public JsonTerrainDetailPrototype(DetailPrototype data_, ResourceMapper resMap){		
			data = data_;
			
			textureId = resMap.getTextureId(data.prototypeTexture);
			detailPrefabId = resMap.getRootPrefabId(data.prototype, true);
			detailPrefabObjectId = resMap.getPrefabObjectId(data.prototype, true);
			//meshId = resMap.getPrefabObjectId
		}
	}

	[System.Serializable]
	public class JsonTreeInstance: IFastJsonValue{
		public TreeInstance data;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("color", data.color);
			writer.writeKeyVal("heightScale", data.heightScale);
			writer.writeKeyVal("lightmapColor", data.lightmapColor);
			writer.writeKeyVal("position", data.position);
			writer.writeKeyVal("rotation", data.rotation);//this is an angle and not a quaternion, surprisingly
			writer.writeKeyVal("prototypeIndex", data.prototypeIndex);
			writer.writeKeyVal("widthScale", data.widthScale);			
			writer.endObject();
		}
		
		public JsonTreeInstance(TreeInstance inst){
			data = inst;
		}
	}
	
	[System.Serializable]
	public class JsonTreePrototype: IFastJsonValue{		
		public TreePrototype src;
		public int prefabId = -1;
		public int prefabObjectId = -1;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("prefabId", prefabId);
			writer.writeKeyVal("prefabObjectId", prefabObjectId);
			writer.endObject();
		}
		
		public JsonTreePrototype(TreePrototype src_, ResourceMapper resMap){
			src = src_;
			prefabId = resMap.getRootPrefabId(src.prefab, true);
			prefabObjectId = resMap.getPrefabObjectId(src.prefab, true);
		}
	}
	
	[System.Serializable]
	public class JsonTerrainData: IFastJsonValue{
		public static readonly string terrainAssetExportFolder = "!terrain!";
		
		public TerrainData terrainData = null;
		
		public string name;
		public string path;
		public string exportPath;
		
		public List<int> alphaMapTextureIds = new List<int>();
		public JsonBounds bounds = new JsonBounds();
		
		public List<JsonTerrainDetailPrototype> detailPrototypes = new List<JsonTerrainDetailPrototype>();
		public List<JsonTreeInstance> treeInstances = new List<JsonTreeInstance>();
		public List<JsonTreePrototype> treePrototypes = new List<JsonTreePrototype>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("path", path);
			
			writer.writeKeyVal("alphaMapWidth", terrainData.alphamapWidth);
			writer.writeKeyVal("alphaMapHeight", terrainData.alphamapHeight);
			writer.writeKeyVal("alphaMapLayers", terrainData.alphamapLayers);
			writer.writeKeyVal("alphaMapResolution", terrainData.alphamapResolution);
			writer.writeKeyVal("alphaMapTextureIds", alphaMapTextureIds);
		
			writer.writeKeyVal("baseMapResolution", terrainData.baseMapResolution);
			writer.writeKeyVal("bounds", bounds);
		
			writer.writeKeyVal("detailHeight", terrainData.detailHeight);
			writer.writeKeyVal("terrainDetails", detailPrototypes);
		
			writer.writeKeyVal("detailResolution", terrainData.detailResolution);
			writer.writeKeyVal("detailWidth", terrainData.detailWidth);
		
			writer.writeKeyVal("heighmapWidth", terrainData.heightmapWidth);
			writer.writeKeyVal("heightmapHeight", terrainData.heightmapHeight);
			writer.writeKeyVal("heightmapResolution", terrainData.heightmapResolution);
			writer.writeKeyVal("heightmapScale", terrainData.heightmapScale);
		
			writer.writeKeyVal("worldSize", terrainData.size);
			writer.writeKeyVal("thickness", terrainData.thickness);
			writer.writeKeyVal("treeInstanceCount", terrainData.treeInstanceCount);
		
			writer.writeKeyVal("treeInstances", treeInstances);
			writer.writeKeyVal("treePrototypes", treePrototypes);
		
			writer.writeKeyVal("wavingGrassAmount", terrainData.wavingGrassAmount);
			writer.writeKeyVal("wavingGrassSpeed", terrainData.wavingGrassSpeed);
			writer.writeKeyVal("wavingGrassStrength", terrainData.wavingGrassStrength);
			writer.writeKeyVal("wavingGrassTint", terrainData.wavingGrassTint);
			
			writer.endObject();
		}
		
		public JsonTerrainData(TerrainData terrainData_, ResourceMapper resMap){
			if (!terrainData_)
				throw new System.ArgumentNullException("terrainData");
				
			terrainData = terrainData_;
			
			name = terrainData.name;
			path = AssetDatabase.GetAssetPath(terrainData);
			exportPath = System.IO.Path.Combine(terrainAssetExportFolder, path);
			exportPath = System.IO.Path.ChangeExtension(exportPath, ".bin");
				
			var tex2ds = terrainData.alphamapTextures;
			alphaMapTextureIds.Clear();
			foreach(var cur in tex2ds){
				var curId = resMap.getTextureId(cur);
				alphaMapTextureIds.Add(curId);
			}
				
			//writer.writeKeyVal("alphaMapTextureIds", alphaMapTextureIds);			
			
			bounds = new JsonBounds(terrainData.bounds);
			
			detailPrototypes.Clear();
			var srcDetailPrototypes = terrainData.detailPrototypes;
			foreach(var cur in srcDetailPrototypes){
				detailPrototypes.Add(new JsonTerrainDetailPrototype(cur, resMap));
			}
			
			var srcTreePrototypes = terrainData.treePrototypes;
			treePrototypes.Clear();
			foreach(var cur in srcTreePrototypes){			
				treePrototypes.Add(new JsonTreePrototype(cur, resMap));
			}
						
			//writer.writeKeyVal("treeInstances", treeInstances);
			var srcTreeInstances = terrainData.treeInstances;
			treeInstances.Clear();
			foreach(var cur in srcTreeInstances){
				treeInstances.Add(new JsonTreeInstance(cur));
			}
		}
	};
}
