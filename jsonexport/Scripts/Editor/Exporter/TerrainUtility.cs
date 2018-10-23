using UnityEngine;
using UnityEditor;

namespace SceneExport{
	public static class TerrainUtility{
		[System.Serializable]
		class TerrainDataSaver{
			
		}
		
		public static void saveTerrain(JsonTerrainData curTerrain, string targetDir, string projectPath, bool savePngs, Logger logger = null){		
			Logger.makeValid(ref logger);
			//logger.logFormat("Saving terrain {0}, {1}, {2}", targetDir, projectPath, curTerrain);
			if (curTerrain == null){
				return;
			}
			
			var targetPath = System.IO.Path.Combine(targetDir, curTerrain.exportPath);//
			
			var dstDir = System.IO.Path.GetDirectoryName(targetPath);
			System.IO.Directory.CreateDirectory(dstDir);
			
			var heightPath = System.IO.Path.ChangeExtension(targetPath, ".height");
			//logger.logFormat("Saving to \"{0}\", \"{1}\"", targetPath, heightPath);
			
			var terData = curTerrain.terrainData;
			int hMapW = terData.heightmapWidth;
			int hMapH = terData.heightmapHeight;				
			var numAlphas = terData.alphamapLayers;
			//logger.logFormat("w: {0}; h: {1}; alphas: {2}", w, h, numAlphas);			
			var heightData = terData.GetHeights(0, 0, hMapW, hMapH);
			
			var alphaW = terData.alphamapWidth;
			var alphaH = terData.alphamapHeight;
			var alphaData = terData.GetAlphamaps(0, 0, alphaW, alphaH);
			
			var detailW = terData.detailWidth;
			var detailH = terData.detailHeight;
			var detailPrototypes = terData.detailPrototypes;
			var numDetailLayers = detailPrototypes.Length;
			//var numDetailLayers = terData.detailResolution;
			
			//binary map
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(targetPath, System.IO.FileMode.Create))){
				writer.Write(hMapW);
				writer.Write(hMapH);
				writer.Write(alphaW);
				writer.Write(alphaH);
				writer.Write(numAlphas);
				writer.Write(detailW);
				writer.Write(detailH);
				writer.Write(numDetailLayers);
				//heightmap
				for(var y = 0; y < hMapH; y++){
					for(var x = 0; x < hMapW; x++){
						writer.Write(heightData[y, x]);
					}
				}
				
				//splats
				for(var y = 0; y < alphaH; y++){
					for(var x = 0; x < alphaW; x++){
						for(var alpha = 0; alpha < numAlphas; alpha++){
							writer.Write(alphaData[y, x, alpha]);
						}
					}
				}
				
				//detail layers
				for(var detLayer = 0; detLayer < numDetailLayers; detLayer++){
					var detailData = terData.GetDetailLayer(0, 0, detailW, detailH, detLayer);
					for(var y = 0; y < detailH; y++){
						for(var x = 0; x < detailW; x++){
							//those are ints? Apparently within 0..16 range? Sigh.
							writer.Write(detailData[y, x]);
						}
					}
				}
			}
			
			//only height
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(heightPath, System.IO.FileMode.Create))){
				//writer.Write(w);
				//writer.Write(h);
				for(var y = 0; y < hMapH; y++){
					for(var x = 0; x < hMapW; x++){
						writer.Write(heightData[y, x]);
					}
				}
			}
			
			/*
			png height, debugging
			*/
			if (savePngs){
				var pixels = new Color[hMapW * hMapH];
				for(var y = 0; y < hMapH; y++){
					var rowOffset = y * hMapH;
					for(var x = 0; x < hMapW; x++){
						var f = heightData[y, x];
						pixels[x + rowOffset] = new Color(f, f, f, 1.0f);
					}
				}
				var pngPath = heightPath + ".png";
				TextureUtility.saveRawColorsToPng(pngPath, hMapW, hMapH, pixels, true);
			}
			
			//split splatmaps
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
				
				//png splat
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
			
		
			for(var detLayer = 0; detLayer < numDetailLayers; detLayer++){
				var detailData = terData.GetDetailLayer(0, 0, detailW, detailH, detLayer);
				var ext = string.Format(".detail{0}", detLayer);
				var curDetailPath = System.IO.Path.ChangeExtension(targetPath, ext);
				using(var writer = new System.IO.BinaryWriter(
						System.IO.File.Open(curDetailPath, System.IO.FileMode.Create))){
				
					for(var y = 0; y < detailH; y++){
						for(var x = 0; x < detailW; x++){
							//those are ints? Apparently within 0..16 range? Sigh.
							writer.Write(detailData[y, x]);
						}
					}
				}
				
				if (savePngs){
					var pngPath = curDetailPath + ".png";
					var pixels = new Color[detailW * detailH];
					for(var y = 0; y < detailH; y++){
						var rowOffset = y * detailW;
						for(var x = 0; x < detailW; x++){
							//those are ints? Apparently within 0..16 range? Sigh.
							float val = (float)detailData[y, x]/(float)15.0f;
							var c = new Color(val, val, val);
							pixels[x + rowOffset] = c;
						}
					}
					
					TextureUtility.saveRawColorsToPng(pngPath, detailW, detailH, pixels, true);			
				}
			}
		}
	}
}