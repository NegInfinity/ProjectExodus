using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonAudioClip: IFastJsonValue{
		public AudioClip audioClip = null;
		public string name;
		public ResId id = ResId.invalid;
		public string assetPath;
		public string exportPath;
		void writeClipData(FastJsonWriter writer){
			bool initialized = audioClip != null;
			writer.writeKeyVal("initialized", initialized);
			if (!audioClip)
				return;

			writer.writeKeyVal("name", name);
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("assetPath", assetPath);
			writer.writeKeyVal("exportPath", exportPath);
							
			writer.writeKeyVal("ambisonic", audioClip.ambisonic);
			writer.writeKeyVal("channels", audioClip.channels);
			writer.writeKeyVal("frequency", audioClip.frequency);
			writer.writeKeyVal("length", audioClip.length);
			writer.writeKeyVal("loadInBackground", audioClip.loadInBackground);
			writer.writeKeyVal("loadType", audioClip.loadType.ToString());
			writer.writeKeyVal("preloadAudioData", audioClip.preloadAudioData);
			writer.writeKeyVal("samples", audioClip.samples);
		}
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writeClipData(writer);
			writer.endObject();
		}
		
		public JsonAudioClip(AudioClip clip_, ResourceMapper resMap){
			audioClip = clip_;
			if (!audioClip)
				return;
			name = audioClip.name;
			id = resMap.getAudioClipId(audioClip);
			assetPath = AssetDatabase.GetAssetPath(audioClip);
		}
		
		public JsonAudioClip(){
		}
	}
}