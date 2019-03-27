using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class JsonPhysicsJoint: IFastJsonValue{
		public Joint joint = null;
		public string jointType;

		public static readonly string springJointType = "spring";
		public static readonly string hingeJointType = "hinge";
		public static readonly string characterJointType = "character";
		public static readonly string fixedJointType = "fixed";
		public static readonly string configurableJointType = "configurable";

		public JsonObjectReference<Rigidbody> connectedBody = new JsonObjectReference<Rigidbody>(null);	
		public JsonObjectReference<GameObject> connectedBodyObject = new JsonObjectReference<GameObject>(null);	

		List<JsonSpringJointData> springJointData = new List<JsonSpringJointData>();
		List<JsonHingeJointData> hingeJointData = new List<JsonHingeJointData>();
		List<JsonConfigurableJointData> configurableJointData = new List<JsonConfigurableJointData>();
		List<JsonCharacterJointData> characterJointData = new List<JsonCharacterJointData>();

		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("jointType", jointType);

			writer.writeKeyVal("anchor", joint.anchor);
			writer.writeKeyVal("connectedAnchor", joint.connectedAnchor);
			writer.writeKeyVal("connectedBody", connectedBody);
			writer.writeKeyVal("connectedBodyObject", connectedBodyObject);
			writer.writeKeyVal("axis", joint.axis);
			writer.writeKeyVal("breakForce", joint.breakForce.ToString());
			writer.writeKeyVal("breakTorque", joint.breakTorque.ToString());
			writer.writeKeyVal("connectedMassScale", joint.connectedMassScale);
			writer.writeKeyVal("enableCollision", joint.enableCollision);
			writer.writeKeyVal("autoConfigureConnectedAnchor", joint.autoConfigureConnectedAnchor);

			writer.writeKeyVal("springJointData", springJointData, true);
			writer.writeKeyVal("hingeJointData", hingeJointData, true);
			writer.writeKeyVal("configurableJointData", configurableJointData, true);
			writer.writeKeyVal("characterJointData", characterJointData, true);

			writer.endObject();
		}

		public JsonPhysicsJoint(Joint joint_){
			if (!joint_)
				throw new System.ArgumentNullException("joint_");
			joint = joint_;

			connectedBody = new JsonObjectReference<Rigidbody>(joint.connectedBody);
			connectedBodyObject = joint.connectedBody ? 
				new JsonObjectReference<GameObject>(joint.connectedBody.gameObject)
				: new JsonObjectReference<GameObject>(null);

			var fixedJoint = joint as FixedJoint;
			var hingeJoint = joint as HingeJoint;
			var springJoint = joint as SpringJoint;
			var characterJoint = joint as CharacterJoint;
			var configurableJoint = joint as ConfigurableJoint;			

			if (fixedJoint){
				jointType = fixedJointType;
			}
			if (hingeJoint){
				jointType = hingeJointType;
				hingeJointData.Add(new JsonHingeJointData(hingeJoint));
			}
			if (springJoint){
				jointType = springJointType;
				springJointData.Add(new JsonSpringJointData(springJoint));
			}
			if (characterJoint){
				jointType = characterJointType;
				characterJointData.Add(new JsonCharacterJointData(characterJoint));
			}
			if (configurableJoint){
				jointType = configurableJointType;
				configurableJointData.Add(new JsonConfigurableJointData(configurableJoint));
			}
		}
	}
}
