using UnityEngine;
using UnityEditor;

namespace SceneExport{
	/*
	Need to turn this thing into a generic.
	*/
	[System.Serializable]
	public struct AnimatorControllerKey{
		public readonly UnityEditor.Animations.AnimatorController controller;
		public readonly Animator animator;
			
		public override string ToString(){
			return string.Format("[AnimationClipKey]{{controller: {0}({1}); animator: {2}({3})}}", 
				controller, controller? controller.GetInstanceID(): 0, 
				animator, animator ? animator.GetInstanceID(): 0
			);
		}
		
		public override int GetHashCode(){
			int hash = 17;
			hash = hash * 23 + (controller ? controller.GetHashCode(): 0);
			hash = hash * 23 + (animator ? animator.GetHashCode(): 0);
			return hash;
		}
			
		public override bool Equals(object obj){
			if (obj is AnimatorControllerKey){
				return this.Equals((AnimatorControllerKey)obj);
			}
				
			return false;
		}
			
		public bool Equals(AnimatorControllerKey other){
			return (controller == other.controller) 
				&& (animator == other.animator);					
		}
			
		public AnimatorControllerKey(UnityEditor.Animations.AnimatorController animControl_, Animator animator_){
			if (!animControl_)
				throw new System.ArgumentNullException("animControl_");
			if (!animator_)
				throw new System.ArgumentNullException("animator_");
			controller = animControl_;
			animator = animator_;
		}
	}
}