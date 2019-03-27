using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class JsonSpringJointData: IFastJsonValue{
		public SpringJoint joint;

		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("damper", joint.damper);
			writer.writeKeyVal("maxDistance", joint.maxDistance);
			writer.writeKeyVal("minDistance", joint.minDistance);
			writer.writeKeyVal("spring", joint.spring);
			writer.writeKeyVal("tolerance", joint.tolerance);
			writer.endObject();
		}
		public JsonSpringJointData(SpringJoint joint_){
			if (!joint_)
				throw new System.ArgumentNullException("joint_");
			joint = joint_;				
		}
	}
}