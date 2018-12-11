using UnityEngine;
using UnityEditor;

namespace SceneExport{
	public static class TaskDetectorExtensions{
		public static bool safeCheckTimeLimit(this SlowTaskDetector task){
			if (task == null)
				return true;
			return task.checkWithinTimeLimit();
		}
	}
	
	public class SlowTaskDetector{
		//BAH. System.Diagnoistics.Stopwatch is broken.
		//System.Diagnostics.Stopwatch stopWatch = new System.Diagnostics.Stopwatch();
		float maxTime;
		float lastTime;
				
		static float getTime(){
			return Time.realtimeSinceStartup;
		}
		
		void updateLastTime(){
			lastTime = getTime();
		}
		
		public SlowTaskDetector(float maxTime_ = 0.5f){
			maxTime = maxTime_;
			lastTime = -1.0f;			
		}
		
		float getElapsedTime(){
			if (lastTime < 0)
				lastTime = getTime();
			return getTime() - lastTime;
		}
		
		void resetElapsedTime(){
			lastTime = getTime();
		}
		
		public bool checkWithinTimeLimit(){
			float elapsed  = getElapsedTime();
			if (elapsed  > maxTime){
				resetElapsedTime();
				return false;
			}
			
			return true;
		}
	}
}