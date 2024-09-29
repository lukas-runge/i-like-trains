<script lang="ts">
	import { onMount } from "svelte";
	import * as BABYLON from "@babylonjs/core";
	import SwitchIcon from "./SwitchIcon.svelte";

	type Curve = {
		type: "curve";
		radius: number;
		angle: number;
		direction: "left" | "right";
		flipped?: boolean;
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
		R0: 20.0,
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
		straightShort: {
			name: "Straight Short",
			lines: [{ type: "straight", length: 9 }]
		},
		straightShort2: {
			name: "Straight Short",
			lines: [{ type: "straight", length: 7 }]
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
		...Object.fromEntries(
			(["right", "left"] as const).flatMap((direction) =>
				Object.entries(CURVES).map(([key, radius]) => [
					`switchDown${key}${direction.charAt(0).toUpperCase() + direction.slice(1)}`,
					{
						name: `Switch Down ${direction}`,
						lines: [
							{ type: "straight", length: 25 },
							{ type: "curve", radius, angle: CURVE_ANGLE, direction, flipped: true }
						]
					},
				]))),
	} as Record<string, TrackType>;

	console.log(TRACK_TYPES);

	type Track = {
		id?: number;
		type: TrackType;
		startFrom?: [number, number];
		highlight?: boolean;
	};

	let trackPlan: Track[] = [
		{ id: 7, type: TRACK_TYPES.switchR1Left },

		// Abstell 1
		{ type: TRACK_TYPES.curveR1Left, startFrom: [7, 1] },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },

		// Außen 4
		{ id: 12, type: TRACK_TYPES.switchR1Left, startFrom: [7, 0] },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ id: 14, type: TRACK_TYPES.switchR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },

		// Ue1
		{ type: TRACK_TYPES.curveR1Left, startFrom: [14, 1] },
		{ type: TRACK_TYPES.curveR1Left },
		{ id: 13, type: TRACK_TYPES.switchDownR1Left },
		{ id: 1, type: TRACK_TYPES.switchDownR1Left },

		// außen 1
		{ type: TRACK_TYPES.straight },
		{ id: 11, type: TRACK_TYPES.switchR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },

		// Außen 3
		{ type: TRACK_TYPES.curveR1Left, startFrom: [12, 1]},
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ id: 6, type: TRACK_TYPES.switchR1Left },

		// Gleis 3
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },

		// Gleis 2
		{ type: TRACK_TYPES.curveR1Left, startFrom: [6, 1] },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ id: 5, type: TRACK_TYPES.switchDownR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ id: 4, type: TRACK_TYPES.switchDownR1Left },
		{ id: 3, type: TRACK_TYPES.switchR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },

		// Innen 1
		{ type: TRACK_TYPES.curveR1Left, startFrom: [3, 1] },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },

		{ type: TRACK_TYPES.curveR1Left, startFrom: [11, 1] },
		{ type: TRACK_TYPES.curveR1Left },
		{ id: 10, type: TRACK_TYPES.switchDownR1Left },
		{ id: 2, type: TRACK_TYPES.switchR1Left },

		// Innen 2
		{ type: TRACK_TYPES.curveR1Right, startFrom: [2, 1] },
		{ id: 9, type: TRACK_TYPES.switchDownR1Left },
		{ id: 8, type: TRACK_TYPES.switchDownR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straightShort },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.curveR1Left },
		{ type: TRACK_TYPES.straightShort },
		{ type: TRACK_TYPES.straightShort },

		// Gleis 1
		{ type: TRACK_TYPES.curveR1Left, startFrom: [4, 1] },
		{ type: TRACK_TYPES.straightShort },
		{ id: 15, type: TRACK_TYPES.switchR1Right },

		// Abstell 4
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.curveR1Right },

		// Abstell 5
		{ type: TRACK_TYPES.curveR1Right, startFrom: [15, 1] },
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.straight },

		// Abstell 2
		{ type: TRACK_TYPES.curveR1Left, startFrom: [9, 1] },
		{ type: TRACK_TYPES.straight },

		// Oben
		{ type: TRACK_TYPES.straight, startFrom: [8, 1]},
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.curveR0Left },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straightShort2 },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.curveR0Right },
		{ type: TRACK_TYPES.straight },
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
	let trackMaterial: BABYLON.StandardMaterial;
	let trackMaterialRed: BABYLON.StandardMaterial;

	type Switch = {
		id: number;
		direction: "left" | "right";
		straight: BABYLON.Mesh;
		curve: BABYLON.Mesh;
		currentDirection: "straight" | "curve";
		initialized?: boolean;
	};
	let switches: Switch[] = [];

	onMount(() => {
		// Create BabylonJS scene
		canvas = document.getElementById("renderCanvas") as HTMLCanvasElement;
		engine = new BABYLON.Engine(canvas, true);
		scene = new BABYLON.Scene(engine);

		const resizeObserver = new ResizeObserver(() => {
			engine.resize();
		});
		resizeObserver.observe(canvas);

		// Setup camera and light
		// camera from top view
		const camera = new BABYLON.ArcRotateCamera(
			"camera1",
			Math.PI / 2,
			0,
			300,
			new BABYLON.Vector3(-100, 0, 100),
			scene
		);
		camera.attachControl(canvas, true);
		const light = new BABYLON.HemisphericLight("light1", new BABYLON.Vector3(0, 1, -1), scene);

		trackMaterial = new BABYLON.StandardMaterial("basicMaterial", scene);
		const groundMaterial = new BABYLON.StandardMaterial("groundMaterial", scene);
		const grayColor = new BABYLON.Color3(0.5, 0.5, 0.5);
		const whiteColor = new BABYLON.Color3(2, 2, 2);
		const redColor = new BABYLON.Color3(1, 0, 0);
		trackMaterial.diffuseColor = grayColor;
		trackMaterial.specularPower = 1000000;
		trackMaterialRed = new BABYLON.StandardMaterial("trackMaterialRed", scene);
		trackMaterialRed.diffuseColor = redColor;
		trackMaterialRed.specularPower = 1000000;
		groundMaterial.diffuseColor = whiteColor;
		groundMaterial.specularPower = 1000000;
		const yellowColor = new BABYLON.Color3(1, 1, 0);
		const yellowMaterial = new BABYLON.StandardMaterial("yellowMaterial", scene);
		yellowMaterial.diffuseColor = yellowColor;

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
		function createCurve(radius: number, angle: number, direction: "left" | "right", pandr: PandR, material: BABYLON.Material, flipped = false) {
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

			if (flipped) {
				// track.setEnabled(false);
				track.rotation.z += Math.PI;
				track.position.y += height;

				// move -25 depending on pandr rotation
				// track.position.z -= 25; if pandr.rotation.y == 0
				track.position.addInPlace(new BABYLON.Vector3(25, 0, 0).rotateByQuaternionToRef(pandr.rotation.toQuaternion(), new BABYLON.Vector3()));

				return {
					pandr: {
						// also flipped
						position: points[points.length - 1].rotateByQuaternionToRef(pandr.rotation.toQuaternion(), new BABYLON.Vector3()).addInPlace(pandr.position).addInPlace(new BABYLON.Vector3(-12, 0, -1.8).rotateByQuaternionToRef(pandr.rotation.toQuaternion(), new BABYLON.Vector3())),
						// also flipped
						rotation: pandr.rotation.add(new BABYLON.Vector3(0, (180 + sign * angle) * Math.PI / 180, 0))
					},
					mesh: track
				}
			}

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

		for (const track of trackPlan) {
			if (track.startFrom) {
				current = last[track.startFrom[0]][track.startFrom[1]];
			}

			last[track.id!] = [];

			const material = track.highlight ? yellowMaterial : trackMaterial;
			const meshes = [];
			for (let i = 0; i < track.type.lines.length; i++) {
				const line = track.type.lines[i];
				if (line.type === "straight") {
					const { pandr, mesh } = createStraight(line.length, current, material);
					last[track.id!].push(pandr);
					meshes.push(mesh);
				} else if (line.type === "curve") {
					const { pandr, mesh } = createCurve(line.radius, line.angle, line.direction, current, material, line.flipped);
					last[track.id!].push(pandr);
					meshes.push(mesh);
				}
			}
			if (track.type.name.includes("Switch")) {
				switches = [...switches, ({
					straight: meshes[0],
					curve: meshes[1],
					currentDirection: "straight",
					id: track.id!,
					direction: track.type.name.includes("left") ? "left" : "right"
				})];
			}
			current = last[track.id!][0];
		}

		for (const switchE of switches) {
			setSwitchDirection(switchE, switchE.currentDirection);
		}

		scene.onPointerPick = (_, pickResult) => {
			if (pickResult.hit && pickResult.pickedMesh) {
				for (const switchE of switches) {
					if (pickResult.pickedMesh === switchE.straight || pickResult.pickedMesh === switchE.curve) {
						setSwitchDirection(switchE, switchE.currentDirection === "straight" ? "curve" : "straight");
						return;
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

		return () => {
			engine.dispose();
			resizeObserver.disconnect();
		};
	});

	function setSwitchDirection(switchE: Switch, direction: "straight" | "curve") {
		const offset = 0.05;
		if (direction === "straight") {
			switchE.straight.material = trackMaterialRed;
			switchE.curve.material = trackMaterial;
			if (switchE.initialized) switchE.curve.position.y -= offset;
			switchE.straight.position.y += offset;
		} else {
			switchE.straight.material = trackMaterial;
			switchE.curve.material = trackMaterialRed;
			if (switchE.initialized) switchE.straight.position.y -= offset;
			switchE.curve.position.y += offset;
		}
		switchE.currentDirection = direction;
		switchE.initialized = true;
		switches = switches;
	}
</script>

<div class="row">
	<div class="col-sm-3">
		<h5>Switches</h5>
		<ul class="list-group">
			{#each switches.sort((a, b) => a.id - b.id) as switchE}
				<li class="list-group-item">
					<div class="d-flex justify-content-between">
						<div>Weiche {switchE.id}</div>
						<button on:click={() => setSwitchDirection(switchE, switchE.currentDirection === "straight" ? "curve" : "straight")} class="btn btn-outline-primary btn-sm">
							<SwitchIcon active={switchE.currentDirection} direction={switchE.direction} />
							<i class="fas fa-exchange-alt"></i>
						</button>
					</div>
				</li>
			{/each}
		</ul>
	</div>
	<div class="col-sm-9">
		<canvas id="renderCanvas"></canvas>
	</div>
</div>

<style>
	canvas {
		display: block;
		width: 100%;
		height: 100%;

		outline: none;
  		-webkit-tap-highlight-color: rgba(255, 255, 255, 0);
	}
</style>
