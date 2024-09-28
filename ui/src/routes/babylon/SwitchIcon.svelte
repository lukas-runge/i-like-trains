<script lang="ts">
    export let direction: "left" | "right" = "right";
    export let active: "straight" | "curve" = "straight";
</script>

<svg xmlns="http://www.w3.org/2000/svg" viewBox="-10 -10 20 20" fill="none" stroke-width="4" stroke-linecap="round" stroke-linejoin="round">
    <line x1="0" y1="-10" x2="0" y2="10" stroke="black" />
    <circle cx="{direction == "left" ? -30 : 30}" cy="10" r="30" fill="none" stroke={active === "curve" ? "red" : "black"} />
    {#if active === "straight"}
        <line x1="0" y1="-10" x2="0" y2="10" stroke="red" />
    {/if}
</svg>

<style>
    svg {
        height: 1.5rem;
    }
</style>