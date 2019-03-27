using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class JsonHingeJointData: IFastJsonValue{
		public HingeJoint joint;

		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("angle", joint.angle);
			writer.writeKeyVal("limits", joint.limits);
			writer.writeKeyVal("motor", joint.motor);
			writer.writeKeyVal("spring", joint.spring);
			writer.writeKeyVal("useLimits", joint.useLimits);
			writer.writeKeyVal("useMotor", joint.useMotor);
			writer.writeKeyVal("useSpring", joint.useSpring);
			writer.endObject();
		}

		public JsonHingeJointData(HingeJoint joint_){
			if (!joint_)
				throw new System.ArgumentNullException("joint_");
			joint = joint_;
		}
	}
}