using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class MeshIdData{
		public int meshId = -1;
		public List<int> meshMaterials = new List<int>();
		public MeshIdData(){
		}
		public MeshIdData(GameObject srcObj, ResourceMapper resMap){
			if (!srcObj)
				return;
			var filter = srcObj.GetComponent<MeshFilter>();
			if (filter && filter.sharedMesh){
				meshId = resMap.getMeshId(filter.sharedMesh);
			}
			var renderer = srcObj.GetComponent<Renderer>();
			if (renderer){
				foreach(var cur in renderer.sharedMaterials){
					meshMaterials.Add(resMap.getMaterialId(cur));
				}
			}
		}
	}
}
