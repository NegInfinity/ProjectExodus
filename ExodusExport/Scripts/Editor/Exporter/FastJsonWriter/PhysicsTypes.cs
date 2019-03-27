using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public void writeRawValue(JointDrive val){
			beginRawObject(false);
			writeKeyVal("positionSpring", val.positionSpring, false);
			writeKeyVal("positionDamper", val.positionDamper, false);
			writeKeyVal("maximumForce", val.maximumForce, false);
			endObject(false);
		}

		public void writeKeyVal(string key, JointDrive val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeRawValue(SoftJointLimit val){
			beginRawObject(false);
			writeKeyVal("limit", val.limit, false);
			writeKeyVal("spring", val.bounciness, false);
			endObject(false);
		}

		public void writeKeyVal(string key, SoftJointLimit val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeRawValue(SoftJointLimitSpring val){
			beginRawObject(false);
			writeKeyVal("spring", val.spring, false);
			writeKeyVal("damper", val.damper, false);
			endObject(false);
		}

		public void writeKeyVal(string key, SoftJointLimitSpring val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeRawValue(JointMotor val){
			beginRawObject(false);
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
			beginRawObject(false);
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
			beginRawObject(false);
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
