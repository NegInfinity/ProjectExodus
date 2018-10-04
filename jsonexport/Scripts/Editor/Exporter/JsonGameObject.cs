using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public partial class Exporter{
		[System.Serializable]
		public class JsonGameObject{
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
			public bool isStatic = true;
			public bool lightMapStatic = true;
			public bool occluderStatic = true;
			public bool occludeeStatic = true;
			public bool navigationStatic = true;
			public bool reflectionProbeStatic = true;
			public JsonGameObject(GameObject obj, Exporter exp){
				name = obj.name;
				instanceId = obj.GetInstanceID();
				id = exp.objects.getId(obj);
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

				if (obj.transform.parent){
					localMatrix = obj.transform.parent.worldToLocalMatrix * localMatrix;
				}
				renderer = exp.makeRenderer(obj.GetComponent<Renderer>());
				light = exp.makeLight(obj.GetComponent<Light>());

				mesh = exp.getMeshId(obj);

				foreach(Transform curChild in obj.transform){
					children.Add(exp.objects.getId(curChild.gameObject));
				}
				if (obj.transform.parent)
					parent = exp.objects.findId(obj.transform.parent.gameObject);
			}
		}
	}
}