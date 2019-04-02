using UnityEngine;
using UnityEditor;

namespace SceneExport{
    [System.Serializable]
    public class JsonCollider: IFastJsonValue{
        public string colliderType;//"box", "sphere", "capsule", "mesh"
		public int colliderIndex = -1;
        public Vector3 center = Vector3.zero;//all colliders
        public Vector3 size = Vector3.zero;
        public int direction = 0;//0 = x, 1 = y, 2 = z; 
        public float radius = 0.0f;//capsule and sphere
        public float height = 0.0f;//capsule only        
        public ResId meshId = ResId.invalid; //meshCollider
        public Collider collider= null;

		public bool triMeshCollision = false;//used for triangular mesh collision somewhere if this is set
		public bool convexMeshCollision = false;//used for convex mesh collision somewhere if this is et.

        static readonly string colliderTypeBox = "box";
        static readonly string colliderTypeSphere = "sphere";
        static readonly string colliderTypeCapsule = "capsule";
        static readonly string colliderTypeMesh = "mesh";

		public bool isSupportedType(){
			return !string.IsNullOrEmpty(colliderType);
		}

        public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
            writer.writeKeyVal("colliderType", colliderType);
            writer.writeKeyVal("colliderIndex", colliderIndex);
            writer.writeKeyVal("center", center);
            writer.writeKeyVal("size", size);
            writer.writeKeyVal("direction", direction);
            writer.writeKeyVal("radius", radius);
            writer.writeKeyVal("height", height);
            writer.writeKeyVal("meshId", meshId);

            if (!collider){
                Debug.LogWarningFormat("Collider not set");
                return;
            }

            writer.writeKeyVal("enabled", collider.enabled);
            writer.writeKeyVal("contactOffset", collider.contactOffset);
            writer.writeKeyVal("bounds", new JsonBounds(collider.bounds));
            writer.writeKeyVal("isTrigger", collider.isTrigger);

			writer.writeKeyVal("triMeshCollision", triMeshCollision);
			writer.writeKeyVal("convexMeshCollision", convexMeshCollision);

            //material(physics), sharedMaterial, tag, hideFlags, name....
            //TODO: physics material...            
			writer.endObject();
        }

        public JsonCollider(Collider col, int colliderIndex_, ResourceMapper resMap){
			colliderIndex = colliderIndex_;
            if (!col)
                throw new System.ArgumentNullException("col");
            if (resMap == null)
                throw new System.ArgumentNullException("resMap");
            var box = col as BoxCollider;
            var sphere = col as SphereCollider;
            var capsule = col as CapsuleCollider;
            var meshCol = col as MeshCollider;

            collider = col;
            if (box){
                colliderType = colliderTypeBox;
                center = box.center;
                size = box.size;
            }   
            if (sphere){
                colliderType = colliderTypeSphere;
                center = sphere.center;
                radius = sphere.radius;
            }   
            if(capsule){
                colliderType = colliderTypeCapsule;
                center = capsule.center;
                radius = capsule.radius;
                height = capsule.height;
                direction = capsule.direction;
            }
            if (meshCol){
                colliderType = colliderTypeMesh;
                meshId = resMap.getMeshId(meshCol.sharedMesh, 
					meshCol.convex ? MeshUsageFlags.ConvexCollider: MeshUsageFlags.TriangleCollider
				);

				bool isConvex = meshCol.convex;

				/*
				if (resMap.isValidMeshId(meshId))
					resMap.flagMeshId(meshId, isConvex, !isConvex);//by default there will be no flags on a mesh...
				*/
            }
        }
    }
}
