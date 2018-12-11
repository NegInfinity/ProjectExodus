using UnityEngine;
using UnityEditor;

namespace SceneExport{
	/*
	This is a debug structure not used by unreal importer.
	*/
	[System.Serializable]
	public class JsonTransform: IFastJsonValue{
		public Vector3 x;
		public Vector3 y;
		public Vector3 z;
		public Vector3 pos;
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			
			writer.writeKeyVal("x", x);
			writer.writeKeyVal("y", y);
			writer.writeKeyVal("z", z);
			writer.writeKeyVal("pos", pos);
			
			writer.endObject();
		}
			
		public JsonTransform(){
			x = new Vector3(1.0f, 0.0f, 0.0f);
			y = new Vector3(0.0f, 1.0f, 0.0f);
			z = new Vector3(0.0f, 0.0f, 1.0f);
			pos = Vector3.zero;
		}
		
		void setFromMatrix(Matrix4x4 m){
			x = m.MultiplyVector(new Vector3(1.0f, 0.0f, 0.0f));
			y = m.MultiplyVector(new Vector3(0.0f, 1.0f, 0.0f));
			z = m.MultiplyVector(new Vector3(0.0f, 0.0f, 1.0f));
			pos = m.MultiplyPoint(Vector3.zero);
		}
		
		public JsonTransform(Matrix4x4 m, bool inverse = false){
			if (inverse)
				m = m.inverse;
			setFromMatrix(m);
		}
			
		public JsonTransform(Transform t, bool useGlobal){
			if (!t)
				throw new System.ArgumentNullException("t");
				
			var matrix = t.localToWorldMatrix;
			if (!useGlobal && t.parent){
				var invParent = t.parent.worldToLocalMatrix;
				matrix = invParent * matrix;
			}
			
			setFromMatrix(matrix);
		}
	}
}