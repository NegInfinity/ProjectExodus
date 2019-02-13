using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public static class Utility{
		/*
		Returns node matrix relative to rootNode. root node can be null
		*/
		public static Matrix4x4 getRelativeMatrix(Transform node, Transform rootNode){
			if(!node)
				throw new System.ArgumentNullException("node");
				
			var nodeMatrix = node.localToWorldMatrix;
			if (!rootNode)
				return nodeMatrix;
			var invRootNodeMatrix = rootNode.worldToLocalMatrix;
			return invRootNodeMatrix * nodeMatrix;
		}
		
		public static Matrix4x4 getRelativeInverseMatrix(Transform node, Transform rootNode){
			if (!node)
				throw new System.ArgumentNullException("node");
			var nodeInvMatrix = node.worldToLocalMatrix;
			if (!rootNode)
				return nodeInvMatrix;
			var rootNodeMatrix = rootNode.localToWorldMatrix;
			return nodeInvMatrix * rootNodeMatrix;
		}
		
		public static T getSrcPrefabAssetObject<T>(T src, bool returnNullIfNotFound) where T: Object{
			if (!src)
				throw new System.ArgumentNullException("src");
			
			var linked = PrefabUtility.GetCorrespondingObjectFromSource(src) as T;
			if (!linked){
				if (returnNullIfNotFound)
					return null;
				return src;
			}
			return linked;
		}

		public static GameObject getPrefabInstanceRoot(GameObject obj){
			return PrefabUtility.FindPrefabRoot(obj);			
		}

		public static bool isPrefabModelInstance(Object obj){
			var prefType = PrefabUtility.GetPrefabType(obj);
			return (prefType == PrefabType.ModelPrefabInstance);
		}

		public static bool isPrefabInstance(Object obj){
			var prefType = PrefabUtility.GetPrefabType(obj);
			return (prefType == PrefabType.PrefabInstance);
		}

		public static bool isTrackablePrefab(Object obj){
			var prefType = PrefabUtility.GetPrefabType(obj);
			return (prefType == PrefabType.ModelPrefab) || (prefType == PrefabType.Prefab);
		}
		
		public static Vector4 getIdxVector4(this float[] floats, int vertIndex){
			var baseOffset = vertIndex * 4;
			return new Vector4(
				floats[baseOffset], floats[baseOffset + 1], floats[baseOffset + 2], floats[baseOffset + 3]);
		}
		
		public static void setIdxVector4(this float[] floats, int vertIndex, Vector4 newVal){
			var baseOffset = vertIndex * 4;
			floats[baseOffset] = newVal.x;
			floats[baseOffset + 1] = newVal.y;
			floats[baseOffset + 2] = newVal.z;
			floats[baseOffset + 3] = newVal.w;
		}
		
		public static Vector3 getIdxVector3(this float[] floats, int vertIndex){
			var baseOffset = vertIndex * 3;
			return new Vector3(floats[baseOffset], floats[baseOffset + 1], floats[baseOffset + 2]);
		}
		
		public static void setIdxVector3(this float[] floats, int vertIndex, Vector3 newVal){
			var baseOffset = vertIndex * 3;
			floats[baseOffset] = newVal.x;
			floats[baseOffset + 1] = newVal.y;
			floats[baseOffset + 2] = newVal.z;
		}
		
		public delegate Data IndexedDataProcessorCallback<Data>(Data arg, int argIndex);
		
		/*
		We need to fold it into one generic function, at some point.
		*/
		public static void processFloats2(this float[] floats, IndexedDataProcessorCallback<Vector2> callback){
			if (floats == null)
				return;
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			for(int offset = 0, index = 0; offset < (floats.Length - 1); offset += 2, index++){
				var src = new Vector2(floats[offset], floats[offset + 1]);
				var dst = callback(src, index);
				floats[offset + 0] = dst.x;
				floats[offset + 1] = dst.y;
			}
		} 
		
		public static void processFloats3(this float[] floats, IndexedDataProcessorCallback<Vector3> callback){
			if (floats == null)
				return;
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			for(int offset = 0, index = 0; offset < (floats.Length - 2); offset += 3, index++){
				var src = new Vector3(floats[offset], floats[offset + 1], floats[offset + 2]);
				var dst = callback(src, index);
				floats[offset + 0] = dst.x;
				floats[offset + 1] = dst.y;
				floats[offset + 2] = dst.z;				
			}
		} 
		
		public static void processFloats4(this float[] floats, IndexedDataProcessorCallback<Vector4> callback){
			if (floats == null)
				return;
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			for(int offset = 0, index = 0; offset < (floats.Length - 3); offset += 4, index++){
				var src = new Vector4(floats[offset], floats[offset + 1], floats[offset + 2], floats[offset + 3]);
				
				var dst = callback(src, index);
				
				floats[offset + 0] = dst.x;
				floats[offset + 1] = dst.y;
				floats[offset + 2] = dst.z;				
				floats[offset + 3] = dst.w;
			}
		} 
		
		public static Vector2 getIdxVector2(this float[] floats, int vertIndex){
			var baseOffset = vertIndex * 2;
			return new Vector3(floats[baseOffset], floats[baseOffset + 1]);
		}
		
		public static void setIdxVector2(this float[] floats, int vertIndex, Vector2 newVal){
			var baseOffset = vertIndex * 2;
			floats[baseOffset] = newVal.x;
			floats[baseOffset + 1] = newVal.y;
		}
			
		public static bool isNullOrEmpty<T>(this T[] arg){
			return (arg == null) || (arg.Length == 0);
		}
		
		public static Transform findChildByName(this Transform root, string name){
			if (!root)
				throw new System.ArgumentNullException("root");
				
			if (root.name == name)
				return root;
			foreach(Transform child in root){
				if (!child)
					continue;
				var childResult = findChildByName(child, name);
				if (childResult)
					return childResult;					
			}
				
			return null;
		}
		
		public static Transform findChildByPath(this Transform root, string path, Transform relativeTo = null){
			if (!root)
				throw new System.ArgumentNullException("root");
				
			if (root.getScenePath(relativeTo) == path)
				return root;
			foreach(Transform child in root){
				if (!child)
					continue;
				var childResult = findChildByPath(child, path, relativeTo);
				if (childResult)
					return childResult;					
			}
				
			return null;
		}
		
		static public List<Transform> findTransformsByPath<Container>(Container names, Transform root, Transform relativeTo = null) where Container: ICollection<string>{
			/*
			I can collapse it all into shorter linq queries, but...
			*/
			if (!root)
				throw new System.ArgumentNullException("root");
			var result = new List<Transform>();
			foreach(var curName in names){
				var curChild = findChildByPath(root, curName, relativeTo);
				if (!curChild)
					Debug.LogWarningFormat(
						string.Format("Could not locate child \"{0}\" by name in transform root \"{1}\"", curName, root)
					);
						
				result.Add(curChild);
			}
			
			return result;
		}
		
		static public List<Transform> findNamedTransforms<Container>(Container names, Transform root) where Container: ICollection<string>{
			if (!root)
				throw new System.ArgumentNullException("root");
			var result = new List<Transform>();
			foreach(var curName in names){
				var curChild = findChildByName(root, curName);
				if (!curChild)
					Debug.LogWarningFormat(
						string.Format("Could not locate child \"{0}\" by name in transform root \"{1}\"", curName, root)
					);
						
				result.Add(curChild);
			}
			
			return result;
		}
		/*
		public static Value getValueGenerate<Key, Value, Dict>(this Dict dict, Key key, System.Func<Value> generator)
				where Dict: IDictionary<Key, Value>{*/ //Bah
		public static Value getValueGenerate<Key, Value>(this IDictionary<Key, Value> dict, Key key, System.Func<Value> generator){
			Value result;
			if (dict.TryGetValue(key, out result))
				return result;
			return generator();
		}
		
		public static Value getValueSetGenerate<Key, Value>(this IDictionary<Key, Value> dict, Key key, System.Func<Value> generator){
			Value result;
			if (dict.TryGetValue(key, out result))
				return result;
			var newVal = generator();
			dict.Add(key, newVal);
			return newVal;
		}
	
		public static string toJsonString<T>(this T arg) where T: IFastJsonValue{
			if (arg == null)
				return string.Empty;
			var writer = new FastJsonWriter();
			arg.writeRawJsonValue(writer);
			return writer.getString();
		}
		
		public static void saveToJsonFile<T>(this T arg, string filename) where T: IFastJsonValue{
			saveStringToFile(filename, arg.toJsonString());
		}
		
		public static Matrix4x4 toMatrix(this Quaternion v){
			Matrix4x4 result = Matrix4x4.identity;
			result.SetTRS(Vector3.zero, v, Vector3.one);
			return result;
		}

		public static void saveStringToFile(string filename, string data){
			System.IO.File.WriteAllText(filename, data, System.Text.Encoding.UTF8);
		}

		public static void saveBytesToFile(string filename, byte[] data){
			System.IO.File.WriteAllBytes(filename, data);
		}
		
		public static int beginHash(){
			return 17;
		}

		public static int combineHash<T>(int prevHash, T obj){
			int nextHash = 0;
			if ((object)obj != null)
				nextHash = obj.GetHashCode();
			return prevHash * 23 + nextHash;
		}
		
		public static bool isInProjectPath(string path){
			var targetDir = System.IO.Path.GetDirectoryName(System.IO.Path.GetFullPath(path));
			var projectPath = getProjectPath();
			return projectPath == targetDir;
		}
		
		public static string getProjectPath(){
			if (!Application.isEditor){
				throw new System.ArgumentException("The application is not running in editor mode");
			}
				
			var dataPath = System.IO.Path.GetFullPath(Application.dataPath);
			var projectPath = System.IO.Path.GetDirectoryName(dataPath);
			return projectPath;
		}
		
		static public Vector3[] copyArray(Vector3[] arg){
			int size = 0;
			if (arg != null)
				size = arg.Length;
			var result = new Vector3[size];
			for(int i = 0; i < size; i++)
				result[i] = arg[i];
			return result;
		}

		static public int[] copyArray(int[] arg){
			int size = 0;
			if (arg != null)
				size = arg.Length;
			var result = new int[size];
			for(int i = 0; i < size; i++)
				result[i] = arg[i];
			return result;
		}
	}
}