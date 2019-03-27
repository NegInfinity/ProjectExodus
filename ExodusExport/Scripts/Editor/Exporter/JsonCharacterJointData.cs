using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class JsonCharacterJointData: IFastJsonValue{
		public CharacterJoint joint;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("enableProjection", joint.enableProjection);
			writer.writeKeyVal("highTwistLimit", joint.highTwistLimit);
			writer.writeKeyVal("lowTwistLimit", joint.lowTwistLimit);
			writer.writeKeyVal("projectionAngle", joint.projectionAngle);
			writer.writeKeyVal("projectionDistance", joint.projectionDistance);
			writer.writeKeyVal("swing1Limit", joint.swing1Limit);
			writer.writeKeyVal("swing2Limit", joint.swing2Limit);
			writer.writeKeyVal("swingAxis", joint.swingAxis);
			writer.writeKeyVal("swingLimitSpring", joint.swingLimitSpring);
			writer.writeKeyVal("twistLimitSpring", joint.twistLimitSpring);
			writer.endObject();
		}
		public JsonCharacterJointData(CharacterJoint joint_){
			if (!joint_)
				throw new System.ArgumentNullException("joint_");
			joint = joint_;
		}
	}
}