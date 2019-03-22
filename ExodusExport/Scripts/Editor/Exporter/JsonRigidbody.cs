using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace SceneExport{
	[System.Serializable]
	public class JsonRigidbody: IFastJsonValue{
		public Rigidbody rig = null;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			//The constant manual value dump is getting annoying. Need to think up a better way to do it.
			writer.writeKeyVal("instanceId", rig.GetInstanceID());

			writer.writeKeyVal("angularDrag", rig.angularDrag);
			writer.writeKeyVal("angularVelocity", rig.angularVelocity);
			writer.writeKeyVal("centerOfMass", rig.centerOfMass);

			writer.writeKeyVal("collisionDetectionMode", rig.collisionDetectionMode.ToString());
			writer.writeKeyVal("constraints", (int)rig.constraints);
			writer.writeKeyVal("detectCollisions", rig.detectCollisions);
			writer.writeKeyVal("drag", rig.drag);

			writer.writeKeyVal("freezeRotation", rig.freezeRotation);
			writer.writeKeyVal("inertiaTensor", rig.inertiaTensor);
			writer.writeKeyVal("interpolation", rig.interpolation.ToString());

			writer.writeKeyVal("isKinematic", rig.isKinematic);
			writer.writeKeyVal("mass", rig.mass);

			writer.writeKeyVal("maxAngularVelocity", rig.maxAngularVelocity);
			writer.writeKeyVal("maxDepenetrationVelocity", rig.maxDepenetrationVelocity);

			writer.writeKeyVal("position", rig.position);
			writer.writeKeyVal("rotation", rig.rotation);
			writer.writeKeyVal("sleepThreshold", rig.sleepThreshold);
			writer.writeKeyVal("solverIterations", rig.solverIterations);
			writer.writeKeyVal("solverVelocityIterations", rig.solverVelocityIterations);
			writer.writeKeyVal("useGravity", rig.useGravity);
			writer.writeKeyVal("velocity", rig.velocity);
			writer.writeKeyVal("worldCenterOfMass", rig.worldCenterOfMass);

			writer.endObject();
		}

		public JsonRigidbody(Rigidbody rig_){
			if (!rig_){
				throw new System.ArgumentNullException("rig_");
			}

			rig = rig_;
		}
	}
}
