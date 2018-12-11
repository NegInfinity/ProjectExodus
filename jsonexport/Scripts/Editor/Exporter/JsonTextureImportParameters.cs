using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonTextureImportParameters: IFastJsonValue{
		public bool initialized = false;
		
		public TextureImporter importer = null;
		public List<JsonSpriteMetaData> spritesheet = new List<JsonSpriteMetaData>();
		
		void writeImporterData(FastJsonWriter writer){
			if (!importer)
				return;
			writer.writeKeyVal("assetPath", importer.assetPath);
			writer.writeKeyVal("borderMipmap", importer.borderMipmap);
			writer.writeKeyVal("compressionQuality", importer.compressionQuality);
			writer.writeKeyVal("convertToNormalmap", importer.convertToNormalmap);
			writer.writeKeyVal("crunchedCompression", importer.crunchedCompression);
			writer.writeKeyVal("fadeout", importer.fadeout);
			writer.writeKeyVal("filterMode", importer.filterMode.ToString());
			writer.writeKeyVal("generateCubemap", importer.generateCubemap.ToString());
			writer.writeKeyVal("importSettingsMissing", importer.importSettingsMissing);
			writer.writeKeyVal("isReadable", importer.isReadable);
			writer.writeKeyVal("maxTextureSize", importer.maxTextureSize);
			writer.writeKeyVal("mipmapFadeDistanceEnd", importer.mipmapFadeDistanceEnd);
			writer.writeKeyVal("mipmapFadeDistanceStart", importer.mipmapFadeDistanceStart);
			writer.writeKeyVal("mipmapFilter", importer.mipmapFilter.ToString());
			writer.writeKeyVal("mipMapsPreserveCoverage", importer.mipMapsPreserveCoverage);
			writer.writeKeyVal("heightmapScale", importer.heightmapScale);
			writer.writeKeyVal("npotScale", importer.npotScale.ToString());
			writer.writeKeyVal("qualifiesForSpritePacking", importer.qualifiesForSpritePacking);
			writer.writeKeyVal("spriteBorder", importer.spriteBorder);
			writer.writeKeyVal("spriteImportMode", importer.spriteImportMode.ToString());
			writer.writeKeyVal("spritePackingTag", importer.spritePackingTag);
			writer.writeKeyVal("spritePivot", importer.spritePivot);
			writer.writeKeyVal("spritePixelsPerUnit", importer.spritePixelsPerUnit);
			
			writer.writeKeyVal("spritesheet", spritesheet);
			//importer.spritesheet
			writer.writeKeyVal("sRGBTexture", importer.sRGBTexture);
			writer.writeKeyVal("streamingMipmaps", importer.streamingMipmaps);
			writer.writeKeyVal("streamingMipmapsPriority", importer.streamingMipmapsPriority);
			writer.writeKeyVal("textureCompression", importer.textureCompression.ToString());
			writer.writeKeyVal("textureType", importer.textureType.ToString());
			writer.writeKeyVal("textureShape", importer.textureShape.ToString());
			writer.writeKeyVal("userData", importer.userData);
			writer.writeKeyVal("wrapMode", importer.wrapMode.ToString());
			writer.writeKeyVal("wrapModeU", importer.wrapModeU.ToString());
			writer.writeKeyVal("wrapModeV", importer.wrapModeV.ToString());
			writer.writeKeyVal("wrapModeW", importer.wrapModeW.ToString());
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("initialized", initialized);
			writeImporterData(writer);			
			writer.endObject();
		}
		
		public JsonTextureImportParameters(){
			initialized = false;
		}
		
		public JsonTextureImportParameters(TextureImporter importer_){
			importer = importer_;
			if (!importer)
				return;
			initialized = true;
			foreach(var cur in importer.spritesheet){
				spritesheet.Add(new JsonSpriteMetaData(cur));
			}
		}
	}
}