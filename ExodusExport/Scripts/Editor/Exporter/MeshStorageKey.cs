using UnityEngine;

namespace SceneExport{
	/*
	By default meshes are treated as triangular collider.

	A request for a mesh collider will 
	*/
	[System.Serializable][System.Flags]
	public enum MeshUsageFlags{
		None = 0,
		ConvexCollider = 1,
		TriangleCollider = 2
	}


	/*
	This class holds mesh-specific data --> The reference to the mesh itself, Prefab on which the mesh was originally found,
	and the root skeleton.
	*/
	[System.Serializable]
	public struct MeshStorageKey{
		public readonly Mesh mesh;//Immutability, huh. Ugh.
		public readonly MeshUsageFlags usageFlags;
		public readonly GameObject prefab;
		public readonly Transform skeletonRoot;

		public string getMeshAssetSuffix(){
			if (usageFlags.HasFlag(MeshUsageFlags.ConvexCollider))
				return("_convex");
			if (usageFlags.HasFlag(MeshUsageFlags.TriangleCollider))
				return("_trimesh");
			return "";
		}
			
		public override string ToString(){
			return string.Format("[MeshStorageKey]{{ mesh: {0}({1}); usageFlags: {2}; prefab: {3}({4}); skeletonRoot: {5}({6}) }}", 
				mesh, mesh ? mesh.GetInstanceID(): 0, 
				usageFlags,
				prefab, prefab ? prefab.GetInstanceID(): 0, 
				skeletonRoot, skeletonRoot ? skeletonRoot.GetInstanceID(): 0
			);
		}
		
		public override int GetHashCode(){
			int hash = 17;
			hash = hash * 23 + (mesh ? mesh.GetHashCode(): 0);
			hash = hash * 23 + usageFlags.GetHashCode();
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
			
		public MeshStorageKey(Mesh mesh_, MeshUsageFlags usageFlags_, GameObject prefab_ = null, Transform skeletonRoot_ = null){
			mesh = mesh_;
			usageFlags = usageFlags_;
			prefab = prefab_;
			skeletonRoot = skeletonRoot_;
		}
	};		
}
