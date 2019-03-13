using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonTerrainDetailPrototype: IFastJsonValue{
		public DetailPrototype data;
		
		public ResId textureId = ResId.invalid;
		public ResId detailPrefabId = ResId.invalid;
		public ResId detailPrefabObjectId = ResId.invalid;
		
		public MeshIdData meshIdData = new MeshIdData();
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
			writer.writeKeyVal("detailMeshId", meshIdData.meshId);
			writer.writeKeyVal("detailMeshMaterials", meshIdData.meshMaterials);
			writer.writeKeyVal("detailPrefabId", detailPrefabId);
			writer.writeKeyVal("detailPrefabObjectId", detailPrefabObjectId);
			
			writer.endObject();
		}		
		
		public JsonTerrainDetailPrototype(DetailPrototype data_, ResourceMapper resMap){		
			data = data_;
			
			textureId = resMap.getTextureId(data.prototypeTexture);
			detailPrefabId = resMap.getRootPrefabId(data.prototype, true);
			detailPrefabObjectId = resMap.getPrefabObjectId(data.prototype, true);
			
			meshIdData = new MeshIdData(data.prototype, resMap);
			//meshId = resMap.getPrefabObjectId
		}
	}
}