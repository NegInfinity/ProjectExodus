using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public void writeRawValue(SoftJointLimit val){
			beginRawObject();
			writeKeyVal("limit", val.limit, false);
			writeKeyVal("spring", val.bounciness, false);
			endObject(false);
		}

		public void writeKeyVal(string key, SoftJointLimit val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeRawValue(SoftJointLimitSpring val){
			beginRawObject();
			writeKeyVal("spring", val.spring, false);
			writeKeyVal("damper", val.damper, false);
			endObject(false);
		}

		public void writeKeyVal(string key, SoftJointLimitSpring val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeRawValue(JointMotor val){
			beginRawObject();
			writeKeyVal("targetVelocity", val.targetVelocity, false);
			writeKeyVal("force", val.force, false);
			writeKeyVal("freeSpin", val.freeSpin, false);
			endObject(false);
		}

		public void writeKeyVal(string key, JointMotor val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeRawValue(JointLimits val){
			beginRawObject();
			writeKeyVal("min", val.min, false);
			writeKeyVal("max", val.max, false);
			writeKeyVal("bounciness", val.bounciness, false);
			writeKeyVal("bounceMinVelocity", val.bounceMinVelocity, false);
			writeKeyVal("contactDistance", val.contactDistance, false);
			endObject(false);
		}

		public void writeKeyVal(string key, JointLimits val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeRawValue(JointSpring val){
			beginRawObject();
			writeKeyVal("spring", val.spring, false);
			writeKeyVal("damper", val.damper, false);
			writeKeyVal("targetPosition", val.targetPosition, false);
			endObject(false);
		}

		public void writeKeyVal(string key, JointSpring val){
			writeKey(key);
			writeRawValue(val);
		}
	}
}
