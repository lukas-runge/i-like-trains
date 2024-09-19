<script lang="ts">
    import { onMount } from "svelte";
    import { create, toBinary } from "@bufbuild/protobuf";
    import { ControlPacketSchema, ControlPacket_Direction } from "$lib/protobuf/message_pb";

    let supported = true;
    let writable: WritableStreamDefaultWriter<Uint8Array> | null = null;
    let error: string | null = null;
    let abortSignal: AbortSignal;

    onMount(() => {
        if (!("serial" in navigator)) {
            supported = false;
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
                            const decoder = new TextDecoder();
                            const text = decoder.decode(chunk);
                            onText(text);
                        },
                    }),
                    { signal: abortSignal }
                );
                writable = p.writable.getWriter();
                console.log("Connected");
            })
            .catch((e) => {
                console.error(e);
                error = e.message;
            });
    }

    function onText(text: string) {
        console.log(`🎉: ${text}`);
    }

    function go() {
        const packet = create(ControlPacketSchema, {
            address: 13,
            command: {
                case: "drive",
                value: {
                    speed: 27,
                    direction: ControlPacket_Direction.FORWARD,
                },
            },
        });
        const binary = toBinary(ControlPacketSchema, packet);
        if (!writable) {
            console.error("No writable stream");
            return;
        }
        writable.write(binary);
    }
</script>

{#if supported}
    <div>
        Webserial Status:
        {#if writable}
            <span class="text-success">Connected</span>
        {:else}
            {#if error}
                <span class="text-danger">Error: {error == "Failed to execute 'requestPort' on 'Serial': Must be handling a user gesture to show a permission request." ? "Cannot access serial port without user gesture. Please press the button on the device." : error}</span>
            {:else}
                <span class="text-warning">Disconnected</span>
            {/if}
            <div class="my-3">
                <button class="btn btn-outline-primary" on:click={connect}>Connect</button>
            </div>
        {/if}
    </div>
    <hr />
    <div>
        <button on:click={go} class="btn btn-outline-primary">Go</button>
    </div>
{:else}
    <div class="alert alert-danger">
        <b>Unsupported browser</b><br />
        Unfortunately, your browser does not support the features required to run this application (Webserial API).<br />
        Please use a Chrome-based browser (e.g. Google Chrome, Microsoft Edge, Brave, Opera) to control your model railway.
    </div>
{/if}
