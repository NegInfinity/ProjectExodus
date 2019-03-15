using UnityEngine;

namespace SceneExport{
	/*
	This class holds mesh-specific data --> The reference to the mesh itself, Prefab on which the mesh was originally found,
	and the root skeleton.
	*/
	[System.Serializable]
	public struct MeshStorageKey{
		public readonly Mesh mesh;//Immutability, huh. Ugh.
		public readonly GameObject prefab;
		public readonly Transform skeletonRoot;
			
		public override string ToString(){
			return string.Format("[MeshStorageKey]{{ mesh: {0}({1}); prefab: {2}({3}); skeletonRoot: {4}({5}) }}", 
				mesh, mesh ? mesh.GetInstanceID(): 0, 
				prefab, prefab ? prefab.GetInstanceID(): 0, 
				skeletonRoot, skeletonRoot ? skeletonRoot.GetInstanceID(): 0
			);
		}
		
		public override int GetHashCode(){
			int hash = 17;
			hash = hash * 23 + (mesh ? mesh.GetHashCode(): 0);
			hash = hash * 23 + (prefab ? prefab.GetHashCode(): 0);
			hash = hash * 23 + (skeletonRoot ? skeletonRoot.GetHashCode(): 0);
			return hash;
		}
			
		public override bool Equals(object obj){
			if (obj is MeshStorageKey){
				return this.Equals((MeshStorageKey)obj);
			}
				
			return false;
		}
			
		public bool Equals(MeshStorageKey other){
			return (mesh == other.mesh) 
				&& (prefab == other.prefab)
				&& (skeletonRoot == other.skeletonRoot);					
		}
			
		public MeshStorageKey(Mesh mesh_, GameObject prefab_ = null, Transform skeletonRoot_ = null){
			mesh = mesh_;
			prefab = prefab_;
			skeletonRoot = skeletonRoot_;
		}
	};		
}
