<script lang="ts">
    import "../app.scss";
</script>

<main>
    <div class="container py-4">
        <header class="pb-3 mb-4 border-bottom">
            <a href="/" class="d-flex align-items-center text-body-emphasis text-decoration-none">
                <i class="fas fa-train fs-1 me-2"></i>
                <span class="fs-4">Web Train Control</span>
            </a>
        </header>

        <slot />

        <footer class="pt-3 mt-4 text-body-secondary border-top">© 2024 Lukas Runge, Felix Scheib & Hannes Rüger</footer>
    </div>
</main>
