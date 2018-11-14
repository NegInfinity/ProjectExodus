using UnityEngine;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	public static class TextureUtility{	
		static readonly string[] supportedTexExtensions = new string[]{".bmp", ".float", ".pcx", ".png", 
			".psd", ".tga", ".jpg", ".jpeg", ".exr", ".dds", ".hdr"};
		
		static bool isSupportedTexExtension(string ext){
			return supportedTexExtensions.Contains(ext.ToLower());
		}
		
		static void destroyEditorObject(Object obj){
			if (Application.isEditor && !Application.isPlaying)
				Object.DestroyImmediate(obj);
			else
				Object.Destroy(obj);
		}
		
		public class EditorObjectGuard<T>: System.IDisposable where T: Object{
			public T obj;
			public void Dispose(){
				destroyEditorObject(obj);
			}
			public EditorObjectGuard(T obj_){
				obj = obj_;
			}
		}
		
		public static EditorObjectGuard<T> makeGuard<T>(T obj) where T: Object{
			return new EditorObjectGuard<T>(obj);
		}
		
		/*
		Warning: Uses unity texture creation functions
		*/
		public static void saveRawColorsToPng(string filePath, int width, int height, Color[] colors, bool linear){
			var fmt = TextureFormat.ARGB32;;
			using(var guard = makeGuard(new Texture2D(width, height, fmt, false, linear))){
				var tex = guard.obj;
				tex.SetPixels(colors);
				tex.Apply();
				var bytes = tex.EncodeToPNG();
				System.IO.File.WriteAllBytes(filePath, bytes);
			}
		}
		
		public static void copyTexture(JsonTexture jsonTex, string targetDir, string projectDir, Logger logger = null){
			logger = Logger.getValid(logger);
			var texPath = jsonTex.path;
			var ext = System.IO.Path.GetExtension(texPath).ToLower();
			var srcPath = System.IO.Path.Combine(projectDir, texPath);//TODO: The asset can be elswhere.
				
			bool supportedFile = isSupportedTexExtension(ext);
			if (!supportedFile){
				logger.logWarningFormat(
					"Import of \"{0}\" format used by \"{1}\" is not supported.\n" + 
					"Attempting conversion to \"*.png\"\n" + 
					"Target material will be remapped to use converted texture.", ext, texPath);
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
				logger.logWarningFormat("Asset \"{0}\" is not found on disk, attempting recovery from texture data.", srcPath);	
			}
				
			bool useExr = false;
			var formatExt = useExr ? ".exr" : ".png";
			logger.logWarningFormat("Attempting to write image data in {1} format for: {0}\nData Loss possible.", texPath, formatExt);
			var tex2D = (Texture2D)(jsonTex.textureRef);
			if (!tex2D){
				logger.logWarningFormat("Not a 2d texture: \"{0}\". Texture not saved.", texPath);
				return;
			}
				
			var savePath = System.IO.Path.ChangeExtension(dstPath, formatExt);
			
			TextureUtility.saveReadOnlyTexture(savePath, tex2D, jsonTex, useExr);
		}
	
		/*
		Have to do it the hard way, unless we implement tiff reader in C#.
		*/
		public static void saveReadOnlyTexture(string path, Texture2D tex, JsonTexture jsonTex, bool useExr, Logger logger = null){
			logger = Logger.getValid(logger);
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
				
			logger.logFormat("Format of processed normalmap : {0}", tex.format);
			if (compressedNormalMap){
				logger.logWarningFormat("Compressed normalmap detected: \"{0}\" ({1}).\n" + 
					"Texture conversion will be performed.\n" + 
					"Data loss may occur while saving to png.", jsonTex.path, tex.format);
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
	}
}
