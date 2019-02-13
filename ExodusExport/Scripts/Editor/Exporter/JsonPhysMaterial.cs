using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEditor;
using UnityEngine.Assertions;

namespace SceneExport{
	[System.Serializable]
	public class JsonPhysMaterial: IFastJsonValue{
		public string name;
		public string path;
		public int id = -1;
		public PhysicMaterial physMaterial = null;

		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", physMaterial.name);
			writer.writeKeyVal("path", AssetDatabase.GetAssetPath(physMaterial));
			if (physMaterial){
				writer.writeKeyVal("staticFriction", physMaterial.staticFriction);
				writer.writeKeyVal("dynamicFriction", physMaterial.dynamicFriction);
				writer.writeKeyVal("bounciness", physMaterial.bounciness);
				writer.writeKeyVal("bounceCombine", physMaterial.bounceCombine.ToString());
				writer.writeKeyVal("frictionCombine", physMaterial.frictionCombine.ToString());
			}
			writer.endObject();
		}

		public JsonPhysMaterial(PhysicMaterial physMat, ResourceMapper resMap){
			Assert.IsNotNull(physMat);
			Assert.IsNotNull(resMap);
			// id = 
			name = physMat.name;
			path = AssetDatabase.GetAssetPath(physMat);
			physMaterial = physMat;
		}
	}
}
