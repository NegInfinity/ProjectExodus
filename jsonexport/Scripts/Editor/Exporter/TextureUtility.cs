using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public static class TextureUtility{	
		/*
		Have to do it the hard way, unless we implement tiff reader in C#.
		*/
		public static void saveReadOnlyTexture(string path, Texture2D tex, JsonTexture jsonTex, bool useExr){//Texture2D tex, bool sRGB, bool useExr){
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
	}
}