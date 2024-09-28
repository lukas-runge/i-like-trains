<script lang="ts">
	import { onMount } from "svelte";
	import * as BABYLON from "@babylonjs/core";

	type Curve = {
		type: "curve";
		radius: number;
		angle: number;
		direction: "left" | "right";
	};

	type StraightLine = {
		type: "straight";
		length: number;
	};

	type TrackType = {
		name: string;
		ends: { id: number; x: number; y: number; angle: number }[];
		lines: Array<Curve | StraightLine>;
	};

	const CURVES = {
		R1: 36.0,
		R2: 43.75,
		R3: 51.5,
		R4: 57.93,
		R5: 64.36
	};

	const CURVE_COUNT = 12;
	const CURVE_ANGLE = 360 / CURVE_COUNT;

	const TRACK_TYPES = {
		straight: {
			name: "Straight",
			ends: [{ id: 1, x: 25, y: 0, angle: 0 }],
			lines: [{ type: "straight", length: 25 }]
		},
		curveR1Right: {
			name: "Curve R1 Right",
			ends: [{ id: 1, x: CURVES.R1, y: CURVES.R1, angle: 90 }],
			lines: [{ type: "curve", radius: CURVES.R1, angle: CURVE_ANGLE, direction: "right" }]
		},
		curveR1Left: {
			name: "Curve R1 Left",
			ends: [{ id: 1, x: CURVES.R1, y: CURVES.R1, angle: 90 }],
			lines: [{ type: "curve", radius: CURVES.R1, angle: CURVE_ANGLE, direction: "left" }]
		},
		switchLeft: {
			name: "Switch Left",
			ends: [
				{ id: 1, x: 25, y: 0, angle: 0 },
				{ id: 2, x: CURVES.R2, y: CURVES.R1, angle: 90 }
			],
			lines: [
				{ type: "straight", length: 25 },
				{ type: "curve", radius: CURVES.R1, angle: CURVE_ANGLE, direction: "left" }
			]
		}
	} as Record<string, TrackType>;

	type Track = {
		id: number;
		type: TrackType;
		startFrom?: [number, number];
	};

	let trackPlan: Track[] = [
		//{ id: 1, type: TRACK_TYPES.straight },
		{ id: 5, type: TRACK_TYPES.switchLeft },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Left },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Left },
		{ id: 5, type: TRACK_TYPES.curveR1Left },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		{ id: 5, type: TRACK_TYPES.curveR1Right },
		// { id: 1, type: TRACK_TYPES.straight },
		// { id: 2, type: TRACK_TYPES.straight },
		// { id: 3, type: TRACK_TYPES.straight },
		// { id: 5, type: TRACK_TYPES.curveR1Right },
		// { id: 5, type: TRACK_TYPES.curveR1Right },
		// { id: 5, type: TRACK_TYPES.curveR1Right },
		// { id: 4, type: TRACK_TYPES.straight },
		// { id: 5, type: TRACK_TYPES.curveR1Right },
		// { id: 5, type: TRACK_TYPES.curveR1Right },
		// { id: 5, type: TRACK_TYPES.curveR1Right },
		// { id: 3, type: TRACK_TYPES.straight },
	];

	let engine: BABYLON.Engine;
	let scene: BABYLON.Scene;
	let canvas: HTMLCanvasElement;

	onMount(() => {
		// Create BabylonJS scene
		canvas = document.getElementById("renderCanvas") as HTMLCanvasElement;
		engine = new BABYLON.Engine(canvas, true);
		scene = new BABYLON.Scene(engine);

		// Setup camera and light
		// camera from top view
		const camera = new BABYLON.ArcRotateCamera(
			"camera1",
			0,
			0,
			100,
			new BABYLON.Vector3(25, 0, 0),
			scene
		);
		camera.attachControl(canvas, true);
		const light = new BABYLON.HemisphericLight("light1", new BABYLON.Vector3(0, 1, -1), scene);

		const basicMaterial = new BABYLON.StandardMaterial("basicMaterial", scene);
		const groundMaterial = new BABYLON.StandardMaterial("groundMaterial", scene);
		const grayColor = new BABYLON.Color3(0.5, 0.5, 0.5);
		const blackColor = new BABYLON.Color3(0.1, 0.1, 0.1);
		basicMaterial.diffuseColor = grayColor;
		groundMaterial.diffuseColor = blackColor;

		const size = 2;
		const trackCrossSectionShape = [
			new BABYLON.Vector3(0, 0, 0),
			new BABYLON.Vector3(0, size, 0),
			new BABYLON.Vector3(size, size, 0),
			new BABYLON.Vector3(size, 0, 0),
			new BABYLON.Vector3(0, 0, 0)
		];

		// put cube in center of scene
		const cube = BABYLON.MeshBuilder.CreateBox("cube", { size: 1 }, scene);

		const ground = BABYLON.MeshBuilder.CreateGround("ground", { width: 100, height: 100 }, scene);
		ground.position.y = -0.5;
		ground.material = groundMaterial;

		type CreatorReturn = {
			endPosition: BABYLON.Vector3;
			endRotation: BABYLON.Vector3;
		}

		// Function to create a straight track
		function createStraight(length: number, position: BABYLON.Vector3, rotation: BABYLON.Vector3): CreatorReturn {
			const points = [
				new BABYLON.Vector3(0, 0, 0),
				new BABYLON.Vector3(length, 0, 0)
			];

			const track = BABYLON.MeshBuilder.ExtrudeShape(
				"track",
				{
					shape: trackCrossSectionShape, path: points, cap: BABYLON.Mesh.NO_CAP, sideOrientation: BABYLON.Mesh.DOUBLESIDE},
				scene
			);
			track.material = basicMaterial;
			track.rotation = rotation.clone();
			track.position = position.clone();


			return {
				endPosition: position.add(new BABYLON.Vector3(length, 0, 0).rotateByQuaternionToRef(rotation.toQuaternion(), new BABYLON.Vector3())),
				endRotation: rotation
			};
		}

		// Function to create a curve track
		function createCurve(radius: number, angle: number, direction: "left" | "right", position: BABYLON.Vector3, rotation: BABYLON.Vector3): CreatorReturn {
			const points = [];
			const angleOffset = 90;
			const sign = (direction == "left" ? 1 : -1);
			for (let i = 0; i <= angle; i++) {
				const x = (Math.cos(((i - angleOffset) * Math.PI) / 180)) * radius;
				const y = sign *(Math.sin(((i - angleOffset) * Math.PI) / 180) + 1) * radius;
				points.push(new BABYLON.Vector3(x, 0, y));
			}

			const track = BABYLON.MeshBuilder.ExtrudeShape(
				"track",
				{ shape: trackCrossSectionShape, path: points, cap: BABYLON.Mesh.NO_CAP, sideOrientation: BABYLON.Mesh.DOUBLESIDE },
				scene
			);
			track.material = basicMaterial;
			track.position = position.clone();
			track.rotation = rotation.clone();
			return {
				endPosition: points[points.length - 1].rotateByQuaternionToRef(rotation.toQuaternion(), new BABYLON.Vector3()).addInPlace(position),
				endRotation: rotation.add(new BABYLON.Vector3(0, sign * -angle * Math.PI / 180, 0))
			};

		}

		// Render each track in the track plan
		let currentPosition = new BABYLON.Vector3(0, 0, 0);
		let currentRotation = new BABYLON.Vector3(0, 0, 0);
		for (const track of trackPlan) {
			let end: CreatorReturn | undefined;
			for (const line of track.type.lines) {
				if (line.type === "straight") {
					end = createStraight(line.length, currentPosition, currentRotation);
				} else if (line.type === "curve") {
					end = createCurve(line.radius, line.angle, line.direction, currentPosition, currentRotation);
				}
			}
			if (end) {
				currentPosition = end.endPosition.clone();
				currentRotation = end.endRotation.clone();
			}
		}

		engine.runRenderLoop(() => {
			scene.render();
		});

		window.addEventListener("resize", () => {
			engine.resize();
		});
	});
</script>

<canvas id="renderCanvas"></canvas>

<style>
	canvas {
		display: block;
		width: 100%;
		height: 100%;
	}
</style>
