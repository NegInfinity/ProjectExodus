using UnityEngine;

using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonSkinRendererData: IFastJsonValue{
		public string quality;
		public bool skinnedMotionVectors = false;
		public bool updateWhenOffscreen = false;
		public List<string> boneNames = new List<string>();
		public List<ResId> boneIds = new List<ResId>();
		public List<Matrix4x4> boneTransforms = new List<Matrix4x4>();
		
		public ResId meshId = ResId.invalid;
		public List<ResId> materials = new List<ResId>();		
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("quality", quality);
			writer.writeKeyVal("skinnedMotionVectors", skinnedMotionVectors);
			writer.writeKeyVal("updateWhenOffscreen", updateWhenOffscreen);
			writer.writeKeyVal("boneNames", boneNames);
			writer.writeKeyVal("boneIds", boneIds);
			writer.writeKeyVal("boneTransforms", boneTransforms);
			
			writer.writeKeyVal("meshId", meshId);
			writer.writeKeyVal("materials", materials);
			writer.endObject();
		}
		
		public JsonSkinRendererData(SkinnedMeshRenderer rend, GameObjectMapper objMap, ResourceMapper resMap){
			if (!rend)
				throw new System.ArgumentNullException("rend");
			quality = rend.quality.ToString();
			skinnedMotionVectors = rend.skinnedMotionVectors;
			updateWhenOffscreen = rend.updateWhenOffscreen;
			
			boneNames.Clear();
			boneIds.Clear();
			foreach(var cur in rend.bones){
				boneNames.Add(cur.name);
				boneTransforms.Add(cur.localToWorldMatrix);				
				boneIds.Add(objMap.getId(cur.gameObject));				
			}
			
			var sharedMesh  = rend.sharedMesh;
			meshId = resMap.getOrRegMeshId(rend, null);//resMap.getMeshId(sharedMesh);
			
			materials.Clear();
			var sharedMats = rend.sharedMaterials;
			foreach(var curMat in sharedMats){
				materials.Add(resMap.getMaterialId(curMat));
			}
		}
	}
}
