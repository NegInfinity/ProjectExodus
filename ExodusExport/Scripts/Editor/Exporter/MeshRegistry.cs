using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	/*
		Well, due to differences in handling unity's and unreal skeletal meshes, 
		we can no longer use naked mesh to uniquely id
		mesh being used.
			
		So we're going to pair it with prefab root and.... I guess skeleton root as well.
		This won't be needed for non-skinned meshes, of course.
	*/
	[System.Serializable]
	public class MeshRegistry{
		public ObjectMapper<MeshStorageKey> meshes = new ObjectMapper<MeshStorageKey>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();

		public List<Material> findMeshMaterials(Mesh mesh){
			List<Material> result = null;
			if (meshMaterials.TryGetValue(mesh, out result))
				return result;
			return null;
		}

		public ResId getMeshId(Mesh obj, MeshUsageFlags useFlags){
			var key = new MeshStorageKey(obj, useFlags);
			return meshes.getId(key, true);
		}

		public bool isValidMeshId(ResId id){
			return meshes.isValidId(id);
		}

		public ResId findMeshId(Mesh obj, MeshUsageFlags useFlags){
			var key = new MeshStorageKey(obj, useFlags);
			return meshes.getId(key, false);
		}

		public ResourceStorageWatcher<ObjectMapper<MeshStorageKey>, MeshStorageKey> createWatcher(){
			return meshes.createWatcher();
		}

		public ResId getOrRegMeshId(MeshStorageKey meshKey, GameObject obj, Mesh mesh){
			if (!mesh){
				return ResId.invalid;
			}
			ResId result = meshes.getId(meshKey, true, null);
			if (meshMaterials.ContainsKey(mesh))
				return result;
							
			var r = obj.GetComponent<Renderer>();
			if (r){
				meshMaterials[mesh] = new List<Material>(r.sharedMaterials);
			}
			return result;
		}
	}
}
