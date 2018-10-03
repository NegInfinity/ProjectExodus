using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public class FastJsonWriter{
		public int indent = 0;
		public StringBuilder builder = new StringBuilder();
		public Stack<int> valCount = new Stack<int>();

		public void beginDocument(){
			beginRawObject();
		}

		public void endDocument(){
			endObject();
		}

		public string getString(){
			return builder.ToString();
		}

		public void writeIndent(){
			for (int i = 0; i < indent; i++)
				builder.Append("\t");
		}

		public void processComma(){
			var count = valCount.Pop();
			valCount.Push(count+1);
			if (count > 0)
				builder.AppendLine(",");
			writeIndent();
		}

		public void beginRawObject(){
			builder.AppendLine("{");
			indent++;
			valCount.Push(0);
		}

		public void beginRawArray(){
			builder.AppendLine("[");
			indent++;
			valCount.Push(0);
		}

		public void endObject(){
			indent--;
			builder.AppendLine();
			writeIndent();
			builder.Append("}");
			valCount.Pop();
		}

		public void endArray(){
			indent--;
			builder.AppendLine();
			writeIndent();
			builder.Append("]");
			valCount.Pop();
		}

		public void beginArrayValue(){
			processComma();
			beginRawArray();
		}

		public void beginObjectValue(){
			processComma();
			beginRawObject();
		}

		public void writeKey(string key){
			processComma();
			writeString(key);
			builder.Append(": ");
		}

		public void writeKeyVal(string key, string val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, bool val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, int val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, float val){
			writeKey(key);
			writeRawValue(val);
		}

		public void beginKeyArray(string key){
			writeKey(key);
			beginRawArray();
		}

		public void beginKeyObject(string key){
			writeKey(key);
			beginRawObject();
		}

		public void writeRawValue(bool b){
			if (b)
				builder.Append("true");
			else
				builder.Append("false");
		}

		public void writeRawValue(int[] val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(IList<int> val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(float[] val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(Color[] val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(IList<float> val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(Vector3 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			endObject();
		}

		public void writeRawValue(Color val){
			beginRawObject();
			writeKeyVal("r", val.r);
			writeKeyVal("g", val.g);
			writeKeyVal("b", val.b);
			writeKeyVal("a", val.a);
			endObject();
		}

		public void writeRawValue(Matrix4x4 val){
			beginRawObject();
			writeKeyVal("e00", val.m00);
			writeKeyVal("e01", val.m01);
			writeKeyVal("e02", val.m02);
			writeKeyVal("e03", val.m03);

			writeKeyVal("e10", val.m10);
			writeKeyVal("e11", val.m11);
			writeKeyVal("e12", val.m12);
			writeKeyVal("e13", val.m13);

			writeKeyVal("e20", val.m20);
			writeKeyVal("e21", val.m21);
			writeKeyVal("e22", val.m22);
			writeKeyVal("e23", val.m23);

			writeKeyVal("e30", val.m30);
			writeKeyVal("e31", val.m31);
			writeKeyVal("e32", val.m32);
			writeKeyVal("e33", val.m33);
			endObject();
		}

		public void writeRawValue(Vector2 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			endObject();
		}

		public void writeRawValue(Vector4 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			writeKeyVal("w", val.w);
			endObject();
		}

		public void writeRawValue(Quaternion val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			writeKeyVal("w", val.w);
			endObject();
		}

		public void writeRawValue(int val){
			builder.Append(val);
		}

		public void writeRawValue(float val){
			builder.Append(val);
		}

		public void writeRawValue(string s){
			writeString(s);
		}

		/*
		public void writeKeyVal<Val>(string key, Val val){
			writeKey(key);
			dynamic tmp = val;
			writeRawValue(tmp);
		}

		public void writeValue<Val>(Val val){
			processComma();
			dynamic tmp = val;
			writeRawValue(tmp);
		}
		*/
		public void writeKeyVal(string key, int[]val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(int[] val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Color[]val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(Color[] val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, List<int> val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(List<int> val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, float[]val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(float[] val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, List<float> val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(List<float> val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(string val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(int val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(float val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Vector3 val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Vector2 val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Vector4 val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Quaternion val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Color val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Matrix4x4 val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Vector3 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Vector2 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Vector4 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Quaternion val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Color val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Matrix4x4 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeString(string s){
			builder.Append("\"");
			if (s != null){
				foreach(var c in s){
					switch(c){
						case '"':
							builder.Append("\\\""); break;
        				case '\\': 
	        				builder.Append("\\\\"); break;
    	    			case '\b': 
        					builder.Append("\\b"); break;
        				case '\f': 
        					builder.Append("\\f"); break;
        				case '\n': 
        					builder.Append("\\n"); break;
        				case '\r': 
        					builder.Append("\\r"); break;
						case '\t': 
							builder.Append("\\t"); break;
						default:{
							if (('\x00' <= c) && (c <= '\x1f')){
								builder.Append("\\u");
								builder.AppendFormat("{0:x4}", (int)c);
							}
							else
								builder.Append(c);
							break;
						}
					}
				}
			}
			builder.Append("\"");
		}
	}

	public static class Utility{
		public static void saveStringToFile(string filename, string data){
			System.IO.File.WriteAllText(filename, data, System.Text.Encoding.UTF8);
		}

		public static void saveBytesToFile(string filename, byte[] data){
			System.IO.File.WriteAllBytes(filename, data);
		}
	}

	public class Exporter{
		static public Vector3[] copyArray(Vector3[] arg){
			int size = 0;
			if (arg != null)
				size = arg.Length;
			var result = new Vector3[size];
			for(int i = 0; i < size; i++)
				result[i] = arg[i];
			return result;
		}

		static public int[] copyArray(int[] arg){
			int size = 0;
			if (arg != null)
				size = arg.Length;
			var result = new int[size];
			for(int i = 0; i < size; i++)
				result[i] = arg[i];
			return result;
		}

		static public float[] toFloats(Matrix4x4 arg){
			float[] result = new float[16];
			for(int i = 0; i < 4; i++){
				for(int j = 0; j < 4; j++){
					result[i*4+j] = arg[i, j];
				}
			}
			return result;
		}

		static public float[] toFloats(Vector3[] arg){
			int numValues = 0;
			if (arg != null)
				numValues = arg.Length;
			var result = new float[numValues * 3];
			for(int i = 0; i < numValues; i++){
				result[i*3 + 0] = arg[i].x;
				result[i*3 + 1] = arg[i].y;
				result[i*3 + 2] = arg[i].z;
			}
			return result;				
		}

		static public float[] toFloats(Vector2[] arg){
			int numValues = 0;
			if (arg != null)
				numValues = arg.Length;
			var result = new float[numValues * 2];
			for(int i = 0; i < numValues; i++){
				result[i*2 + 0] = arg[i].x;
				result[i*2 + 1] = arg[i].y;
			}
			return result;				
		}


		[System.Serializable]
		public class JsonMesh{
			public int id = -1;
			public string name;
			public string path;
			public List<int> materials = new List<int>();
			public bool readable = false;
			public int vertexCount = 0;
			public Color[] colors = null;

			public float[] verts = null;
			public float[] normals = null;
			public float[] uv0 = null;
			public float[] uv1 = null;
			public float[] uv2 = null;
			public float[] uv3 = null;

			/*
			public List<float> verts = new List<float>();
			public List<float> normals = new List<float>();
			public List<float> uv0 = new List<float>();
			public List<float> uv1 = new List<float>();
			public List<float> uv2 = new List<float>();
			public List<float> uv3  = new List<float>();
			*/

			[System.Serializable]
			public class SubMesh{
				//public List<int> triangles = new List<int>();
				public int[] triangles = null;//new int[0];
			};

			public List<SubMesh> subMeshes = new List<SubMesh>();
			//public int[][] subMeshes = new int[][0];
			public int subMeshCount = 0;

			public JsonMesh(Mesh mesh, Exporter exp){
				id = exp.meshes.findId(mesh);
				name = mesh.name;
				//Debug.LogFormat("Processing mesh {0}", name);
				var filePath = AssetDatabase.GetAssetPath(mesh);
				exp.registerResource(filePath);
				path = filePath;

				List<Material> foundMaterials = null;
				if (exp.meshMaterials.TryGetValue(mesh, out foundMaterials) && (foundMaterials != null)){
					foreach(var cur in foundMaterials){
						materials.Add(exp.materials.getId(cur));
					}
				}

				#if !UNITY_EDITOR
				readable = mesh.isReadable;
				if (!readable){
					Debug.LogErrorFormat(string.Format("Mesh {0} is not marked as readable. Canot proceed", name);
					return;
				}
				#endif
				vertexCount = mesh.vertexCount;
				if (vertexCount <= 0)
					return;

				colors = mesh.colors;
				verts = toFloats(mesh.vertices);//toFloats(mesh.vertices);
				normals = toFloats(mesh.normals);
				uv0 = toFloats(mesh.uv);
				uv1 = toFloats(mesh.uv2);
				uv2 = toFloats(mesh.uv3);
				uv3 = toFloats(mesh.uv4);
				//uv1 = mesh.uv2;
				//uv2 = mesh.uv3;
				//uv3 = mesh.uv4;

				subMeshCount = mesh.subMeshCount;
				for(int i = 0; i < subMeshCount; i++){
					var subMesh = new SubMesh();
					subMesh.triangles = copyArray(mesh.GetTriangles(i));
					subMeshes.Add(subMesh);
				}

				//Debug.LogFormat("Processed mesh {0}", name);
			}
		}

		[System.Serializable]
		public class JsonMaterial{
			public int id = -1;
			public int renderQueue = 0;
			public string name;
			public string path;
			public string shader;
			//public string renderType;
			public int mainTexture = -1;
			public Vector2 mainTextureOffset = Vector2.zero;
			public Vector2 mainTextureScale = Vector2.one;
			public Color color = Color.white;

			public bool useNormalMap = false;
			public bool useAlphaTest = false;
			public bool useAlphaBlend = false;
			public bool useAlphaPremultiply = false;
			public bool useEmission = false;
			public bool useParallax = false;
			public bool useDetailMap = false;
			public bool useMetallic = false;
			public bool useSpecular = false;
			public int albedoTex = -1;
			public int specularTex = -1;
			public int metallicTex = -1;
			public int normalMapTex = -1;
			public int occlusionTex = -1;
			public int parallaxTex = -1;
			public int emissionTex = -1;
			public int detailMaskTex = -1;
			public int detailAlbedoTex = -1;
			public int detailNormalMapTex = -1;
			public float alphaCutoff = 1.0f;
			public float smoothness = 0.5f;
			public Color specularColor = Color.white;
			public float metallic = 0.5f;
			public float bumpScale = 1.0f;
			public float parallaxScale = 1.0f;
			public float occlusionStrength = 1.0f;
			public Color emissionColor = Color.black;
			public float detailMapScale = 1.0f;
			public float secondaryUv = 1.0f;

			public JsonMaterial(Material mat, Exporter exp){
				name = mat.name;
				id = exp.materials.findId(mat);
				renderQueue = mat.renderQueue;
				var path = AssetDatabase.GetAssetPath(mat);
				this.path = path;
				shader = mat.shader.name;
				mainTexture = exp.getTextureId(mat.mainTexture);
				mainTextureOffset = mat.mainTextureOffset;
				mainTextureScale = mat.mainTextureScale;
				color = mat.color;

				useNormalMap = mat.IsKeywordEnabled("_NORMALMAP");

				useAlphaTest = mat.IsKeywordEnabled("_ALPHATEST_ON");
				useAlphaBlend = mat.IsKeywordEnabled("_ALPHABLEND_ON");
				useAlphaPremultiply = mat.IsKeywordEnabled("_ALPHAPREMULTIPLY_ON");
				useEmission = mat.IsKeywordEnabled("_EMISSION");
				useParallax = mat.IsKeywordEnabled("_PARALLAXMAP");
				useDetailMap = mat.IsKeywordEnabled("_DETAIL_MULX2");
				useMetallic = mat.IsKeywordEnabled("_METALLICGLOSSMAP");
				useSpecular = mat.IsKeywordEnabled("_SPECCGLOSSMAP");

				System.Func<string, int> getTexId = (texName) => {
					if (!mat.HasProperty(texName))
						return -1;
					return exp.getTextureId(mat.GetTexture(texName));
				};
				System.Func<string, float, float> getFloat = (paramName, defaultVal) => {
					if (!mat.HasProperty(paramName))
						return defaultVal;
					return mat.GetFloat(paramName);
				};
				System.Func<string, Color, Color> getColor = (paramName, defaultVal) => {
					if (!mat.HasProperty(paramName))
						return defaultVal;
					return mat.GetColor(paramName);
				};

				albedoTex = getTexId("_MainTex");
				specularTex = getTexId("_SpecGlossMap");
				metallicTex= getTexId("_MetallicGlossMap");
				normalMapTex = getTexId("_BumpMap");
				occlusionTex = getTexId("_OcclusionMap");
				parallaxTex = getTexId("_ParallaxMap");
				emissionTex = getTexId("_EmissionMap");
				detailMaskTex = getTexId("_DetailMask");
				detailAlbedoTex = getTexId("_DetailAlbedoMap");
				detailNormalMapTex= getTexId("_DetailNormalMap");

				alphaCutoff = getFloat("_Cutoff", 1.0f);
				smoothness = getFloat("_Glossiness", 0.5f);
				specularColor = getColor("_SpecColor", Color.white);
				metallic = getFloat("_Metallic", 0.5f);
				bumpScale = getFloat("_BumpScale", 1.0f);
				parallaxScale = getFloat("_Parallax", 1.0f);
				occlusionStrength = getFloat("_OcclusionStrength", 1.0f);
				emissionColor = getColor("_EmissionColor", Color.black);
				detailMapScale = getFloat("_DetailNormalMapScale", 1.0f);
				secondaryUv = getFloat("_UVSec", 1.0f);
			}
		}

		[System.Serializable]
		public class JsonScene{
			public List<JsonGameObject> objects = new List<JsonGameObject>();
			public List<JsonMaterial> materials = new List<JsonMaterial>();
			public List<JsonMesh> meshes = new List<JsonMesh>();
			public List<JsonTexture> textures = new List<JsonTexture>();
			public List<string> resources = new List<string>();

			public string toJsonString(){
				FastJsonWriter writer = new FastJsonWriter();
				writer.beginDocument();
				writer.beginKeyArray("objects");
				foreach(var cur in objects){
					writer.beginObjectValue();
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("instanceId", cur.instanceId);
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("localPosition", cur.localPosition);
					writer.writeKeyVal("localRotation", cur.localRotation);
					writer.writeKeyVal("localScale", cur.localScale);
					writer.writeKeyVal("worldMatrix", cur.worldMatrix);
					writer.writeKeyVal("localMatrix", cur.localMatrix);
					writer.writeKeyVal("children", cur.children);
					writer.writeKeyVal("parent", cur.parent);
					writer.writeKeyVal("mesh", cur.mesh);
					writer.writeKeyVal("isStatic", cur.isStatic);
					writer.writeKeyVal("lightMapStatic", cur.lightMapStatic);
					writer.writeKeyVal("navigationStatic", cur.navigationStatic);
					writer.writeKeyVal("occluderStatic", cur.occluderStatic);
					writer.writeKeyVal("occludeeStatic", cur.occludeeStatic);
					writer.writeKeyVal("reflectionProbeStatic", cur.reflectionProbeStatic);
					writer.beginKeyArray("renderer");
					if (cur.renderer != null){
						foreach(var r in cur.renderer){
							writer.beginObjectValue();
							writer.writeKeyVal("lightmapIndex", r.lightmapIndex);
							writer.writeKeyVal("shadowCastingMode", r.shadowCastingMode);
							writer.writeKeyVal("lightmapScaleOffset", r.lightmapScaleOffset);
							writer.writeKeyVal("materials", r.materials);
							writer.writeKeyVal("receiveShadows", r.receiveShadows);
							writer.endObject();
						}
					}
					writer.endArray();
					writer.beginKeyArray("light");
					if (cur.light != null){
						foreach(var l in cur.light){
							writer.beginObjectValue();
							writer.writeKeyVal("bounceIntensity", l.bounceIntensity);
							writer.writeKeyVal("color", l.color);
							writer.writeKeyVal("intensity", l.intensity);
							writer.writeKeyVal("range", l.range);
							writer.writeKeyVal("renderMode", l.renderMode);
							writer.writeKeyVal("shadows", l.shadows);
							writer.writeKeyVal("shadowStrength", l.shadowStrength);
							writer.writeKeyVal("spotAngle", l.spotAngle);
							writer.writeKeyVal("type", l.type);
							writer.endObject();
						}
					}
					writer.endArray();
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("materials");
				foreach(var cur in materials){
					writer.beginObjectValue();
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("renderQueue", cur.renderQueue);
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("path", cur.path);
					writer.writeKeyVal("shader", cur.shader);
					//writer.writeKeyVal("renderType", cur.renderType);
					writer.writeKeyVal("mainTexture", cur.mainTexture);
					writer.writeKeyVal("mainTextureOffset", cur.mainTextureOffset);
					writer.writeKeyVal("mainTextureScale", cur.mainTextureScale);
					writer.writeKeyVal("color", cur.color);
					writer.writeKeyVal("useNormalMap", cur.useNormalMap);
					writer.writeKeyVal("useAlphaTest", cur.useAlphaTest);
					writer.writeKeyVal("useAlphaBlend", cur.useAlphaBlend);
					writer.writeKeyVal("useAlphaPremultiply", cur.useAlphaPremultiply);
					writer.writeKeyVal("useEmission", cur.useEmission);
					writer.writeKeyVal("useParallax", cur.useParallax);
					writer.writeKeyVal("useDetailMap", cur.useDetailMap);
					writer.writeKeyVal("useMetallic", cur.useMetallic);
					writer.writeKeyVal("useSpecular", cur.useSpecular);
					writer.writeKeyVal("albedoTex", cur.albedoTex);
					writer.writeKeyVal("specularTex", cur.specularTex);
					writer.writeKeyVal("metallicTex", cur.metallicTex);
					writer.writeKeyVal("normalMapTex", cur.normalMapTex);
					writer.writeKeyVal("occlusionTex", cur.occlusionTex);
					writer.writeKeyVal("parallaxTex", cur.parallaxTex);
					writer.writeKeyVal("emissionTex", cur.emissionTex);
					writer.writeKeyVal("detailMaskTex", cur.detailMaskTex);
					writer.writeKeyVal("detailAlbedoTex", cur.detailAlbedoTex);
					writer.writeKeyVal("detailNormalMapTex", cur.detailNormalMapTex);
					writer.writeKeyVal("alphaCutoff", cur.alphaCutoff);
					writer.writeKeyVal("smoothness", cur.smoothness);
					writer.writeKeyVal("specularColor", cur.specularColor);
					writer.writeKeyVal("metallic", cur.metallic);
					writer.writeKeyVal("bumpScale", cur.bumpScale);
					writer.writeKeyVal("parallaxScale", cur.parallaxScale);
					writer.writeKeyVal("occlusionStrength", cur.occlusionStrength);
					writer.writeKeyVal("emissionColor", cur.emissionColor);
					writer.writeKeyVal("detailMapScale", cur.detailMapScale);
					writer.writeKeyVal("secondaryUv", cur.secondaryUv);
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("meshes");
				foreach(var cur in meshes){
					writer.beginObjectValue();
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("path", cur.path);
					writer.writeKeyVal("materials", cur.materials);
					writer.writeKeyVal("readable", cur.readable);
					writer.writeKeyVal("vertexCount", cur.vertexCount);
					writer.writeKeyVal("colors", cur.colors);
					writer.writeKeyVal("verts", cur.verts);
					writer.writeKeyVal("normals", cur.normals);
					writer.writeKeyVal("uv0", cur.uv0);
					writer.writeKeyVal("uv1", cur.uv1);
					writer.writeKeyVal("uv2", cur.uv2);
					writer.writeKeyVal("uv3", cur.uv3);
					writer.writeKeyVal("subMeshCount", cur.subMeshCount);
					writer.beginKeyArray("subMeshes");
					foreach(var curSubMesh in cur.subMeshes){
						writer.beginObjectValue();
						writer.writeKeyVal("triangles", curSubMesh.triangles);
						writer.endObject();
					}
					writer.endArray();
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("textures");
				foreach(var cur in textures){
					writer.beginObjectValue();
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("path", cur.path);
					writer.writeKeyVal("filterMode", cur.filterMode);
					writer.writeKeyVal("mipMapBias", cur.mipMapBias);
					writer.writeKeyVal("width", cur.width);
					writer.writeKeyVal("height", cur.height);
					writer.writeKeyVal("wrapMode", cur.wrapMode);
					writer.writeKeyVal("isTex2D", cur.isTex2D);
					writer.writeKeyVal("isRenderTarget", cur.isRenderTarget);
					writer.writeKeyVal("alphaTransparency", cur.alphaTransparency);
					writer.writeKeyVal("anisoLevel", cur.anisoLevel);
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("resources");
				foreach(var cur in resources){
					writer.writeValue(cur);
				}
				writer.endArray();
				writer.endDocument();
				return writer.getString();
			}

		}

		[System.Serializable]
		public class JsonTexture{		
			public string name;
			public int id = -1;
			public string path;
			public string filterMode;
			public float mipMapBias = 0.0f;
			public int width = 0;
			public int height = 0;
			public string wrapMode;
			public bool isTex2D = false;
			public bool isRenderTarget = false;
			public bool alphaTransparency = false;
			public float anisoLevel = 0.0f;
			public string base64;
			public JsonTexture(Texture tex, Exporter exp){
				name = tex.name;
				id = exp.textures.findId(tex);
				var filePath = AssetDatabase.GetAssetPath(tex);
				exp.registerResource(filePath);
				path = filePath;
				filterMode = tex.filterMode.ToString();
				width = tex.width;
				height = tex.height;
				wrapMode = tex.wrapMode.ToString();
				var tex2D = tex as Texture2D;
				var rendTarget = tex as RenderTexture;
				isTex2D = tex2D != null;
				isRenderTarget = rendTarget != null;
				if (isTex2D){
					alphaTransparency = tex2D.alphaIsTransparency;
				}
				if (isRenderTarget){
					anisoLevel = rendTarget.anisoLevel;
				}
			}
		}

		public class ObjectMapper<Resource>{
			public delegate void OnNewObjectAdded(Resource res);
			public int nextId = 0;
			public Dictionary<Resource, int> objectMap = new Dictionary<Resource, int>();
			public List<Resource> objectList = new List<Resource>();

			public int findId(Resource obj){
				int result = -1;
				if (Object.Equals(obj, null))
					return result;
				if (objectMap.TryGetValue(obj, out result))
					return result;
				return -1;
			}

			public int getId(Resource obj, OnNewObjectAdded onAddCallback = null){
				int result = -1;
				if (Object.Equals(obj, null))
					return result;
				if (objectMap.TryGetValue(obj, out result))
					return result;
				result = nextId;
				objectMap[obj] = result;
				objectList.Add(obj);
				nextId++;
				if (onAddCallback != null)
					onAddCallback(obj);
				return result;						
			}
		}

		ObjectMapper<GameObject> objects = new ObjectMapper<GameObject>();
		ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		ObjectMapper<Mesh> meshes = new ObjectMapper<Mesh>();
		ObjectMapper<Material> materials = new ObjectMapper<Material>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();
		HashSet<string> resources = new HashSet<string>();

		int getObjectId(GameObject obj){
			return objects.getId(obj);
		}

		int getTextureId(Texture tex){
			return textures.getId(tex);
		}

		int getMeshId(Mesh obj){
			return meshes.getId(obj);
		}

		int getMaterialId(Material obj){
			return materials.getId(obj);
		}

		void registerResource(string path){
			resources.Add(path);
		}

		[System.Serializable]
		public class JsonRendererData{
			//public bool hasRenderer = false;
			public bool receiveShadows = false;
			public string shadowCastingMode;
			public int lightmapIndex = -1;
			public Vector4 lightmapScaleOffset = new Vector4(1.0f, 1.0f, 0.0f, 0.0f);
			public List<int> materials = new List<int>();
		};

		[System.Serializable]
		public class JsonGameObject{
			public string name;
			public int instanceId = -1;
			public int id = -1;
			public Vector3 localPosition = Vector3.zero;
			public Quaternion localRotation = Quaternion.identity;
			public Vector3 localScale = Vector3.one;
			public Matrix4x4 worldMatrix = Matrix4x4.identity;
			public Matrix4x4 localMatrix = Matrix4x4.identity;
			public List<int> children = new List<int>();
			public int parent = -1;
			public int mesh = -1;
			public JsonRendererData[] renderer = null;
			public JsonLight[] light = null;
			public bool isStatic = true;
			public bool lightMapStatic = true;
			public bool occluderStatic = true;
			public bool occludeeStatic = true;
			public bool navigationStatic = true;
			public bool reflectionProbeStatic = true;
			public JsonGameObject(GameObject obj, Exporter exp){
				name = obj.name;
				instanceId = obj.GetInstanceID();
				id = exp.objects.getId(obj);
				localPosition = obj.transform.localPosition;
				localRotation = obj.transform.localRotation;
				localScale = obj.transform.localScale;
				worldMatrix = obj.transform.localToWorldMatrix;
				localMatrix = worldMatrix;

				isStatic = obj.isStatic;
				var flags = GameObjectUtility.GetStaticEditorFlags(obj);
				lightMapStatic = (flags & StaticEditorFlags.LightmapStatic) != 0;
				occluderStatic = (flags & StaticEditorFlags.OccluderStatic) != 0;
				occludeeStatic = (flags & StaticEditorFlags.OccludeeStatic) != 0;
				navigationStatic = (flags & StaticEditorFlags.NavigationStatic) != 0;
				reflectionProbeStatic = (flags & StaticEditorFlags.ReflectionProbeStatic) != 0;

				if (obj.transform.parent){
					localMatrix = obj.transform.parent.worldToLocalMatrix * localMatrix;
				}
				renderer = exp.makeRenderer(obj.GetComponent<Renderer>());
				light = exp.makeLight(obj.GetComponent<Light>());

				mesh = exp.getMeshId(obj);

				foreach(Transform curChild in obj.transform){
					children.Add(exp.objects.getId(curChild.gameObject));
				}
				if (obj.transform.parent)
					parent = exp.objects.findId(obj.transform.parent.gameObject);
			}
		}

		public JsonRendererData[] makeRenderer(Renderer r){
			if (!r)
				return null;
			var result = new JsonRendererData();
			result.receiveShadows = r.receiveShadows;
			result.shadowCastingMode = r.shadowCastingMode.ToString();
			result.lightmapIndex = r.lightmapIndex;
			result.lightmapScaleOffset = r.lightmapScaleOffset;
			foreach(var cur in r.sharedMaterials){
				result.materials.Add(materials.getId(cur));
			}
			return new JsonRendererData[]{result};
		}

		[System.Serializable]
		public class JsonLight{
			public Color color = Color.white;
			public float range = 0.0f;
			public float spotAngle = 0.0f;
			public string type;
			public float shadowStrength = 0.0f;
			public float intensity = 0.0f;
			public string renderMode;
			public string shadows;
			public float bounceIntensity = 0.0f;
		};

		public JsonLight[] makeLight(Light l){
			if (!l)
				return null;
			var result = new JsonLight();
			result.color  = l.color;
			result.range = l.range;
			result.spotAngle = l.spotAngle;
			result.type = l.type.ToString();
			result.renderMode = l.renderMode.ToString();
			result.shadowStrength = l.shadowStrength;
			result.shadows = l.shadows.ToString();
			result.intensity = l.intensity;
			return new JsonLight[]{result};
		}

		public float[] makeVector(Vector2 arg){
			return new float[]{arg.x, arg.y};
		}

		public float[] makeVector(Vector3 arg){
			return new float[]{arg.x, arg.y, arg.z};
		}

		public float[] makeVector(Vector4 arg){
			return new float[]{arg.x, arg.y, arg.z, arg.w};
		}

		int getMeshId(GameObject obj){
			int result = -1;
			var meshFilter = obj.GetComponent<MeshFilter>();
			if (!meshFilter)
				return result;

			var mesh = meshFilter.sharedMesh;
			result = meshes.getId(mesh);

			if (meshMaterials.ContainsKey(mesh))
				return result;
			var r = obj.GetComponent<Renderer>();
			if (r){
				meshMaterials[mesh] = new List<Material>(r.sharedMaterials);
			}
			return result;
		}

		public JsonScene exportObjects(GameObject[] args){
			var result = new JsonScene();
			foreach(var cur in args){
				if (!cur)
					continue;
				objects.getId(cur);
			}
			for(int i = 0; i < objects.objectList.Count; i++){
				result.objects.Add(new JsonGameObject(objects.objectList[i], this));
			}

			foreach(var cur in meshes.objectList){
				result.meshes.Add(new JsonMesh(cur, this));
			}

			foreach(var cur in materials.objectList){
				result.materials.Add(new JsonMaterial(cur, this));
			}

			foreach(var cur in textures.objectList){
				result.textures.Add(new JsonTexture(cur, this));
			}

			result.resources = new List<string>(resources);
			result.resources.Sort();

			return result;
		}

		public JsonScene exportOneObject(GameObject obj){
			return exportObjects(new GameObject[]{obj});
		}
	}

	public class ExportMenu: MonoBehaviour{
		[MenuItem("GameObject/Scene Export/Export selected objects", false, 0)]
		public static void  exportJsonScene(MenuCommand menuCommand){
			if (Selection.transforms.Length <= 0)
				return;

			List<GameObject> objects = new List<GameObject>();
			foreach(var cur in Selection.transforms){
				if (cur == null)
					continue;
				var curObj = cur.gameObject;
				if (!curObj)
					continue;
				objects.Add(curObj);
			}
			if (objects.Count <= 0)
				return;
			
			var path = EditorUtility.SaveFilePanel("Save category config", "", objects[0].name, "json");
			if (path == string.Empty)
				return;
			var exporter = new Exporter();

			var jsonObj = exporter.exportObjects(objects.ToArray());
			Utility.saveStringToFile(path, jsonObj.toJsonString());
		}

		[MenuItem("GameObject/Scene Export/Export current object", false, 0)]
		public static void  exportJsonObject(MenuCommand menuCommand){
			if (Selection.activeObject == null)
				return;
			var obj = Selection.activeGameObject;
			var path = EditorUtility.SaveFilePanel("Save category config", "", obj.name, "json");
			if (path == string.Empty)
				return;
			var exporter = new Exporter();

			var jsonObj = exporter.exportOneObject(obj);
			Utility.saveStringToFile(path, jsonObj.toJsonString());
		}
	}
}

