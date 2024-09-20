<script lang="ts">
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

	function toRadians(angle: number) {
		return (angle * Math.PI) / 180;
	}
	function fromRadians(radians: number) {
		return (radians * 180) / Math.PI;
	}

	function getCoordsOfCircleSegment(
		radius: number,
		angle: number,
		direction: "left" | "right",
		id?: string
	) {
		if (direction == "right") {
			angle -= 90;

			const radians = toRadians(angle);
			return [
				{ x: 0, y: 0, angle: 0 },
				{
					id,
					x: radius * Math.cos(radians),
					y: radius * Math.sin(radians) + radius,
					angle: angle + 90
				}
			];
		} else {
			angle = 90 - angle;
			const radians = toRadians(angle);
			return [
				{ x: 0, y: 0, angle: 0 },
				{
					id,
					x: radius * Math.cos(radians),
					y: radius * Math.sin(radians) - radius,
					angle: angle - 90
				}
			];
		}
	}

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
			ends: [
				{ id: 1, x: 0, y: 0, angle: 0 },
				{ id: 2, x: 25, y: 0, angle: 0 }
			],
			lines: [{ type: "straight", length: 25 }]
		},
		...Object.fromEntries(
			(["right", "left"] as const).flatMap((direction) =>
				Object.entries(CURVES).map(([key, radius]) => [
					`curve${key}${direction.charAt(0).toUpperCase() + direction.slice(1)}`,
					{
						name: `Curve ${key}`,
						ends: getCoordsOfCircleSegment(radius, CURVE_ANGLE, direction),
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
		switchLeft: {
			name: "Switch Left",
			ends: [
				{ id: 1, x: 0, y: 0, angle: 0 },
				{ id: 2, x: 25, y: 0, angle: 0 },
				getCoordsOfCircleSegment(CURVES.R2, CURVE_ANGLE, "left", "switchLeft")[1]
			],
			lines: [
				{ type: "straight", length: 25 },
				{ type: "curve", radius: CURVES.R2, angle: CURVE_ANGLE, direction: "left" }
			]
		},
		switchRight: {
			name: "Switch Right",
			ends: [
				{ id: 1, x: 0, y: 0, angle: 0 },
				{ id: 2, x: 25, y: 0, angle: 0 },
				getCoordsOfCircleSegment(CURVES.R2, CURVE_ANGLE, "right", "switchRight")[1]
			],
			lines: [
				{ type: "straight", length: 25 },
				{ type: "curve", radius: CURVES.R2, angle: CURVE_ANGLE, direction: "right" }
			]
		}
	} as Record<string, TrackType>;

	type Track = {
		id: number;
		type: TrackType;
		startFrom?: number;
	};

	let trackPlan = [
		{ type: TRACK_TYPES.curveR1Right },
		{ type: TRACK_TYPES.switchLeft },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.curveR2Right },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.curveR2Right },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.switchRight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight },
		{ type: TRACK_TYPES.straight }
	] as Track[];

	let i = 10000;
	for (const t of trackPlan) {
		if (!t.id) {
			t.id = i++;
		}
	}

	let calculatedTrackPlan = [] as { track: Track; x: number; y: number; rotation: number }[];

	function buildTrackPlan() {
		let currentX = 0;
		let currentY = 0;
		let currentRotation = 0;
		const visitedTrackIds = new Set<number>();

		for (let i = 0; i < trackPlan.length; i++) {
			const track = trackPlan[i];

			if (visitedTrackIds.has(track.id)) {
				// Skip already processed tracks to avoid infinite loops
				continue;
			}

			visitedTrackIds.add(track.id);

			calculatedTrackPlan.push({
				track,
				x: currentX,
				y: currentY,
				rotation: currentRotation
			});

			let end = track.type.ends[track.type.ends.length - 1];
			const radians = toRadians(currentRotation);
			const transformedEndX = end.x * Math.cos(radians) - end.y * Math.sin(radians);
			const transformedEndY = end.x * Math.sin(radians) + end.y * Math.cos(radians);
			currentX += transformedEndX;
			currentY += transformedEndY;
			currentRotation += end.angle;
		}
	}

	function renderTrack(track: Track) {
		const color = `#${Math.floor(Math.random() * 16777215)
			.toString(16)
			.padStart(6, "0")}`;
		let svgContent = "";
		track.type.lines.forEach((line) => {
			if (line.type === "straight") {
				svgContent += `<line x1="0" y1="0" x2="${line.length}" y2="0" stroke="${color}" stroke-width="5" />`;
			} else if (line.type === "curve") {
				let a = line.angle;
				const r = line.radius;
				const angleRadians = (Math.PI / 180) * a;
				let endX = r * Math.cos(angleRadians);
				let endY = r * Math.sin(angleRadians);

				const largeArcFlag = a > 180 ? 1 : 0;

				if (line.direction == "right") {
					svgContent += `<g transform="translate(0, ${r}) rotate(-90)">`;
					svgContent += `<path d="M ${r} 0 A ${r} ${r} 0 ${largeArcFlag} 1 ${endX} ${endY}" stroke="${color}" stroke-width="5" fill="none" />`;
					svgContent += `</g>`;
				}

				if (line.direction == "left") {
					svgContent += `<g transform="rotate(${-90 - line.angle}) translate(${-r}, 0)">`;
					svgContent += `<path d="M ${r} 0 A ${r} ${r} 0 ${largeArcFlag} 0 ${endX} ${endY}" stroke="${color}" stroke-width="5" fill="none" />`;
					svgContent += `</g>`;
				}
			}
		});
		return svgContent;
	}

	buildTrackPlan();
</script>

<svg viewBox="-70 -90 300 300">
	{#each calculatedTrackPlan as track (track.track.id)}
		<g class="track" transform="translate({track.x} {track.y}) rotate({track.rotation})">
			{@html renderTrack(track.track)}
		</g>
	{/each}
</svg>

<style>
	svg {
		background-color: #f0f0f0;
		width: 90vh;
		height: 90vh;
	}
	.track {
		transform-origin: 0 0;
	}
</style>
