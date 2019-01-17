using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public struct AnimationClipKey{
		public readonly AnimationClip animClip;//Immutability, huh. Ugh.
		//public readonly Transform skeletonRoot;//
		public readonly Animator animator;//let's do it this way. It doubles as skeleton root as well.
			
		public override string ToString(){
			return string.Format("[AnimationClipKey]{{clip: {0}({1}); animator: {2}({3})}}", 
				animClip, animClip? animClip.GetInstanceID(): 0, 
				animator, animator ? animator.GetInstanceID(): 0
			);
		}
		
		public override int GetHashCode(){
			int hash = 17;
			hash = hash * 23 + (animClip ? animClip.GetHashCode(): 0);
			hash = hash * 23 + (animator ? animator.GetHashCode(): 0);
			return hash;
		}
			
		public override bool Equals(object obj){
			if (obj is AnimationClipKey){
				return this.Equals((AnimationClipKey)obj);
			}
				
			return false;
		}
			
		public bool Equals(AnimationClipKey other){
			return (animClip == other.animClip) 
				&& (animator == other.animator);					
		}
			
		public AnimationClipKey(AnimationClip animClip_, Animator animator_){
			if (!animClip_)
				throw new System.ArgumentNullException("animClip_");
			if (!animator_)
				throw new System.ArgumentNullException("animator_");
			animClip = animClip_;
			animator = animator_;
		}
	};			
}