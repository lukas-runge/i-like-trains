<script lang="ts">
	import { onMount } from "svelte";
	import { create, toBinary, fromBinary } from "@bufbuild/protobuf";
	import {
		ControlPacketSchema,
		ControlPacket_Direction,
		HandshakeSchema,
		HandshakeType,
		MessageSchema,
		type Handshake
	} from "$lib/protobuf/message_pb";

	let status:
		| { type: "disconnected" }
		| { type: "handshaking" }
		| { type: "connected" }
		| { type: "error"; message: string }
		| { type: "unsupported" } = { type: "disconnected" };

	let writable: WritableStreamDefaultWriter<Uint8Array> | null = null;
	let abortSignal: AbortSignal;
	let resolveHandshakePromise: ((value: void) => void) | null = null;
	let rejectHandshakePromise: ((reason: any) => void) | null = null;
	let incomingByteQueue = new Uint8Array();

	onMount(() => {
		if (!("serial" in navigator)) {
			status = { type: "unsupported" };
			return;
		}

		const abortController = new AbortController();
		abortSignal = abortController.signal;

		connect();

		return () => abortController.abort();
	});

	function connect() {
		navigator.serial
			.requestPort()
			.then((p) => p.open({ baudRate: 115200 }).then(() => p))
			.then((p) => {
				if (!p.readable) {
					throw new Error("No readable stream");
				}
				if (!p.writable) {
					throw new Error("No writable stream");
				}
				p.readable.pipeTo(
					new WritableStream({
						write(chunk) {
							handleDataChunk(chunk);
						}
					}),
					{ signal: abortSignal }
				);
				writable = p.writable.getWriter();
				status = { type: "handshaking" };
				handshake();
			})
			.catch((e) => {
				console.error(e);
				status = { type: "error", message: e.message };
			});
	}

	function handleDataChunk(chunk: Uint8Array) {
		incomingByteQueue = new Uint8Array([...incomingByteQueue, ...chunk]);
		while (incomingByteQueue.length >= 4) {
			const length =
				incomingByteQueue[0] |
				(incomingByteQueue[1] << 8) |
				(incomingByteQueue[2] << 16) |
				(incomingByteQueue[3] << 24);
			if (incomingByteQueue.length < length + 4) {
				break;
			}
			onPacket(incomingByteQueue.slice(4, length + 4));
			incomingByteQueue = incomingByteQueue.slice(length + 4);
		}
	}

	async function writePacket<T extends Parameters<typeof create>[0]>(
		schema: T,
		data: Parameters<typeof create<T>>[1]
	) {
		if (!writable) {
			console.error("No writable stream");
			return;
		}
		const bytes = toBinary(schema, create(schema, data));
		const length = new Uint8Array([
			bytes.length & 0xff,
			(bytes.length >> 8) & 0xff,
			(bytes.length >> 16) & 0xff,
			(bytes.length >> 24) & 0xff
		]);
		await writable.write(new Uint8Array([...length, ...bytes]));
	}

	async function handshake() {
		status = { type: "handshaking" };
		writePacket(MessageSchema, {
			content: { case: "handshake", value: { type: HandshakeType.REQUEST } }
		});
		const timeout = setTimeout(() => {
			if (rejectHandshakePromise) {
				rejectHandshakePromise(
					new Error("Handshake timeout - Are you sure the pico is running the correct firmware?")
				);
			}
			clearTimeout(timeout);
		}, 5000);
		new Promise<void>((resolve, reject) => {
			resolveHandshakePromise = () => {
				resolve();
				clearTimeout(timeout);
			};
			rejectHandshakePromise = reject;
		})
			.then(() => {
				status = { type: "connected" };
			})
			.catch((e) => {
				console.error(e);
				status = { type: "error", message: e.message };
			});
	}

	function onPacket(data: Uint8Array) {
		const message = fromBinary(MessageSchema, data);
		switch (message.content.case) {
			case "handshake":
				if (message.content.value.type === HandshakeType.RESPONSE) {
					if (!resolveHandshakePromise) {
						throw new Error("Received handshake response without pending handshake");
					}
					resolveHandshakePromise();
				}
				break;
			case "controlPacket":
				throw new Error("The client should not send control packets to the host");
		}
	}

	async function drive(forward: boolean) {
		await writePacket(MessageSchema, {
			content: {
				case: "controlPacket",
				value: {
					address: 13,
					command: {
						case: "drive",
						value: {
							speed: 31,
							direction: forward
								? ControlPacket_Direction.FORWARD
								: ControlPacket_Direction.BACKWARD
						}
					}
				}
			}
		});
	}

	async function halt() {
		await writePacket(MessageSchema, {
			content: {
				case: "controlPacket",
				value: {
					address: 13,
					command: {
						case: "halt",
						value: {}
					}
				}
			}
		});
	}

	async function estop() {
		await writePacket(MessageSchema, {
			content: {
				case: "controlPacket",
				value: {
					address: 13,
					command: {
						case: "emergencyStop",
						value: {}
					}
				}
			}
		});
	}

	async function setLightState(on: boolean) {
		await writePacket(MessageSchema, {
			content: {
				case: "controlPacket",
				value: {
					address: 13,
					command: {
						case: "light",
						value: {
							on
						}
					}
				}
			}
		});
	}

	function handleKeyboardInput(event: KeyboardEvent) {
		if (event.code === "ArrowLeft") {
			drive(false);
		} else if (event.code === "ArrowRight") {
			drive(true);
		} else if (event.code === "Space") {
			halt();
		} else if (event.key === "e") {
			estop();
		}
	}
</script>

<svelte:window on:keydown={handleKeyboardInput} />

{#if status.type !== "unsupported"}
	<div>
		Webserial Status:
		{#if status.type === "handshaking"}
			<span class="text-primary">
				Connecting... <div class="spinner spinner-border spinner-border-sm"></div>
			</span>
		{:else if status.type === "connected"}
			<span class="text-success">Connected</span>
		{:else if status.type === "error" || status.type === "disconnected"}
			{#if status.type == "error"}
				<span class="text-danger"
					>Error: {status.message ==
					"Failed to execute 'requestPort' on 'Serial': Must be handling a user gesture to show a permission request."
						? "Cannot access serial port without user gesture. Please press the button on the device."
						: status.message}</span
				>
			{:else}
				<span class="text-danger">Disconnected</span>
			{/if}
			<div class="my-3">
				<button class="btn btn-outline-primary" on:click={connect}>Connect</button>
			</div>
		{/if}
	</div>
	<hr />
	<div>
		<button on:click={() => drive(false)} class="btn btn-outline-primary"
			><i class="fas fa-arrow-left" /></button
		>
		<button on:click={halt} class="btn btn-outline-primary">HALT</button>
		<button on:click={estop} class="btn btn-outline-primary">E-STOP</button>
		<button on:click={() => drive(true)} class="btn btn-outline-primary"
			><i class="fas fa-arrow-right" /></button
		>
		<button on:click={() => setLightState(true)} class="btn btn-outline-primary"
			><i class="fas fa-lightbulb" style="color: orange;" /></button
		>
		<button on:click={() => setLightState(false)} class="btn btn-outline-primary"
			><i class="fas fa-lightbulb" style="color: black;" /></button
		>

		<ul class="mt-3 list-unstyled">
			<li>Press <kbd>←</kbd> to drive the train backwards</li>
			<li>Press <kbd>→</kbd> to drive the train forwards</li>
			<li>Press <kbd>Space</kbd> to halt the train</li>
			<li>Press <kbd>E</kbd> to emergency stop the train</li>
		</ul>
	</div>
{:else}
	<div class="alert alert-danger">
		<b>Unsupported browser</b><br />
		Unfortunately, your browser does not support the features required to run this application (Webserial
		API).<br />
		Please use a Chrome-based browser (e.g. Google Chrome, Microsoft Edge, Brave, Opera) to control your
		model railway.
	</div>
{/if}
