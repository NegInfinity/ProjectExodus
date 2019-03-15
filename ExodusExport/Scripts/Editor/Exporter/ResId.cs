using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public struct ResId{
		public int id;
		public static readonly int invalidId = -1;
		public static ResId invalid{
			get{
				return new ResId(invalidId);
			}
		}

		public static ResId fromObjectIndex(int index){
			return new ResId(index);
		}

		public int objectIndex{
			get{
				return id;
			}
		}

		public int rawId{
			get{
				return id;
			}
		}

		public bool isValid{
			get{
				return id >= 0;
			}
		}

		public ResId(int id_){
			id = id_;
		}

		public override int GetHashCode(){
			return id;
		}

		public override bool Equals(object obj){
			if (obj is ResId){
				return this.Equals((ResId)obj);
			}
			return false;
		}

		public bool Equals(ResId obj){
			return id == obj.id;
		}

		public static bool operator==(ResId lhs, ResId rhs){
			return lhs.Equals(rhs);
		}

		public static bool operator!=(ResId lhs, ResId rhs){
			return !lhs.Equals(rhs);
		}
	}
}
