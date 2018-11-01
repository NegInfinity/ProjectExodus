using UnityEngine;
using UnityEditor;

namespace SceneExport{
	public static class TerrainUtility{
		//pixels one after another. Data is [y, x]
		static void writeFloat2dBin(string filename, int w, int h, float[,] data){
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(filename, System.IO.FileMode.Create))){
				writeFloat2dBin(writer, w, h, data);
			}		
		}
		
		static void writeFloat2dBin(System.IO.BinaryWriter writer, int w, int h, float[,] data){
			for(var y = 0; y < h; y++){
				for(var x = 0; x < w; x++){
					writer.Write(data[y, x]);
				}
			}
		}
		
		static void writeInt2dBin(string filename, int w, int h, int[,] data){
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(filename, System.IO.FileMode.Create))){
				writeInt2dBin(writer, w, h, data);
			}		
		}
		
		static void writeInt2dBin(System.IO.BinaryWriter writer, int w, int h, int[,] data){
			for(var y = 0; y < h; y++){
				for(var x = 0; x < w; x++){
					writer.Write(data[y, x]);
				}
			}
		}	
		
		//layers one after another. data is [y, x, layer]
		static void writeFloat3dPackedBin(string filename, int w, int h, int numLayers, float[,,] data){
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(filename, System.IO.FileMode.Create))){
				writeFloat3dPackedBin(writer, w, h, numLayers, data);
			}		
		}
		
		//layers one after another. data is [y, x, layer]
		static void writeFloat3dPackedBin(System.IO.BinaryWriter writer, int w, int h, int numLayers, float[,,] data){
			for(var y = 0; y < h; y++){
				for(var x = 0; x < w; x++){
					for(var alpha = 0; alpha < numLayers; alpha++){
						writer.Write(data[y, x, alpha]);
					}
				}
			}
		}
		
		//just one slice. data is [y, x, layer]
		static void writeFloat3dSliceBin(string filename, int w, int h, int layer, float[,,] data){
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(filename, System.IO.FileMode.Create))){
				writeFloat3dSliceBin(writer, w, h, layer, data);
			}		
		}
		
		//just one slice. data is [y, x, layer]
		static void writeFloat3dSliceBin(System.IO.BinaryWriter writer, int w, int h, int layer, float[,,] data){
			for(var y = 0; y < h; y++){
				for(var x = 0; x < w; x++){
					writer.Write(data[y, x, layer]);
				}
			}
		}
		
		static void saveFloat2dAsRawUint16(string filename, int w, int h, float[,] data, bool transpose){
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(filename, System.IO.FileMode.Create))){
				if (!transpose){
					for(int y = 0; y < h; y++){
						for(int x = 0; x < w; x++){
							var src = data[y,x];
							var f = Mathf.Clamp01(src);
							//var i = Mathf.RoundToInt(f * (float)0xFFFF);
							var i = Mathf.FloorToInt(f * (float)0xFFFF);
							var outData = (System.UInt16)i;
							writer.Write(outData);
						}
					}
				}
				else{
					for(int x = 0; x < w; x++){
						for(int y = 0; y < h; y++){
							var src = data[y,x];
							var f = Mathf.Clamp01(src);
							//var i = Mathf.RoundToInt(f * (float)0xFFFF);
							var i = Mathf.FloorToInt(f * (float)0xFFFF);
							var outData = (System.UInt16)i;
							writer.Write(outData);
						}
					}
				}
			}
		}
		
		static void saveFloat3dSliceAsRawUint8(string filename, int w, int h, int level, float[,,] data, bool transpose){
			using(var writer = new System.IO.BinaryWriter(
					System.IO.File.Open(filename, System.IO.FileMode.Create))){
				if (!transpose){
					for(int y = 0; y < h; y++){
						for(int x = 0; x < w; x++){
							var src = data[y,x, level];
							var f = Mathf.Clamp01(src);
							//var i = Mathf.RoundToInt(f * (float)0xFFFF);
							var i = Mathf.FloorToInt(f * (float)0xFF);
							var outData = (System.Byte)i;
							writer.Write(outData);
						}
					}
				}
				else{
					for(int x = 0; x < w; x++){
						for(int y = 0; y < h; y++){
							var src = data[y,x, level];
							var f = Mathf.Clamp01(src);
							//var i = Mathf.RoundToInt(f * (float)0xFFFF);
							var i = Mathf.FloorToInt(f * (float)0xFF);
							var outData = (System.Byte)i;
							writer.Write(outData);
						}
					}
				}
			}
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
			
			//binary map - all stuff combined together.
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
				writeFloat2dBin(writer, hMapW, hMapH, heightData);
				
				//splats
				for(int alpha = 0; alpha < numAlphas; alpha++){
					writeFloat3dSliceBin(writer, alphaW, alphaH, alpha, alphaData);
				}
				
				//detail layers
				for(var detLayer = 0; detLayer < numDetailLayers; detLayer++){
					var detailData = terData.GetDetailLayer(0, 0, detailW, detailH, detLayer);
					writeInt2dBin(writer, detailW, detailH, detailData);
				}
			}
			
			//only height
			writeFloat2dBin(heightPath, hMapW, hMapH, heightData);
			saveFloat2dAsRawUint16(System.IO.Path.Combine(targetDir, 
				curTerrain.heightMapRawPath), hMapW, hMapH, heightData, true
			);
			
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
				writeFloat3dSliceBin(curAlphaPath, alphaW, alphaH, alphaIndex, alphaData);
				
				saveFloat3dSliceAsRawUint8(System.IO.Path.Combine(
						targetDir, curTerrain.alphaMapRawPaths[alphaIndex]
					), alphaW, alphaH, alphaIndex, alphaData, true
				);
				
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
				writeInt2dBin(curDetailPath, detailW, detailH, detailData);
				writeInt2dBin(System.IO.Path.Combine(targetDir, curTerrain.detailMapRawPaths[detLayer]), 
					detailW, detailH, detailData);
				
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