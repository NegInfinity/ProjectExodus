using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class ObjectMapper<Resource>{
		public delegate void OnNewObjectAdded(Resource res);
		public Dictionary<Resource, ResId> objectMap = new Dictionary<Resource, ResId>();
		public List<Resource> objectList = new List<Resource>();

		public class IdEnumerator: IEnumerator<ResId>{
			int index = -1;
			ObjectMapper<Resource> owner = null;

			bool gotValidIndex(){
				return owner.isValidObjectIndex(index);
			}

			public object Current{
				get{
					return Current;
				}
			}

			ResId IEnumerator<ResId>.Current{
				get{
					if (gotValidIndex())
						return ResId.fromObjectIndex(index);
					return ResId.invalid;
				}
			}

			public void Dispose(){
				owner = null;			
			}

			public bool MoveNext(){
				int nextIndex = index + 1;
				if (owner.isValidObjectIndex(nextIndex))
					return false;

				index = nextIndex;
				return true;
			}

			public void Reset(){
				index = -1;
			}

			public IdEnumerator(ObjectMapper<Resource> owner_){
				if (owner_ == null)
					throw new System.ArgumentNullException("owner_");
				owner = owner_;
			}
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

		protected class ObjectIdsWrapper: IEnumerable<ResId>{
			ObjectMapper<Resource> owner = null;

			IEnumerator<ResId> IEnumerable<ResId>.GetEnumerator(){
				return GetEnumerator();
			}

			System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator(){
				return GetEnumerator();
			}

			public IdEnumerator GetEnumerator(){
				return new IdEnumerator(owner);
			}

			public ObjectIdsWrapper(ObjectMapper<Resource> owner_){
				if (owner_ == null)
					throw new System.ArgumentNullException("owner_");
				owner = owner_;
			}
		}

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
