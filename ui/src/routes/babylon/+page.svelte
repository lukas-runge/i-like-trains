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
			lines: [{ type: "straight", length: 25 }]
		},
		...Object.fromEntries(
			(["right", "left"] as const).flatMap((direction) =>
				Object.entries(CURVES).map(([key, radius]) => [
					`curve${key}${direction.charAt(0).toUpperCase() + direction.slice(1)}`,
					{
						name: `Curve ${key}`,
						lines: [
							{
								type: "curve",
								radius,
								angle: CURVE_ANGLE,
								direction
							}
						]
					}
				])
			)
		),

		...Object.fromEntries(
			(["right", "left"] as const).flatMap((direction) =>
				Object.entries(CURVES).map(([key, radius]) => [
					`switch${key}${direction.charAt(0).toUpperCase() + direction.slice(1)}`,
					{
						name: `Switch ${direction}`,
						lines: [
							{ type: "straight", length: 25 },
							{ type: "curve", radius, angle: CURVE_ANGLE, direction }
						]
					},
				]))),
	} as Record<string, TrackType>;

	console.log(TRACK_TYPES);

	type Track = {
		id?: number;
		type: TrackType;
		startFrom?: [number, number];
	};

	let trackPlan: Track[] = [
		{ id: 0, type: TRACK_TYPES.switchR1Left },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right, startFrom: [0, 1]},
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
	];

	for (const track of trackPlan) {
		if (track.id === undefined) {
			track.id = Math.max(...trackPlan.map((t) => t.id ?? 0)) + 1;
		}
	}
	console.log(trackPlan);

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

		const trackMaterial = new BABYLON.StandardMaterial("basicMaterial", scene);
		const groundMaterial = new BABYLON.StandardMaterial("groundMaterial", scene);
		const grayColor = new BABYLON.Color3(0.5, 0.5, 0.5);
		const whiteColor = new BABYLON.Color3(2, 2, 2);
		const redColor = new BABYLON.Color3(1, 0, 0);
		trackMaterial.diffuseColor = grayColor;
		trackMaterial.specularPower = 1000000;
		const trackMaterialRed = new BABYLON.StandardMaterial("trackMaterialRed", scene);
		trackMaterialRed.diffuseColor = redColor;
		trackMaterialRed.specularPower = 1000000;
		groundMaterial.diffuseColor = whiteColor;
		groundMaterial.specularPower = 1000000;

		const width = 2;
		const height = 0.4;
		const trackCrossSectionShape = [
			new BABYLON.Vector3(0, 0, 0),
			new BABYLON.Vector3(0, height, 0),
			new BABYLON.Vector3(width, height, 0),
			new BABYLON.Vector3(width, 0, 0),
			new BABYLON.Vector3(0, 0, 0)
		];

		const ground = BABYLON.MeshBuilder.CreateGround("ground", { width: 1000, height: 1000 }, scene);
		ground.position.y = -0.5;
		ground.material = groundMaterial;

		// Function to create a straight track
		function createStraight(length: number, pandr: PandR, material: BABYLON.Material) {
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
			track.material = material;
			track.rotation = pandr.rotation.clone();
			track.position = pandr.position.clone();


			return {
				pandr: {
					position: pandr.position.add(new BABYLON.Vector3(length, 0, 0).rotateByQuaternionToRef(pandr.rotation.toQuaternion(), new BABYLON.Vector3())),
					rotation: pandr.rotation
				} as PandR,
				mesh: track,
			};
		}

		// Function to create a curve track
		function createCurve(radius: number, angle: number, direction: "left" | "right", pandr: PandR, material: BABYLON.Material) {
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
			track.material = material;
			track.position = pandr.position.clone();
			track.rotation = pandr.rotation.clone();
			track.position.y -= 0.01;
			return {
				pandr: {
					position: points[points.length - 1].rotateByQuaternionToRef(pandr.rotation.toQuaternion(), new BABYLON.Vector3()).addInPlace(pandr.position),
					rotation: pandr.rotation.add(new BABYLON.Vector3(0, sign * -angle * Math.PI / 180, 0))
				},
				mesh: track
			};

		}

		type PandR = {
			position: BABYLON.Vector3;
			rotation: BABYLON.Vector3;
		};

		// Render each track in the track plan
		let current: PandR = { position: new BABYLON.Vector3(0, 0, 0), rotation: new BABYLON.Vector3(0, 0, 0) };
		let last: Record<number, PandR[]> = {};

		type Switch = {
			straight: BABYLON.Mesh;
			curve: BABYLON.Mesh;
			currentDirection: "straight" | "curve";
		};

		const switches = new Set<Switch>();

		for (const track of trackPlan) {
			if (track.startFrom) {
				current = last[track.startFrom[0]][track.startFrom[1]];
			}

			last[track.id!] = [];

			const material = trackMaterial;
			const meshes = [];
			for (let i = 0; i < track.type.lines.length; i++) {
				const line = track.type.lines[i];
				if (line.type === "straight") {
					const { pandr, mesh } = createStraight(line.length, current, material);
					last[track.id!].push(pandr);
					meshes.push(mesh);
				} else if (line.type === "curve") {
					const { pandr, mesh } = createCurve(line.radius, line.angle, line.direction, current, material);
					last[track.id!].push(pandr);
					meshes.push(mesh);
				}
			}
			if (track.type.name.includes("Switch")) {
				switches.add({
					straight: meshes[0],
					curve: meshes[1],
					currentDirection: "straight",
				});
			}
			current = last[track.id!][0];
		}

		function setSwitchDirection(switchE: Switch, direction: "straight" | "curve") {
			if (direction === "straight") {
				switchE.straight.material = trackMaterialRed;
				switchE.curve.material = trackMaterial;
				switchE.curve.position.y = -0.01;
				switchE.straight.position.y = 0.0;
			} else {
				switchE.straight.material = trackMaterial;
				switchE.curve.material = trackMaterialRed;
				switchE.straight.position.y = -0.01;
				switchE.curve.position.y = 0.0;
			}
			switchE.currentDirection = direction;
		}

		for (const switchE of switches) {
			setSwitchDirection(switchE, switchE.currentDirection);
		}

		scene.onPointerPick = (_, pickResult) => {
			if (pickResult.hit && pickResult.pickedMesh) {
				for (const switchE of switches) {
					if (pickResult.pickedMesh === switchE.straight || pickResult.pickedMesh === switchE.curve) {
						setSwitchDirection(switchE, switchE.currentDirection === "straight" ? "curve" : "straight");
					}
				}
			}
		};

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
