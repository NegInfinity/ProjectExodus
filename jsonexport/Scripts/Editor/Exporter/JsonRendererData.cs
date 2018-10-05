using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public partial class Exporter{
		[System.Serializable]
		public class JsonRendererData{
			//public bool hasRenderer = false;
			public bool receiveShadows = false;
			public string shadowCastingMode;
			public int lightmapIndex = -1;
			public Vector4 lightmapScaleOffset = new Vector4(1.0f, 1.0f, 0.0f, 0.0f);
			public List<int> materials = new List<int>();
			
			public void writeJsonValue(FastJsonWriter writer){
				writer.beginObjectValue();
				writer.writeKeyVal("lightmapIndex", lightmapIndex);
				writer.writeKeyVal("shadowCastingMode", shadowCastingMode);
				writer.writeKeyVal("lightmapScaleOffset", lightmapScaleOffset);
				writer.writeKeyVal("materials", materials);
				writer.writeKeyVal("receiveShadows", receiveShadows);
				writer.endObject();
			}
		};
		
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
	}
}
