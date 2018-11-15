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
		
		//We... actually don't need those.
		//public List<int> alphaMapTextureIds = new List<int>();
		public JsonBounds bounds = new JsonBounds();
		
		public string heightMapRawPath;
		public List<string> alphaMapRawPaths = new List<string>();
		public List<string> detailMapRawPaths = new List<string>();
		
		public List<JsonTerrainDetailPrototype> detailPrototypes = new List<JsonTerrainDetailPrototype>();
		public List<JsonTreeInstance> treeInstances = new List<JsonTreeInstance>();
		public List<JsonTreePrototype> treePrototypes = new List<JsonTreePrototype>();
		public List<JsonSplatPrototype> splatPrototypes = new List<JsonSplatPrototype>();
		
		public static void registerLinkedData(TerrainData data, ResourceMapper resMap){
			if (!data)
				return;
			//resMap.registerMaterial(data.ma
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("exportPath", exportPath);
			
			writer.writeKeyVal("heightMapRawPath", heightMapRawPath);
			writer.writeKeyVal("alphaMapRawPaths", alphaMapRawPaths);
			writer.writeKeyVal("detailMapRawPaths", detailMapRawPaths);
			
			writer.writeKeyVal("alphaMapWidth", terrainData.alphamapWidth);
			writer.writeKeyVal("alphaMapHeight", terrainData.alphamapHeight);
			writer.writeKeyVal("alphaMapLayers", terrainData.alphamapLayers);
			writer.writeKeyVal("alphaMapResolution", terrainData.alphamapResolution);
			//writer.writeKeyVal("alphaMapTextureIds", alphaMapTextureIds);
		
			writer.writeKeyVal("baseMapResolution", terrainData.baseMapResolution);
			writer.writeKeyVal("bounds", bounds);
		
			writer.writeKeyVal("detailWidth", terrainData.detailWidth);
			writer.writeKeyVal("detailHeight", terrainData.detailHeight);
			writer.writeKeyVal("detailPrototypes", detailPrototypes);
			writer.writeKeyVal("detailResolution", terrainData.detailResolution);
		
			writer.writeKeyVal("heightmapWidth", terrainData.heightmapWidth);
			writer.writeKeyVal("heightmapHeight", terrainData.heightmapHeight);
			writer.writeKeyVal("heightmapResolution", terrainData.heightmapResolution);
			writer.writeKeyVal("heightmapScale", terrainData.heightmapScale);
		
			writer.writeKeyVal("worldSize", terrainData.size);
			writer.writeKeyVal("thickness", terrainData.thickness);
			writer.writeKeyVal("treeInstanceCount", terrainData.treeInstanceCount);

			writer.writeKeyVal("splatPrototypes", splatPrototypes);		
			writer.writeKeyVal("treePrototypes", treePrototypes);
			writer.writeKeyVal("treeInstances", treeInstances);
		
			writer.writeKeyVal("wavingGrassAmount", terrainData.wavingGrassAmount);
			writer.writeKeyVal("wavingGrassSpeed", terrainData.wavingGrassSpeed);
			writer.writeKeyVal("wavingGrassStrength", terrainData.wavingGrassStrength);
			writer.writeKeyVal("wavingGrassTint", terrainData.wavingGrassTint);
			
			writer.endObject();
		}
		
		public JsonTerrainData(TerrainData terrainData_, ResourceMapper resMap){
			if (!terrainData_)
				throw new System.ArgumentNullException("terrainData_");
				
			terrainData = terrainData_;
			
			name = terrainData.name;
			path = AssetDatabase.GetAssetPath(terrainData);
			exportPath = string.Format("{0}/{1}", terrainAssetExportFolder, path);//System.IO.Path.Combine(terrainAssetExportFolder, path);
			exportPath = System.IO.Path.ChangeExtension(exportPath, ".bin");
			
			heightMapRawPath = System.IO.Path.ChangeExtension(exportPath, ".height.raw");
			detailMapRawPaths.Clear();
			alphaMapRawPaths.Clear();
				
			for(int i = 0; i < terrainData.alphamapLayers; i++){
				var rawExt = string.Format(".alpha{0}.raw", i);
				alphaMapRawPaths.Add(System.IO.Path.ChangeExtension(exportPath, rawExt));
			}
			
			bounds = new JsonBounds(terrainData.bounds);
			
			detailPrototypes.Clear();
			var srcDetailPrototypes = terrainData.detailPrototypes;
			foreach(var cur in srcDetailPrototypes){
				detailPrototypes.Add(new JsonTerrainDetailPrototype(cur, resMap));
			}
			
			for(int i = 0; i < detailPrototypes.Count; i++){
				var rawExt = string.Format(".detail{0}.raw", i);
				detailMapRawPaths.Add(System.IO.Path.ChangeExtension(exportPath, rawExt));
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
			
			var splats = terrainData.splatPrototypes;
			splatPrototypes.Clear();
			foreach(var cur in splats){
				splatPrototypes.Add(new JsonSplatPrototype(cur, resMap));
			}
			
			
			//TODO: detail layer?
		}
	};
}
