using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
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
