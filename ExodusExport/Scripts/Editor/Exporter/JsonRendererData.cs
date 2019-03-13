using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonRendererData: IFastJsonValue{
		//public bool hasRenderer = false;
		public bool receiveShadows = false;
		public string shadowCastingMode;
		public int lightmapIndex = -1;
		public Vector4 lightmapScaleOffset = new Vector4(1.0f, 1.0f, 0.0f, 0.0f);
		public List<ResId> materials = new List<ResId>();
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("lightmapIndex", lightmapIndex);
			writer.writeKeyVal("shadowCastingMode", shadowCastingMode);
			writer.writeKeyVal("lightmapScaleOffset", lightmapScaleOffset);
			writer.writeKeyVal("materials", materials);
			writer.writeKeyVal("receiveShadows", receiveShadows);
			writer.endObject();
		}

		public static JsonRendererData[] makeRendererArray(Renderer r, ResourceMapper resMap){
			if (!r)
				return null;
			var result = new JsonRendererData(r, resMap);
			return new JsonRendererData[]{result};
		}
			
		public JsonRendererData(Renderer r, ResourceMapper resMap){
			receiveShadows = r.receiveShadows;
			shadowCastingMode = r.shadowCastingMode.ToString();
			lightmapIndex = r.lightmapIndex;
			lightmapScaleOffset = r.lightmapScaleOffset;
			foreach(var cur in r.sharedMaterials){
				materials.Add(resMap.getMaterialId(cur));
			}
		}
	};
}
