using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class ObjectMapper<Resource>{
		public delegate void OnNewObjectAdded(Resource res);
		//public int nextId = 0;
		public Dictionary<Resource, int> objectMap = new Dictionary<Resource, int>();
		public List<Resource> objectList = new List<Resource>();

		public bool isValidObjectId(int id){
			return (id >= 0) && (id < objectList.Count);
		}
		
		public int numObjects{
			get{
				return objectList.Count;
			}
		}
		
		public Resource getObject(int id){
			if (!isValidObjectId(id))
				throw new System.ArgumentException(string.Format("Invalid object id {0}", id));
			return objectList[id];
		}
		
		public bool hasObject(Resource obj){
			return ExportUtility.isValidId(getId(obj, false));
		}

		public bool isValidId(int id){
			return (id >= 0) && (id < objectList.Count);
		}
			
		public int findId(Resource obj){
			return getId(obj, false, null);
		}
		
		public int getId(Resource obj){
			return getId(obj, true, null);
		}
		
		protected bool isValidObject(Resource obj){
			return !Object.Equals(obj, null);
		}
		
		protected int addNewObjectInternal(Resource obj, OnNewObjectAdded onAddCallback = null){
			if (objectMap.ContainsKey(obj))
				throw new System.ArgumentException("Logic error: duplicate registertation");
			var result = objectList.Count;
			objectMap.Add(obj, result);
			objectList.Add(obj);
			
			if (onAddCallback != null)
				onAddCallback(obj);
			return result;						
		}
		
		public int registerObject(Resource obj, OnNewObjectAdded onAddCallback = null){
			return getId(obj, true, onAddCallback, false, false);			
		}
		
		public int getId(Resource obj, bool createMissing, OnNewObjectAdded onAddCallback = null, bool throwIfMissing = false, bool throwIfExists = false){
			int result = ExportUtility.invalidId;
			if (!isValidObject(obj))
				return result;
			if (objectMap.TryGetValue(obj, out result)){
				if (throwIfExists){
					throw new System.InvalidOperationException(
						string.Format("Object {0} already exists with id {1}", obj, result)
					);
				}
				return result;
			}
			if (!createMissing){
				if (throwIfMissing){
					throw new System.InvalidOperationException(
						string.Format("Could not find id for resource {0}", obj)
					);
				}
				return -1;
			}
			
			return addNewObjectInternal(obj, onAddCallback);
		}
	}
}
