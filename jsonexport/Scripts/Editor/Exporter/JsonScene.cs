using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	public partial class Exporter{
		[System.Serializable]
		public class JsonScene{
			public List<JsonGameObject> objects = new List<JsonGameObject>();
			public List<JsonMaterial> materials = new List<JsonMaterial>();
			public List<JsonMesh> meshes = new List<JsonMesh>();
			public List<JsonTexture> textures = new List<JsonTexture>();
			public List<string> resources = new List<string>();
			
			static readonly string[] supportedTexExtensions = new string[]{".bmp", ".float", ".pcx", ".png", 
				".psd", ".tga", ".jpg", ".jpeg", ".exr", ".dds", ".hdr"};
			
			bool isSupportedTexExtension(string ext){
				return supportedTexExtensions.Contains(ext);
			}
			
			void copyTexture(JsonTexture jsonTex, string targetDir, string projectDir){
				var texPath = jsonTex.path;
				var ext = System.IO.Path.GetExtension(texPath).ToLower();
				//Debug.LogFormat("Tex {0}, ext {1}", texPath, ext);
				var srcPath = System.IO.Path.Combine(projectDir, texPath);//TODO: The asset can be elswhere.
				
				bool supportedFile = isSupportedTexExtension(ext);
				if (!supportedFile){
					Debug.LogWarningFormat("Unsupported extension: \"{0}\" in texture \"{1}\"\nPNG conversion will be attempted.", ext, texPath);
				}
				bool exists = System.IO.File.Exists(srcPath);
				
				var dstPath = System.IO.Path.Combine(targetDir, texPath);
				var dstDir = System.IO.Path.GetDirectoryName(dstPath);
				System.IO.Directory.CreateDirectory(dstDir);
				
				if (exists){
					if (supportedFile){
						System.IO.File.Copy(srcPath, dstPath, true);
						return;
					}
					else{
						var unsupportedPath = System.IO.Path.Combine(targetDir, "!Unsupported!");
						unsupportedPath = System.IO.Path.Combine(unsupportedPath, texPath);
						System.IO.Directory.CreateDirectory(System.IO.Path.GetDirectoryName(unsupportedPath));
						//System.IO.File.Copy(srcPath, dstPath, true);					
						System.IO.File.Copy(srcPath, unsupportedPath, true);					
					}
				}
				else{
					Debug.LogWarningFormat("Asset {0} not found on disk, attempting recovery from texture data", srcPath);	
				}
				
				bool useExr = false;
				var formatExt = useExr ? ".exr" : ".png";
				Debug.LogWarningFormat("Attempting to write image data in {1} format for: {0}\nData Loss possible.", texPath, formatExt);
				var tex2D = (Texture2D)(jsonTex.textureRef);
				if (!tex2D){
					Debug.LogWarningFormat("Not a 2d texture: {0}", texPath);
					return;
				}
				
				var savePath = System.IO.Path.ChangeExtension(dstPath, formatExt);
				/*
				bool encodeSuccessful = true;
				try{
					var bytes = useExr ? tex2D.EncodeToEXR() : tex2D.EncodeToPNG();
					Utility.saveBytesToFile(savePath, bytes);				
				}
				catch(System.Exception e){
					Debug.LogWarningFormat("Normal saving failed for {0}. Exception message: {1}", jsonTex.path, e.Message);
					encodeSuccessful = false;
				}
				
				if (encodeSuccessful)
					return;*/
				
				saveReadOnlyTexture(savePath, tex2D, jsonTex, useExr);
			}
			
			/*
			bool isCompressedNormalMap(TextureFormat fmt){
				return (fmt == TextureFormat.BC5)
			}
			*/
			
			/*
			Have to do it the hard way, unless we implement tiff reader in C#.
			*/
			void saveReadOnlyTexture(string path, Texture2D tex, JsonTexture jsonTex, bool useExr){//Texture2D tex, bool sRGB, bool useExr){
				bool compressedNormalMap = jsonTex.normalMapFlag && 
					((tex.format == TextureFormat.DXT5) || (tex.format == TextureFormat.DXT1) || (tex.format == TextureFormat.BC5));
				var texFmt = TextureFormat.ARGB32;
				var rendFmt = RenderTextureFormat.ARGB32;
				bool linear = !jsonTex.sRGB;
				RenderTexture tmpRend = RenderTexture.GetTemporary(tex.width, tex.height, 0, 
					rendFmt, linear ? RenderTextureReadWrite.Linear: RenderTextureReadWrite.sRGB);
					
				Graphics.Blit(tex, tmpRend);
				RenderTexture prev = tmpRend;
				RenderTexture.active = tmpRend;
				Texture2D tmpTex2d = new Texture2D(tex.width, tex.height, texFmt, false, linear);
				tmpTex2d.hideFlags = HideFlags.HideAndDontSave;
				tmpTex2d.ReadPixels(new Rect(0, 0, tmpRend.width, tmpRend.height), 0, 0);
				tmpTex2d.Apply();
				
				Debug.LogFormat("Format of processed normalmap : {0}", tex.format);
				if (compressedNormalMap){
					Debug.LogWarningFormat("Compressed normalmap detected: \"{0}\" ({1}). Data loss may occur while saving to png", jsonTex.path, tex.format);
					var pixels = tmpTex2d.GetPixels();
					for(int i = 0; i < pixels.Length; i++){
						var packed = pixels[i];
						float x = packed.r * packed.a;
						float y = packed.g;
						x = x * 2.0f - 1.0f;
						y = y * 2.0f - 1.0f;
						float z = Mathf.Sqrt(1.0f - (x*x + y*y));
						
						pixels[i] = new Color(x*0.5f + 0.5f, y*0.5f + 0.5f, z*0.5f + 0.5f, 1.0f);
					}
					tmpTex2d.SetPixels(pixels);
					tmpTex2d.Apply();
				}
				RenderTexture.active = prev;
				RenderTexture.ReleaseTemporary(tmpRend);
				
				var bytes = useExr ? tmpTex2d.EncodeToEXR(): tmpTex2d.EncodeToPNG();
				if (Application.isEditor && !Application.isPlaying)
					Object.DestroyImmediate(tmpTex2d);
				else
					Object.Destroy(tmpTex2d);
				Utility.saveBytesToFile(path, bytes);
			}
			
			void saveResources(string baseFilename){
				var targetDir = System.IO.Path.GetDirectoryName(System.IO.Path.GetFullPath(baseFilename));
				if (!Application.isEditor){
					throw new System.ArgumentException("The application is not running in editor mode");
				}
				
				var dataPath = System.IO.Path.GetFullPath(Application.dataPath);
				var projectPath = System.IO.Path.GetDirectoryName(dataPath);
				Debug.LogFormat("data Path: {0}", dataPath);
				Debug.LogFormat("projectPath: {0}", projectPath);
				Debug.LogFormat("targetDir: {0}", targetDir);
				
				if (projectPath == targetDir){
					Debug.LogWarningFormat("You're saving into project directory, files will not be copied");
					return;
				}
				
				foreach(var curTex in textures){
					copyTexture(curTex, targetDir, projectPath);
				}
			}
			
			public void saveToFile(string filename, bool saveResourceFiles = false){
				Utility.saveStringToFile(filename, toJsonString());
				if (!saveResourceFiles)
					return;
				saveResources(filename);
			}
			
			public void writeJsonValueBody(FastJsonWriter writer){
				writer.beginKeyArray("objects");
				foreach(var cur in objects){
					cur.writeJsonValue(writer);
				}
				writer.endArray();
				writer.beginKeyArray("materials");
				foreach(var cur in materials){
					cur.writeJsonValue(writer);
				}
				writer.endArray();
				writer.beginKeyArray("meshes");
				foreach(var cur in meshes){
					cur.writeJsonValue(writer);
				}
				writer.endArray();
				writer.beginKeyArray("textures");
				foreach(var cur in textures){
					cur.writeJsonValue(writer);
				}
				writer.endArray();
				writer.beginKeyArray("resources");
				foreach(var cur in resources){
					writer.writeValue(cur);
				}
				writer.endArray();
			}
			
			public void writeJsonValue(FastJsonWriter writer){
				writer.beginObjectValue();
				writeJsonValueBody(writer);
				writer.endObject();
			}

			public string toJsonString(){
				FastJsonWriter writer = new FastJsonWriter();
				writer.beginDocument();
				writeJsonValueBody(writer);
				writer.endDocument();
				return writer.getString();
			}
		}
	}
}