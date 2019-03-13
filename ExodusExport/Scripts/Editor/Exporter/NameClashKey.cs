using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	struct NameClashKey{
		public string name;
		public ResId parentId;
			
		public override int GetHashCode(){
			int result = Utility.beginHash();
			result = Utility.combineHash(result, name);
			result = Utility.combineHash(result, parentId);
			return result;
		}
			
		public override bool Equals(object obj){
			if (!(obj is NameClashKey))
				return false;
			NameClashKey other = (NameClashKey)obj;
			return (name == other.name) && (parentId == other.parentId);					
		}
			
		public NameClashKey(string name_, ResId parentId_){
			name = name_;
			parentId = parentId_;
		}
	}
}