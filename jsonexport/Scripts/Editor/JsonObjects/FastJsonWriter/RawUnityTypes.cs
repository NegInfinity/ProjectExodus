using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public void writeRawValue(Color val){
			beginRawObject(false);
			writeKeyVal("r", val.r, false);
			writeKeyVal("g", val.g, false);
			writeKeyVal("b", val.b, false);
			writeKeyVal("a", val.a, false);
			endObject(false);
		}

		public void writeRawValue(Color32 val){
			beginRawObject(false);
			writeKeyVal("r", val.r, false);
			writeKeyVal("g", val.g, false);
			writeKeyVal("b", val.b, false);
			writeKeyVal("a", val.a, false);
			endObject(false);
		}
		
		public void writeRawValue(Vector2 val){
			beginRawObject(false);
			writeKeyVal("x", val.x, false);
			writeKeyVal("y", val.y, false);
			endObject(false);
		}

		public void writeRawValue(Vector3 val){
			beginRawObject(false);
			writeKeyVal("x", val.x, false);
			writeKeyVal("y", val.y, false);
			writeKeyVal("z", val.z, false);
			endObject(false);
		}

		public void writeRawValue(Vector4 val){
			beginRawObject(false);
			writeKeyVal("x", val.x, false);
			writeKeyVal("y", val.y, false);
			writeKeyVal("z", val.z, false);
			writeKeyVal("w", val.w, false);
			endObject(false);
		}

		public void writeRawValue(Matrix4x4 val){
			beginRawObject();
			writeKeyVal("e00", val.m00);
			writeKeyVal("e01", val.m01, false);
			writeKeyVal("e02", val.m02, false);
			writeKeyVal("e03", val.m03, false);

			writeKeyVal("e10", val.m10);
			writeKeyVal("e11", val.m11, false);
			writeKeyVal("e12", val.m12, false);
			writeKeyVal("e13", val.m13, false);

			writeKeyVal("e20", val.m20);
			writeKeyVal("e21", val.m21, false);
			writeKeyVal("e22", val.m22, false);
			writeKeyVal("e23", val.m23, false);

			writeKeyVal("e30", val.m30);
			writeKeyVal("e31", val.m31, false);
			writeKeyVal("e32", val.m32, false);
			writeKeyVal("e33", val.m33, false);
			endObject();
		}

		public void writeRawValue(Quaternion val){
			beginRawObject(false);
			writeKeyVal("x", val.x, false);
			writeKeyVal("y", val.y, false);
			writeKeyVal("z", val.z, false);
			writeKeyVal("w", val.w, false);
			endObject(false);
		}	
	}
}
