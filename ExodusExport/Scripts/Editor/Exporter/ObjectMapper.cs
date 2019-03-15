using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class ObjectMapper<Resource>{
		public delegate void OnNewObjectAdded(Resource res);

		public Dictionary<Resource, ResId> objectMap = new Dictionary<Resource, ResId>();
		public List<Resource> objectList = new List<Resource>();

		/*/
		public class Watcher{
			ObjectMapper<Resource> owner = null;
			int lastCount = 0;

			public void reset(){
				lastCount = 0;
			}

			void checkOwner(){
				if (owner == null)
					throw new System.ArgumentException("owner", "Owner cannot be null");
			}

			public bool hasNewObjects{
				get{
					checkOwner();
					return owner.numObjects != lastCount;
				}
			}

			public IEnumerable<int> getNewIndexes(){
				if (!hasNewObjects)
					yield break;
				for(int i = lastCount; i < owner.numObjects; i++){
					yield return i;
				}
			}

			public IEnumerable<ResId> getNewIds(){
				if (!hasNewObjects)
					yield break;
				for(int i = lastCount; i < owner.numObjects; i++){
					yield return ResId.fromObjectIndex(i);
				}
			}

			public IEnumerable<Resource> getNewObjects(){
				checkOwner();
				if (!hasNewObjects)
					yield break;
				foreach(ResId cur in getNewIds()){
					yield return owner.getObject(cur);
				}
			}

			public void updateCount(){
				checkOwner();
				lastCount = owner.numObjects;
			}

			public Watcher(ObjectMapper<Resource> owner_){
				if (owner_ == null)
					throw new System.ArgumentNullException("owner_");
				owner = owner_;
			}
		}
		*/
		public ResourceStorageWatcher<ObjectMapper<Resource>, Resource> createWatcher(){
			return new ResourceStorageWatcher<ObjectMapper<Resource>, Resource>(
				this, 
				(obj) => obj.numObjects, 
				(obj, idx) => obj.getObjectByIndex(idx)
			);
		}

		public bool isValidObjectId(ResId id){
			return (id.objectIndex >= 0) && (id.objectIndex < objectList.Count);
		}
		
		public bool isValidObjectIndex(int index){
			return (index >= 0) && (index < objectList.Count);
		}
		
		public int numObjects{
			get{
				return objectList.Count;
			}
		}

		/*
		public Watcher makeWatcher(){
			return new Watcher(this);
		}
		*/

		public IEnumerable<ResId> getObjectIds(){
			return null;
		}
		
		public Resource getObject(ResId id){
			if (!isValidObjectId(id))
				throw new System.ArgumentException(string.Format("Invalid object id {0}", id));
			return objectList[id.rawId];
		}
		
		public Resource getObjectByIndex(int index){
			if ((index < 0) || (index >= objectList.Count))
				throw new System.ArgumentException(string.Format("Invalid object index {0}", index));
			return objectList[index];
		}

		public bool hasObject(Resource obj){
			return ExportUtility.isValidId(getId(obj, false));
		}

		public bool isValidId(ResId id){
			return (id.objectIndex >= 0) && (id.objectIndex < objectList.Count);
		}
			
		public ResId findId(Resource obj){
			return getId(obj, false, null);
		}
		
		public ResId getId(Resource obj){
			return getId(obj, true, null);
		}
		
		protected bool isValidObject(Resource obj){
			return !Object.Equals(obj, null);
		}
		
		protected ResId addNewObjectInternal(Resource obj, OnNewObjectAdded onAddCallback = null){
			if (objectMap.ContainsKey(obj))
				throw new System.ArgumentException("Logic error: duplicate registertation");
			var result = new ResId(objectList.Count);
			objectMap.Add(obj, result);
			objectList.Add(obj);
			
			if (onAddCallback != null)
				onAddCallback(obj);
			return result;						
		}
		
		public ResId registerObject(Resource obj, OnNewObjectAdded onAddCallback = null){
			return getId(obj, true, onAddCallback, false, false);			
		}
		
		public ResId getId(Resource obj, bool createMissing, OnNewObjectAdded onAddCallback = null, bool throwIfMissing = false, bool throwIfExists = false){
			//int result = ExportUtility.invalidId;
			var result = ResId.invalid;
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
				return ResId.invalid;
			}
			
			return addNewObjectInternal(obj, onAddCallback);
		}
	}
}
