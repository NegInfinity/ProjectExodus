using UnityEngine;
using UnityEditor;

namespace SceneExport{
	public static class TaskDetectorExtensions{
		public static bool safeCheckTimeLimit(this SlowTaskDetector task){
			if (task == null)
				return true;
			return task.checkTimeLimit();
		}
	}
	
	public class SlowTaskDetector{
		float lastTime;
		float maxTime;
		
		static float getTime(){
			return Time.realtimeSinceStartup;
		}
		
		void updateLastTime(){
			lastTime = getTime();
		}
		
		public SlowTaskDetector(float maxTime_ = 0.5f){
			maxTime = maxTime_;
			lastTime = -1.0f;			
			//updateLastTime(); realtimeSinceStartup cannot be called from constructor;
		}
		
		public bool checkTimeLimit(){
			if (lastTime < 0){
				updateLastTime();
				return true;
			}
			if ((getTime() - lastTime) > maxTime){
				updateLastTime();
				return false;
			}
			
			return true;
		}
	}
}