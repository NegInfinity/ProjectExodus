using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonGameObject: IFastJsonValue{
		public string name;
		public int instanceId = -1;
		public int id = -1;
		public Vector3 localPosition = Vector3.zero;
		public Quaternion localRotation = Quaternion.identity;
		public Vector3 localScale = Vector3.one;
		public Matrix4x4 worldMatrix = Matrix4x4.identity;
		public Matrix4x4 localMatrix = Matrix4x4.identity;
		public List<int> children = new List<int>();
		public int parent = -1;
		public int mesh = -1;
		public JsonRendererData[] renderer = null;
		public JsonLight[] light = null;
		public JsonReflectionProbe[] reflectionProbes = null;
		public bool isStatic = true;
		public bool lightMapStatic = true;
		public bool occluderStatic = true;
		public bool occludeeStatic = true;
		public bool navigationStatic = true;
		public bool reflectionProbeStatic = true;
			
		public bool nameClash = false;
		public string uniqueName = "";
		
		public int prefabRootId = -1;
		public int prefabObjectId = -1;
		public bool prefabInstance = false;
		public bool prefabModelInstance = false;
		public string prefabType = "";
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("instanceId", instanceId);
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("localPosition", localPosition);
			writer.writeKeyVal("localRotation", localRotation);
			writer.writeKeyVal("localScale", localScale);
			writer.writeKeyVal("worldMatrix", worldMatrix);
			writer.writeKeyVal("localMatrix", localMatrix);
			writer.writeKeyVal("children", children);
			writer.writeKeyVal("parent", parent);
			writer.writeKeyVal("mesh", mesh);
			writer.writeKeyVal("isStatic", isStatic);
			writer.writeKeyVal("lightMapStatic", lightMapStatic);
			writer.writeKeyVal("navigationStatic", navigationStatic);
			writer.writeKeyVal("occluderStatic", occluderStatic);
			writer.writeKeyVal("occludeeStatic", occludeeStatic);
			writer.writeKeyVal("reflectionProbeStatic", reflectionProbeStatic);
					
			writer.writeKeyVal("nameClash", nameClash);
			writer.writeKeyVal("uniqueName", uniqueName);
				
			writer.writeKeyVal("prefabRootId", prefabRootId);
			writer.writeKeyVal("prefabObjectId", prefabObjectId);
			writer.writeKeyVal("prefabInstance", prefabInstance);
			writer.writeKeyVal("prefabModelInstance", prefabModelInstance);
			writer.writeKeyVal("prefabType", prefabType);
				
			writer.writeKeyVal("renderer", renderer);
			writer.writeKeyVal("light", light);
			writer.writeKeyVal("reflectionProbes", reflectionProbes);
			
			writer.endObject();
		}
			
		public JsonGameObject(GameObject obj, GameObjectMapper objMap, ResourceMapper resMap){
			name = obj.name;
			instanceId = obj.GetInstanceID();
			id = objMap.getId(obj);
			localPosition = obj.transform.localPosition;
			localRotation = obj.transform.localRotation;
			localScale = obj.transform.localScale;
			worldMatrix = obj.transform.localToWorldMatrix;
			localMatrix = worldMatrix;

			isStatic = obj.isStatic;
			var flags = GameObjectUtility.GetStaticEditorFlags(obj);
			lightMapStatic = (flags & StaticEditorFlags.LightmapStatic) != 0;
			occluderStatic = (flags & StaticEditorFlags.OccluderStatic) != 0;
			occludeeStatic = (flags & StaticEditorFlags.OccludeeStatic) != 0;
			navigationStatic = (flags & StaticEditorFlags.NavigationStatic) != 0;
			reflectionProbeStatic = (flags & StaticEditorFlags.ReflectionProbeStatic) != 0;
			
			var prefType = PrefabUtility.GetPrefabType(obj);
			prefabType = prefType.ToString();
			prefabRootId = resMap.getRootPrefabId(obj, true);
			prefabObjectId = resMap.getPrefabObjectId(obj, true);
			prefabInstance = (prefType == PrefabType.PrefabInstance) || (prefType == PrefabType.ModelPrefabInstance);

			if (obj.transform.parent){
				localMatrix = obj.transform.parent.worldToLocalMatrix * localMatrix;
			}
			renderer = JsonRendererData.makeRendererArray(obj.GetComponent<Renderer>(), resMap);
			light = JsonLight.makeLightArray(obj.GetComponent<Light>());
			reflectionProbes = 
				ExportUtility.convertComponents<ReflectionProbe, JsonReflectionProbe>(obj,
					(c) => new JsonReflectionProbe(c));

			mesh = resMap.getMeshId(obj);

			foreach(Transform curChild in obj.transform){
				var childId = objMap.getId(curChild.gameObject); 
				if (childId < 0){
					//throw new System.ArgumentException("Could not find child id
				}
				//var childId = objMap.getId(curChild.gameObject); 
				children.Add(childId);
			}
			if (obj.transform.parent)
				parent = objMap.findId(obj.transform.parent.gameObject);
		}
	}
}
