using UnityEngine;
using UnityEditor;

namespace SceneExport{
	public static class TerrainUtility{
		public static void saveTerrain(JsonTerrainData curTerrain, string targetDir, string projectPath, bool savePngs, Logger logger = null){		
			Logger.makeValid(ref logger);
			logger.logFormat("Saving terrain {0}, {1}, {2}", targetDir, projectPath, curTerrain);
			if (curTerrain == null){
				return;
			}
			
			var targetPath = System.IO.Path.Combine(targetDir, curTerrain.exportPath);//
			
			var dstDir = System.IO.Path.GetDirectoryName(targetPath);
			System.IO.Directory.CreateDirectory(dstDir);
			
			var heightPath = System.IO.Path.ChangeExtension(targetPath, ".height");
			logger.logFormat("Saving to \"{0}\", \"{1}\"", targetPath, heightPath);
			
			var terData = curTerrain.terrainData;
			int w = terData.heightmapWidth;
			int h = terData.heightmapHeight;				
			var numAlphas = terData.alphamapLayers;
			logger.logFormat("w: {0}; h: {1}; alphas: {2}", w, h, numAlphas);
			
			var heights = terData.GetHeights(0, 0, w, h);
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(heightPath, System.IO.FileMode.Create))){
				//writer.Write(w);
				//writer.Write(h);
				for(var y = 0; y < h; y++){
					for(var x = 0; x < w; x++){
						writer.Write(heights[y, x]);
					}
				}
			}
			
			/*
			png height, debugging
			*/
			if (savePngs){
				var pixels = new Color[w * h];
				for(var y = 0; y < h; y++){
					var rowOffset = y * h;
					for(var x = 0; x < w; x++){
						var f = heights[y, x];
						pixels[x + rowOffset] = new Color(f, f, f, 1.0f);
					}
				}
				var pngPath = heightPath + ".png";
				TextureUtility.saveRawColorsToPng(pngPath, w, h, pixels, true);
			}
			
			var alphaW = terData.alphamapWidth;
			var alphaH = terData.alphamapHeight;
			
			var alphaData = terData.GetAlphamaps(0, 0, alphaW, alphaH);
			for(var alphaIndex = 0; alphaIndex < numAlphas; alphaIndex++){			
				var ext = string.Format(".alpha{0}", alphaIndex);
				var curAlphaPath = System.IO.Path.ChangeExtension(targetPath, ext);
				using(var writer = new System.IO.BinaryWriter(
						System.IO.File.Open(curAlphaPath, System.IO.FileMode.Create))){
					for(var y = 0; y < alphaH; y++){
						for(var x = 0; x < alphaW; x++){
							writer.Write(alphaData[y, x, alphaIndex]);
						}
					}
				}
				
				if (savePngs){
					var pngPath = curAlphaPath + ".png";
					var pixels = new Color[alphaW * alphaH];
					for(var y = 0; y < alphaH; y++){
						var rowOffset = y * alphaH;
						for(var x = 0; x < alphaW; x++){
							var f = alphaData[y, x, alphaIndex];
							pixels[x + rowOffset] = new Color(f, f, f, 1.0f);
						}
					}
					TextureUtility.saveRawColorsToPng(pngPath, alphaW, alphaH, pixels, true);				
				}
			}
		}
	}
}