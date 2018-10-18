using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class Logger{
		public static void makeValid(ref Logger logger){
			if (logger == null)
				logger = defaultLogger;
		}
		[System.Serializable]
		public enum MessageType{
			Log = 0,
			Warning = 1,
			Error = 2
		};
		
		[System.Serializable]
		public class Message{
			public MessageType msgType;
			public string message;
			public Message(MessageType msgType_, string message_){
				msgType = msgType_;
				message = message_;
			}
		}
		
		public List<Message> messages = new List<Message>();
		public bool saveMessages = true;
		
		public void clear(){
			messages.Clear();
		}

		static Logger storedDefaultLogger = null;
		public static Logger defaultLogger{
			get{
				if (storedDefaultLogger == null)
					storedDefaultLogger = new Logger(false);
				return storedDefaultLogger;
			}
		}
		
		public Logger(bool saveMessages_ = true){
			saveMessages = saveMessages_;
		}
		
		public Logger(Logger other){
			messages = other.messages.ToList();
			saveMessages = other.saveMessages;
		}

		public static Logger getValid(Logger arg){
			if (arg == null)
				return defaultLogger;
			return arg;
		}		
		
		void addMessage(MessageType type_, string msg){
			messages.Add(new Message(type_, msg));
		}
		
		public void log(string msg){
			Debug.Log(msg);
			if (saveMessages)
				addMessage(MessageType.Log, msg);
		}
		
		public void logWarning(string msg){
			Debug.LogWarning(msg);
			if (saveMessages)
				addMessage(MessageType.Warning, msg);
		}
		
		public void logError(string msg){
			Debug.LogError(msg);
			if (saveMessages)
				addMessage(MessageType.Error, msg);
		}
		
		public void logFormat(string msg, params object[] args){
			log(string.Format(msg, args));
		}
		
		public void logWarningFormat(string msg, params object[] args){
			logWarning(string.Format(msg, args));
		}
		
		public void logErrorFormat(string msg, params object[] args){
			logError(string.Format(msg, args));
		}
	}
}
