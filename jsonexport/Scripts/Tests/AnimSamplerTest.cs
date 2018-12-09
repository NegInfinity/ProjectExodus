using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

namespace SceneExportTests{
	[System.Serializable]
	public class AnimSamplerTest: MonoBehaviour{
		public Animator srcAnimator = null;
		public AnimationClip animClip = null;
		public HumanBodyBones humanBone = HumanBodyBones.Head;
		
		public List<Vector3> points = new List<Vector3>();
		
		public void OnDrawGizmos(){
			drawGizmos(Color.yellow);
		}
		
		void drawCross(Vector3 pos){
			var dx = new Vector3(1.0f, 0.0f, 0.0f);
			var dy = new Vector3(0.0f, 1.0f, 0.0f);
			var dz = new Vector3(0.0f, 0.0f, 1.0f);
			
			Gizmos.DrawLine(pos - dx * 0.1f, pos + dx * 0.1f);
			Gizmos.DrawLine(pos - dy * 0.1f, pos + dy * 0.1f);
			Gizmos.DrawLine(pos - dz * 0.1f, pos + dz * 0.1f);
		}
		
		public void OnDrawGizmosSelected(){
			drawGizmos(Color.white);
		}
		
		void drawGizmos(Color c){
			var oldColor = Gizmos.color;
			var oldMatrix = Gizmos.matrix;
			Gizmos.matrix = transform.localToWorldMatrix;
			
			foreach(var cur in points)
				drawCross(cur);			
			
			Gizmos.matrix = oldMatrix;
			Gizmos.color = oldColor;			
		}
		
		public void samplePoints(){
			points.Clear();
			for(int i = 0; i < 100; i++){
				points.Add(new Vector3(Random.Range(-1.0f, 1.0f), Random.Range(-1.0f, 1.0f),Random.Range(-1.0f, 1.0f)));
			}
			
			if (!srcAnimator)
				return;
			
			points.Clear();
			var spawned = GameObject.Instantiate(srcAnimator, transform.position, transform.rotation);
			
			foreach(HumanBodyBones cur in System.Enum.GetValues(typeof(HumanBodyBones))){
				if ((cur == HumanBodyBones.LastBone) || ((int)cur == 0))//crashes otherwise
					continue;
					
				var bone = spawned.GetBoneTransform(cur);
				if (!bone)
					continue;
					
				points.Add(bone.position);
			}
		}
		
		public void clearPoints(){
			points.Clear();
		}
		
		void Update(){
		}
	}
}