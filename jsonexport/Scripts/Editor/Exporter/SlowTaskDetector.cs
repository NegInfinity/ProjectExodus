using UnityEngine;
using UnityEditor;

namespace SceneExport{
	public class SlowTaskDetector{
		float lastTime;
		float maxTime;
		
		float getTime(){
			return Time.realtimeSinceStartup;
		}
		
		void updateLastTime(){
			lastTime = getTime();
		}
		
		public SlowTaskDetector(float maxTime_ = 0.5f){
			maxTime = maxTime_;
			updateLastTime();
		}
		
		bool checkTimeLimit(){
			if ((getTime() - lastTime) > maxTime){
				updateLastTime();
				return false;
			}
			
			return true;
		}
	}
}