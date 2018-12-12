using UnityEngine;
using UnityEditor;

namespace SceneExport{
	/*
	This is a debug structure not used by unreal importer.
	*/
	[System.Serializable]
	public struct JsonTransform: IFastJsonValue{
		public Vector3 x;
		public Vector3 y;
		public Vector3 z;
		public Vector3 pos;
		
		public float getMaxDifference(JsonTransform other){
			return Mathf.Max(
				Mathf.Max(
					x.getMaxDifference(other.x), 
					y.getMaxDifference(other.y)
				),
				Mathf.Max(
					z.getMaxDifference(other.z), 
					pos.getMaxDifference(other.pos)
				)
			);
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject(false);
			
			writer.writeKeyVal("x", x, false);
			writer.writeKeyVal("y", y, false);
			writer.writeKeyVal("z", z, false);
			writer.writeKeyVal("pos", pos, false);
			
			writer.endObject(false);
		}
		
		public static JsonTransform identity{
			get{
				return new JsonTransform(Matrix4x4.identity);
				//var result = new JsonTransform()
			}
		}	
		/*
		public JsonTransform(){
			x = new Vector3(1.0f, 0.0f, 0.0f);
			y = new Vector3(0.0f, 1.0f, 0.0f);
			z = new Vector3(0.0f, 0.0f, 1.0f);
			pos = Vector3.zero;
		}
		*/
		
		void setFromMatrix(Matrix4x4 m){
			x = m.MultiplyVector(new Vector3(1.0f, 0.0f, 0.0f));
			y = m.MultiplyVector(new Vector3(0.0f, 1.0f, 0.0f));
			z = m.MultiplyVector(new Vector3(0.0f, 0.0f, 1.0f));
			pos = m.MultiplyPoint(Vector3.zero);
		}
		
		public JsonTransform(Matrix4x4 m, bool inverse = false): this(){
			if (inverse)
				m = m.inverse;
			setFromMatrix(m);//Goddamit. "This object cannot be used before all its fields are assigned to"
		}
		
		void setFromTransforms(Transform t, Transform refTransform){
			if (!t)
				throw new System.ArgumentNullException("t");
				
			var matrix = t.localToWorldMatrix;
			if (refTransform){
				var invParent = refTransform.worldToLocalMatrix;
				matrix = invParent * matrix;
			}
			
			setFromMatrix(matrix);
		}
			
		public JsonTransform(Transform t, Transform refTransform): this(){
			setFromTransforms(t, refTransform);		
		}
			
		public JsonTransform(Transform t, bool useGlobal): this(){
			setFromTransforms(t, useGlobal ? null: t.parent);		
		}
	}
}