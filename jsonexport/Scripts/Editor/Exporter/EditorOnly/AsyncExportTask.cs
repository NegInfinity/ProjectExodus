using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class AsyncExportTask{
		public int currentProgress = 0;
		public int maxProgress = 0;
		public string currentStatus = "";
		public bool finished = false;
		public bool running = false;
		public EditorWindow exporterWindow = null;
		
		public void repaintWindow(){
			if (!exporterWindow)
				return;
			exporterWindow.Repaint();
		}
		
		public SlowTaskDetector slowTaskDetector = new SlowTaskDetector(0.25f);
		public SlowTaskDetector repaintTrigger = new SlowTaskDetector(0.5f);
		
		public void startNew(){
			running = true;
			finished = false;
		}
		
		public void finish(){
			running = false;
			finished = true;
		}
		
		public void markRunning(bool running_ = true){
			running = running_;
		}
		
		public void markStopped(){
			running = false;
		}
		
		public void markTaskStart(){
			finished = false;
		}
		
		public void markFinished(bool finished_ = true){
			finished = finished_;
		}
		
		public void checkRepaint(){
			if (repaintTrigger == null)
				return;
			if (repaintTrigger.checkWithinTimeLimit())
				return;
			repaintWindow();
		}
		
		public bool needsPause(){
			if (slowTaskDetector == null)
				return false;
			return slowTaskDetector.checkWithinTimeLimit();
		}
		
		public void setStatus(string newStatus_){
			currentStatus = newStatus_;
		}
		
		public void beginProgress(int maxProgress_, int startVal = 0){
			maxProgress = maxProgress_;
			currentProgress = 0;
		}
		
		public void setProgress(int curProgress_){
			currentProgress = curProgress_;
		}
		
		public void incProgress(){
			currentProgress++;
		}
		
		public float getFloatProgress(){
			if (maxProgress <= 0)
				return 0;
			var result = (float)currentProgress/(float)maxProgress;
			result = Mathf.Clamp01(result);
			return result;
		}		
	}
}