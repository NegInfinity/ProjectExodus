using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class JsonConfigurableJointData: IFastJsonValue{
		public ConfigurableJoint joint;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("angularXDrive", joint.angularXDrive);
			writer.writeKeyVal("angularXLimitSpring", joint.angularXLimitSpring);
			writer.writeKeyVal("angularXMotion", joint.angularXMotion.ToString());

			writer.writeKeyVal("angularYLimit", joint.angularYLimit);
			writer.writeKeyVal("angularYMotion", joint.angularYMotion.ToString());

			writer.writeKeyVal("angularYZDrive", joint.angularYZDrive);
			writer.writeKeyVal("angularYZLimitSpring", joint.angularYZLimitSpring);

			writer.writeKeyVal("angularZLimit", joint.angularZLimit);
			writer.writeKeyVal("angularZMotion", joint.angularZMotion.ToString());

			writer.writeKeyVal("configuredInWorldSpace", joint.configuredInWorldSpace);
			writer.writeKeyVal("highAngularXLimit", joint.highAngularXLimit);

			writer.writeKeyVal("linearLimit", joint.linearLimit);
			writer.writeKeyVal("linearLimitSpring", joint.linearLimitSpring);

			writer.writeKeyVal("lowAngularXLimit", joint.lowAngularXLimit);
			writer.writeKeyVal("projectionAngle", joint.projectionAngle);
			writer.writeKeyVal("projectionDistance", joint.projectionDistance);
			writer.writeKeyVal("projectionMode", joint.projectionMode.ToString());

			writer.writeKeyVal("rotationDriveMode", joint.rotationDriveMode.ToString());
			writer.writeKeyVal("secondaryAxis", joint.secondaryAxis);

			writer.writeKeyVal("slerpDrive", joint.slerpDrive);
			writer.writeKeyVal("swapBodies", joint.swapBodies);

			writer.writeKeyVal("targetAngularVelocity", joint.targetAngularVelocity);
			writer.writeKeyVal("targetPosition", joint.targetPosition);
			writer.writeKeyVal("targetRotation", joint.targetRotation);
			writer.writeKeyVal("targetVelocity", joint.targetVelocity);

			writer.writeKeyVal("xDrive", joint.xDrive);
			writer.writeKeyVal("xMotion", joint.xMotion.ToString());

			writer.writeKeyVal("yDrive", joint.yDrive);
			writer.writeKeyVal("yMotion", joint.yMotion.ToString());

			writer.writeKeyVal("zDrive", joint.zDrive);
			writer.writeKeyVal("zMotion", joint.zMotion.ToString());

			writer.endObject();
		}
		public JsonConfigurableJointData(ConfigurableJoint joint_){				
			if (!joint_)
				throw new System.ArgumentNullException("joint_");
			joint = joint_;
		}
	}
}
