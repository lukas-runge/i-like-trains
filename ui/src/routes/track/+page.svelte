<script lang="ts">
    type Curve = {
        type: "curve";
        radius: number;
        angle: number;
    };

    type StraightLine = {
        type: "straight";
        length: number;
    };

    type TrackType = {
        name: string;
        ends: { x: number; y: number; angle: number }[];
        lines: Array<Curve | StraightLine>;
    };

    function toRadians(angle: number) {
        return (angle * Math.PI) / 180;
    }
    function fromRadians(radians: number) {
        return (radians * 180) / Math.PI;
    }

    function getCoordsOfCircleSegment(radius: number, angle: number) {
        angle -= 90;
        angle = toRadians(angle);
        return [
            { x: 0, y: 0, angle: 0 },
            {
                x: radius * Math.cos(angle),
                y: radius * Math.sin(angle) + radius,
                angle: fromRadians(angle) + 90,
            },
        ];
    }

    const CURVES = {
        R1: 36.0,
        R2: 43.75,
        R3: 51.5,
        R4: 57.93,
        R5: 64.36,
    };

    const CURVE_COUNT = 12;

    const TRACK_TYPES = {
        straight: {
            name: "Straight",
            ends: [
                { x: 0, y: 0, angle: 0 },
                { x: 25, y: 0, angle: 0 },
            ],
            lines: [{ type: "straight", length: 25 }],
        },
        ...Object.fromEntries(
            Object.entries(CURVES).map(([key, radius]) => [
                `curve${key}` as const,
                {
                    name: `Curve ${key}`,
                    ends: getCoordsOfCircleSegment(radius, 360 / CURVE_COUNT),
                    lines: [
                        {
                            type: "curve",
                            radius,
                            angle: 360 / CURVE_COUNT,
                        },
                    ],
                },
            ])
        ),
    } as Record<"straight" | `curve${keyof typeof CURVES}`, TrackType>;

    type Track = {
        id: number;
        type: TrackType;
    };

    let trackPlan = [
        { id: 1, type: TRACK_TYPES.curveR1 },
        { id: 2, type: TRACK_TYPES.straight },
        { id: 3, type: TRACK_TYPES.straight },
        { id: 4, type: TRACK_TYPES.straight },
        { id: 5, type: TRACK_TYPES.curveR1 },
        { id: 6, type: TRACK_TYPES.curveR5 },
    ];

    let calculatedTrackPlan = [] as { track: Track; x: number; y: number; rotation: number }[];

    function buildTrackPlan() {
        let currentX = 0;
        let currentY = 0;
        let currentRotation = 0;

        for (let i = 0; i < trackPlan.length; i++) {
            const track = trackPlan[i];

            calculatedTrackPlan.push({
                track,
                x: currentX,
                y: currentY,
                rotation: currentRotation,
            });
            let end = track.type.ends[1];
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
                const a = line.angle;
                const r = line.radius;
                const angleRadians = (Math.PI / 180) * a;
                let endX = r * Math.cos(angleRadians);
                let endY = r * Math.sin(angleRadians);
                const largeArcFlag = a > 180 ? 1 : 0;
                svgContent += `<g transform="translate(0, ${r}) rotate(-90)">`;
                svgContent += `<path d="M ${r} 0 A ${r} ${r} 0 ${largeArcFlag} 1 ${endX} ${endY}" stroke="${color}" stroke-width="5" fill="none" />`;
                svgContent += `</g>`;
            }
        });
        return svgContent;
    }

    buildTrackPlan();
</script>

<svg viewBox="-70 -20 300 300">
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
