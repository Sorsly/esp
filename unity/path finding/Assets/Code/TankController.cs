﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TankController : MonoBehaviour {
	Vector3 targetPosition;
	Vector3 lookAtTarget;
	Quaternion playerRot;
	float rotSpeed = 5;
	float speed = 10;
	bool moving = false;
	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		if (Input.GetMouseButton (0)) {
			SetTargetPosition ();
		}
		if (moving) {
			Move ();
		}
	}
	void SetTargetPosition(){
		Ray ray = Camera.main.ScreenPointToRay (Input.mousePosition);
		RaycastHit hit;

		if(Physics.Raycast(ray,out hit, 1000)){
			targetPosition = hit.point;
			//this.transform.LookAt(targetPosition);
			lookAtTarget = new Vector3(targetPosition.x - transform.position.x, transform.position.y, targetPosition.z - transform.position.z);
			playerRot = Quaternion.LookRotation (lookAtTarget);
			moving = true;
		}
	}
	void Move(){
		transform.rotation = Quaternion.Slerp(transform.rotation, playerRot, rotSpeed*Time.deltaTime);
		transform.position = Vector3.MoveTowards(transform.position, targetPosition, speed*Time.deltaTime);
		if (transform.position == targetPosition) {
			moving = false;
		}
	}
}

/*
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;

public class TankController : MonoBehaviour {
	Vector3 targetPosition;
	Vector3 lookAtTarget;
	Quaternion playerRot;
	float rotSpeed = 5;
	float speed = 10;
	bool moving = false;
	Transform _destination;
	NavMeshAgent _navMeshAgent;

	// Use this for initialization
	void Start () {
		_navMeshAgent = this.GetComponent<NavMeshAgent> ();
	}

	// Update is called once per frame
	void Update () {
		if (Input.GetMouseButton (0)) {
			SetTargetPosition ();
		}
		if (moving) {
			Move ();
		}
	}
	void SetTargetPosition(){
		Ray ray = Camera.main.ScreenPointToRay (Input.mousePosition);
		RaycastHit hit;

		if(Physics.Raycast(ray,out hit, 1000)){
			targetPosition = hit.point;
			//this.transform.LookAt(targetPosition);
			lookAtTarget = new Vector3(targetPosition.x - transform.position.x, transform.position.y, targetPosition.z - transform.position.z);
			playerRot = Quaternion.LookRotation (lookAtTarget);
			moving = true;
		}
	}
	void Move(){
		transform.rotation = Quaternion.Slerp(transform.rotation, playerRot, rotSpeed*Time.deltaTime);
		_destination.transform.position = targetPosition;//Vector3.MoveTowards(transform.position, targetPosition, speed*Time.deltaTime);

		Vector3 targetVector = _destination.transform.position;
		_navMeshAgent.SetDestination (targetVector);

		if (transform.position == targetPosition) {
			moving = false;
		}
	}
}
*/